/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_ALGORITHM_REGISTRATION_3D_CT_H_
# define NLL_ALGORITHM_REGISTRATION_3D_CT_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief 3D affine registration of CT-CT medical volume, note that only XZ and YZ planes
           are used to align the volumes (so the XY will be incorrect if there is such a transformation)
    
    Internally, it is assumed the volume is correctly oriented in a Head-First-Supine orientation
    (this is assumed for the table removal algorithm). The volume is then projected on the XZ, YZ and XY
    planes.
    The XY plane is used to determine the Y position of the table, everything below this position will be removed.
    Finally the 2 pairs of projection are registered using a SURF 2D - ransac - affine estimator one by one and
    a global transformation matrix is computed.
    */
   template <class TransformationEstimatorFactory = impl::SurfEstimatorAffineIsotropicFactory>
   class AffineRegistrationCT3d
   {
   public:
      typedef core::Matrix<double>  Matrix;
      typedef algorithm::AffineRegistrationPointBased2d<TransformationEstimatorFactory>   Registration2D;

      enum Result
      {
         SUCCESS,
         FAILED_TOO_LITTLE_INLIERS
      };

   public:
      /**
       @brief run the registration
       @param minBoundingBoxSource the min index of the bounding box of the source volume
       @param maxBoundingBoxSource the max index of the bounding box of the source volume
       @param minBoundingBoxTarget the min index of the bounding box of the source volume
       @param maxBoundingBoxTarget the max index of the bounding box of the source volume
       @param out the registration matrix to register the target, expressed as a source->target transformation
              (i.e., resampling the <target> with <out> will have the volumes aligned
       @param exportDebug if true the planes, points and registration matrices will be exported
       */
      template <class T, class BufferType>
      Result process( const imaging::VolumeSpatial<T, BufferType>& source,
                      const imaging::VolumeSpatial<T, BufferType>& target,
                      Matrix& out,
                      bool tableRemoval = true,
                      const core::vector3i& minBoundingBoxSource = core::vector3i(),
                      const core::vector3i& maxBoundingBoxSource = core::vector3i(),
                      const core::vector3i& minBoundingBoxTarget = core::vector3i(),
                      const core::vector3i& maxBoundingBoxTarget = core::vector3i(),
                      bool exportDebug = true )
      {
         srand( 0 );   // TODO REMOVE
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "starting CT-CT planar registration..." );
         core::Timer timer;

         {
            std::stringstream ss;
            ss << " source bounding box=" << minBoundingBoxSource[ 0 ] << " " << minBoundingBoxSource[ 1 ] << " " << minBoundingBoxSource[ 2 ] << "|| "
                                          << maxBoundingBoxSource[ 0 ] << " " << maxBoundingBoxSource[ 1 ] << " " << maxBoundingBoxSource[ 2 ] << std::endl;
            ss << " target bounding box=" << minBoundingBoxTarget[ 0 ] << " " << minBoundingBoxTarget[ 1 ] << " " << minBoundingBoxTarget[ 2 ] << "|| "
                                          << maxBoundingBoxTarget[ 0 ] << " " << maxBoundingBoxTarget[ 1 ] << " " << maxBoundingBoxTarget[ 2 ] << std::endl;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // save and set the origins to (0, 0, 0)
         // this is to simplify the transformation computations
         _savePst( source, target );

         out = core::identityMatrix<Matrix>( 4 );

         // preprocess the volume
         core::Image<ui8> pxs, pys, pzs;
         core::Image<ui8> pxt, pyt, pzt;

         core::Timer projectionTimer1;
         getProjections( source, pxs, pys, pzs, true, true, tableRemoval );
         getProjections( target, pxt, pyt, pzt, true, false, tableRemoval );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, " projection time=" + core::val2str( projectionTimer1.getCurrentTime() ) );

         try
         {
            // computes the registration in YZ plane. We must multiply the bounding box by the spacing as the planes are formed like this...
            Registration2D registrationx;
            Matrix tfmx = registrationx.compute( pxs, pxt, core::vector2i( (int)( minBoundingBoxSource[ 1 ] * source.getSpacing()[ 1 ] ), (int)( minBoundingBoxSource[ 2 ] * source.getSpacing()[ 2 ] ) ),
                                                           core::vector2i( (int)( maxBoundingBoxSource[ 1 ] * source.getSpacing()[ 1 ] ), (int)( maxBoundingBoxSource[ 2 ] * source.getSpacing()[ 2 ] ) ),
                                                           core::vector2i( (int)( minBoundingBoxTarget[ 1 ] * target.getSpacing()[ 1 ] ), (int)( minBoundingBoxTarget[ 2 ] * target.getSpacing()[ 2 ] ) ),
                                                           core::vector2i( (int)( maxBoundingBoxTarget[ 1 ] * target.getSpacing()[ 1 ] ), (int)( maxBoundingBoxTarget[ 2 ] * target.getSpacing()[ 2 ] ) ) );

            Matrix rotx( 4, 4 );
            rotx( 0, 0 ) = 1;
            rotx( 1, 1 ) = tfmx( 0, 0 );
            rotx( 2, 1 ) = tfmx( 1, 0 );
            rotx( 1, 2 ) = tfmx( 0, 1 );
            rotx( 2, 2 ) = tfmx( 1, 1 );
            rotx( 1, 3 ) = tfmx( 0, 2 );
            rotx( 2, 3 ) = tfmx( 1, 2 );
            rotx( 3, 3 ) = 1;
            
            // find the biggest volume in case we are registering a very small volume with a big one,
            // the small volume may be translated outside its bounds
            const double volSource = source.size()[ 0 ] * source.getSpacing()[ 0 ] +
                                     source.size()[ 1 ] * source.getSpacing()[ 1 ] +
                                     source.size()[ 2 ] * source.getSpacing()[ 2 ];
            const double volTarget = target.size()[ 0 ] * target.getSpacing()[ 0 ] +
                                     target.size()[ 1 ] * target.getSpacing()[ 1 ] +
                                     target.size()[ 2 ] * target.getSpacing()[ 2 ];
            const core::vector3ui& size = ( volSource > volTarget ) ? source.size() : target.size();
            const core::Matrix<float>& pst = ( volSource > volTarget ) ? source.getPst() : target.getPst();

            core::Timer projectionTimer2;
            imaging::VolumeSpatial<T, BufferType> resampledTarget( size, pst, 0 );
            imaging::resampleVolumeTrilinear( target, rotx, resampledTarget );

            // recompute the projection on the resampled volume
            core::Image<ui8> pxt2, pyt2, pzt2;
            getProjections( resampledTarget, pxt2, pyt2, pzt2, false, true, tableRemoval );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, " projection time2=" + core::val2str( projectionTimer2.getCurrentTime() ) );

            // // computes the registration in XZ plane
            Registration2D registrationy;
            Matrix tfmy = registrationy.compute( pys, pyt2, core::vector2i( (int)( minBoundingBoxSource[ 0 ] * source.getSpacing()[ 0 ] ), (int)( minBoundingBoxSource[ 2 ] * source.getSpacing()[ 2 ] ) ),
                                                            core::vector2i( (int)( maxBoundingBoxSource[ 0 ] * source.getSpacing()[ 0 ] ), (int)( maxBoundingBoxSource[ 2 ] * source.getSpacing()[ 2 ] ) ),
                                                            core::vector2i( (int)( minBoundingBoxTarget[ 0 ] * target.getSpacing()[ 0 ] ), (int)( minBoundingBoxTarget[ 2 ] * target.getSpacing()[ 2 ] ) ),
                                                            core::vector2i( (int)( maxBoundingBoxTarget[ 0 ] * target.getSpacing()[ 0 ] ), (int)( maxBoundingBoxTarget[ 2 ] * target.getSpacing()[ 2 ] ) ) );

            // do some postprocessing...
            Matrix roty( 4, 4 );
            roty( 0, 0 ) =  tfmy( 0, 0 );
            roty( 2, 0 ) =  tfmy( 1, 0 );
            roty( 0, 2 ) =  tfmy( 0, 1 );
            roty( 2, 2 ) =  tfmy( 1, 1 );
            roty( 1, 1 ) = 1;
            roty( 3, 3 ) = 1;
            roty( 0, 3 ) = tfmy( 0, 2 );
            roty( 2, 3 ) = tfmy( 1, 2 );

            // save some debug info
            if ( exportDebug )
            {
               pxSrc = pxs;
               pySrc = pys;
               pxTgt = pxt;
               pyTgt = pyt2;
               pxInliers = registrationx.getInliers();
               pyInliers = registrationy.getInliers();
               pxTfm = tfmx;
               pyTfm = tfmy;
            }

            // we have set the origin of the volumes to 0
            // to correct this for the final transformation, we need to take into account:
            //  - the transformation source->target
            //  - the shift to origin: source->0
            Matrix tr = core::createTranslation4x4( core::vector3d( _pstTarget( 0, 3 ) - _pstSource( 0, 3 ),
                                                                    _pstTarget( 1, 3 ) - _pstSource( 1, 3 ),
                                                                    _pstTarget( 2, 3 ) - _pstSource( 2, 3 ) ) );

            Matrix tr2 = core::createTranslation4x4( core::vector3d( _pstSource( 0, 3 ),
                                                                     _pstSource( 1, 3 ),
                                                                     _pstSource( 2, 3 ) ) );

            Matrix tr2I = core::createTranslation4x4( core::vector3d( - _pstSource( 0, 3 ),
                                                                      - _pstSource( 1, 3 ),
                                                                      - _pstSource( 2, 3 ) ) );
            out =  tr * tr2 *  rotx * roty * tr2I;

         } catch(...)
         {
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "too little inliers, registration failed..." );
            // restore the original origins
            _loadPst( source, target );
            return FAILED_TOO_LITTLE_INLIERS;
         }

         {
            std::stringstream ss;
            ss << " registration successful, time=" << timer.getCurrentTime() << std::endl;
            ss << " final transformation=";
            out.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // restore the original origins
         _loadPst( source, target );
         return SUCCESS;
      }

   private:
      template <class T, class BufferType>
      void _loadPst( const imaging::VolumeSpatial<T, BufferType>& source,
                     const imaging::VolumeSpatial<T, BufferType>& target )
      {
         imaging::VolumeSpatial<T, BufferType>& snc = const_cast< imaging::VolumeSpatial<T, BufferType>& >( source );
         imaging::VolumeSpatial<T, BufferType>& tnc = const_cast< imaging::VolumeSpatial<T, BufferType>& >( target );

         snc.setPst( _pstSource );
         tnc.setPst( _pstTarget );
      }

      template <class T, class BufferType>
      void _savePst( const imaging::VolumeSpatial<T, BufferType>& source,
                     const imaging::VolumeSpatial<T, BufferType>& target )
      {
         _pstSource.clone( source.getPst() );
         _pstTarget.clone( target.getPst() );

         imaging::VolumeSpatial<T, BufferType>& snc = const_cast< imaging::VolumeSpatial<T, BufferType>& >( source );
         imaging::VolumeSpatial<T, BufferType>& tnc = const_cast< imaging::VolumeSpatial<T, BufferType>& >( target );

         snc.setOrigin( core::vector3f( 0, 0, 0 ) );
         tnc.setOrigin( core::vector3f( 0, 0, 0 ) );
      }

      template <class T, class BufferType>
      void getProjections( const imaging::VolumeSpatial<T, BufferType>& v,
                           core::Image<ui8>& px,
                           core::Image<ui8>& py,
                           core::Image<ui8>& pz,
                           bool doPx,
                           bool doPy,
                           bool doTableRemoval )
      {
         // first get the projections and y-position of the table
         imaging::LookUpTransformWindowingRGB lut( -10, 250, 256, 1 );
         lut.createGreyscale();

         ui32 normSizeY;
         ui32 normSizeX;
         pz = projectImageZ( v, lut, normSizeY, normSizeX );
         int ymax;
         if ( doTableRemoval )
            ymax = findTableY( pz );
         else
            ymax = (int)( v.size()[ 1 ] * v.getSpacing()[ 1 ] ) - 1;
         if ( ymax > 0 )
         {
            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp parallel for
            #endif
            for ( int x = 0; x < (int)pz.sizex(); ++x )
            {
               for ( ui32 y = ymax; y < pz.sizey(); ++y )
               {
                  pz( x, y, 0 ) = 0;
               }
            }
         } else {
            ymax = static_cast<int>( v.getSize()[ 1 ] * v.getSpacing()[ 1 ] ) - 1;
         }

         if ( doPx )
         {
            px = projectImageX( v, lut, ymax, normSizeX / 2 );
         }

         if ( doPy )
         {
            py = projectImageY( v, lut, ymax, normSizeY / 2 );
         }
      }

   public:
      template <class T, class BufferType>
      static core::Image<ui8> projectImageY( const imaging::VolumeSpatial<T, BufferType>& v, const imaging::LookUpTransformWindowingRGB& lut, int ymax, ui32 maxSizeY )
      {
         typedef typename imaging::VolumeSpatial<T, BufferType>::ConstDirectionalIterator ConstDirectionalIterator;
         core::Image<ui8> p( static_cast<ui32>( v.getSize()[ 0 ] * v.getSpacing()[ 0 ] ),
                             static_cast<ui32>( v.getSize()[ 2 ] * v.getSpacing()[ 2 ] ),
                             1 );
         const int endz = static_cast<int>( v.getSize()[ 2 ] * v.getSpacing()[ 2 ] ) - 1;
         const double norm = ( maxSizeY * v.getSpacing()[ 1 ] );

         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int z = 0; z < endz; ++z )
         {
            for ( ui32 x = 0; x < v.getSize()[ 0 ] * v.getSpacing()[ 0 ] - 1; ++x )
            {
               double accum = 0;
               ConstDirectionalIterator it = v.getIterator( static_cast<ui32>( x / v.getSpacing()[ 0 ] ),
                                                            0,
                                                            static_cast<ui32>( z / v.getSpacing()[ 2 ] ) );
               for ( ui32 y = 0; y < ymax / v.getSpacing()[ 1 ]; ++y )
               {
                  const double val = lut.transform( static_cast<float>( *it ) )[ 0 ];
                  it.addy();
                  accum += val;
               }
               
               accum /= norm;
               p( x, z, 0 ) = static_cast<ui8>( NLL_BOUND( accum, 0, 255 ) );
            }
         }
         return p;
      }

      template <class T, class BufferType>
      static core::Image<ui8> projectImageX( const imaging::VolumeSpatial<T, BufferType>& v, const imaging::LookUpTransformWindowingRGB& lut, int ymax, ui32 maxSizeY )
      {
         typedef typename imaging::VolumeSpatial<T, BufferType>::ConstDirectionalIterator ConstDirectionalIterator;

         core::Image<ui8> p( static_cast<ui32>( v.getSize()[ 1 ] * v.getSpacing()[ 1 ] ),
                             static_cast<ui32>( v.getSize()[ 2 ] * v.getSpacing()[ 2 ] ),
                             1 );
         const int endz = static_cast<int>( v.getSize()[ 2 ] * v.getSpacing()[ 2 ] ) - 1;
         const double norm = ( maxSizeY * v.getSpacing()[ 0 ] );

         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int z = 0; z < endz; ++z )
         {
            for ( ui32 y = 0; y < (ui32)ymax; ++y )
            {
               double accum = 0;
               ConstDirectionalIterator it = v.getIterator( 0,
                                                            static_cast<ui32>( y / v.getSpacing()[ 1 ] ),
                                                            static_cast<ui32>( z / v.getSpacing()[ 2 ] ) );
               for ( ui32 x = 0; x < v.getSize()[ 0 ]; ++x )
               {
                  const double val = lut.transform( static_cast<float>( *it ) )[ 0 ];
                  it.addx();
                  accum += val;
               }
               accum /= norm;
               p( y, z, 0 ) = static_cast<ui8>( NLL_BOUND( accum, 0, 255 ) );
            }
         }

         return p;
      }

      template <class T, class BufferType>
      static core::Image<ui8> projectImageZ( const imaging::VolumeSpatial<T, BufferType>& v, const imaging::LookUpTransformWindowingRGB& lut, ui32& maxSizeY, ui32& maxSizeX )
      {
         typedef typename imaging::VolumeSpatial<T, BufferType>::ConstDirectionalIterator ConstDirectionalIterator;
         core::Image<ui8> p( static_cast<ui32>( v.getSize()[ 0 ] * v.getSpacing()[ 0 ] ),
                             static_cast<ui32>( v.getSize()[ 1 ] * v.getSpacing()[ 1 ] ),
                             1 );
         ui32 min = p.sizey() - 1;
         ui32 max = 0;

         ui32 minX = p.sizex() - 1;
         ui32 maxX = 0;

         const int endx = static_cast<int>( v.getSize()[ 0 ] * v.getSpacing()[ 0 ] ) - 1;
         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int x = 0; x < endx; ++x )
         {
            for ( ui32 y = 0; y < v.getSize()[ 1 ] * v.getSpacing()[ 1 ] - 1; ++y )
            {
               double accum = 0;
               ConstDirectionalIterator it = v.getIterator( static_cast<ui32>( x / v.getSpacing()[ 0 ] ),
                                                            static_cast<ui32>( y / v.getSpacing()[ 1 ] ),
                                                            0 );
               for ( ui32 z = 0; z < v.getSize()[ 2 ]; ++z )
               {
                  const double val = lut.transform( static_cast<float>( *it ) )[ 0 ];
                  it.addz();
                  accum += val;
               }

               accum /= v.getSize()[ 0 ];

               const ui8 val = static_cast<ui8>( NLL_BOUND( accum * 3, 0, 255 ) );
               p( x, y, 0 ) = val;

               if ( val )
               {
                  if ( min > y )
                  {
                     min = y;
                  }

                  if ( max < y )
                  {
                     max = y;
                  }

                  if ( (int)minX > x )
                  {
                     minX = x;
                  }

                  if ( (int)maxX < x )
                  {
                     maxX = x;
                  }
               }
            }
         }

         maxSizeY = max - min + 1;
         maxSizeX = maxX - minX + 1;
         return p;
      }

      // find the table: from top to bottom, the Y position can be determined: detect the points top to bottom with only a few connected
      // pixels
      static int findTableY( const core::Image<ui8>& iz )
      {
         ui32 nbPixelTable = 0;
         double mean = 0;

         for ( ui32 x = 0; x < iz.sizex(); ++x )
         {
            ui32 lineId = 0;
            ui32 nbConnected[ 5 ] = {0, 0, 0, 0, 0};
            int ymin[ 5 ];
            int ymax[ 5 ];
            for ( int y = iz.sizey() - 1; y > 0; --y )
            {
               if ( iz( x, y, 0 ) > 0 )
               {
                  if ( nbConnected[ lineId ] == 0 )
                     ymin[ lineId ] = y;
                  ++nbConnected[ lineId ];
               } else if ( nbConnected[ lineId ] && abs( y - ymin[ lineId ] ) < 30 )
               {
                  ymax[ lineId ] = y;
                  ++lineId;
                  if ( lineId >= 3)
                     break;
               }
            }

            if ( lineId )
            {
               ++nbPixelTable;
               mean += ymax[ lineId - 1 ];
            }
         }

         if ( nbPixelTable > 120 )
            return static_cast<int>( mean / nbPixelTable );
         return -1;
      }

   public:
      // debug info
      core::Image<ui8>  pxSrc;
      core::Image<ui8>  pxTgt;
      core::Image<ui8>  pySrc;
      core::Image<ui8>  pyTgt;
      typename Registration2D::PointPairs    pxInliers;
      typename Registration2D::PointPairs    pyInliers;
      Matrix            pxTfm;
      Matrix            pyTfm;

   private:
      core::Matrix<float> _pstSource;
      core::Matrix<float> _pstTarget;
   };
}
}

#endif
