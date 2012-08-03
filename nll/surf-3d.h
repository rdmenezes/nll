/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
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

#ifndef NLL_ALGORITHM_SURF_3D_H_
# define NLL_ALGORITHM_SURF_3D_H_

// if defined, SURF will use a global angle detection mecanism which seems more efficient/robust
#define NLL_SURF_USE_FEATURE_ANGLE_UPGRADE

#define NLL_SURF_3D_NB_AREA_PER_FEATURE         4.0f
#define NLL_SURF_3D_NB_AREA_PER_FEATURE_SIZE    20.0f
#define NLL_SURF_3D_NB_SUBAREA_PER_AREA         5.0f

namespace nll
{
namespace algorithm
{
	/**
    @ingroup algorithm
    @brief Implementation of the Speeded Up Robust Features or SURF algorithm for 3 dimentional data

           Feature detector which intend to be robust to scaling, illumination and rotation

    @see http://sites.google.com/site/chrisevansdev/files/opensurf.pdf for good notes on the algorithm
         http://www.vision.ee.ethz.ch/~surf/eccv06.pdf reference paper

    @note we are assuming the spacing is the same for all dimensions
    */
   class SpeededUpRobustFeatures3d
   {
      typedef double                            value_type;
      typedef FastHessianDetPyramid3d::Volume   Volume;
      typedef core::Matrix<value_type>          Matrix;
      typedef core::ProbabilityDistributionFunctionGaussian<value_type> GaussianPdf;

      struct LocalPoint
      {
         LocalPoint( value_type x, value_type y, value_type z ) : dx( x ), dy( y ), dz( z )
         {}

         value_type dx;
         value_type dy;
         value_type dz;
      };

   public:
      struct Point
      {
         typedef core::Buffer1D<value_type> Features;
         Point( core::vector3i p, size_t s ) : position( p ), scale( s ), features( static_cast<int>( 6 * ( NLL_SURF_3D_NB_AREA_PER_FEATURE * NLL_SURF_3D_NB_AREA_PER_FEATURE * NLL_SURF_3D_NB_AREA_PER_FEATURE ) ) ), weight( 1 )
         {}

         Point() : features( static_cast<int>( 6 * ( NLL_SURF_3D_NB_AREA_PER_FEATURE * NLL_SURF_3D_NB_AREA_PER_FEATURE * NLL_SURF_3D_NB_AREA_PER_FEATURE ) ) ), weight( 1 )
         {}

         Features                   features;
         value_type                 orientation1;     // dxy
         value_type                 orientation2;     // drz
         core::vector3i             position;
         size_t                       scale;
         float                      weight;        // this will be used to weight the points in the <AffineRegistrationPointBased2d>. By default the algorithm will set it to 1, but a third party could change this value to give more weight to specific points

         void write( std::ostream& o ) const
         {
            core::write<Features>( features, o );
            core::write<value_type>( orientation1, o );
            core::write<value_type>( orientation2, o );
            position.write( o );
            core::write<size_t>( scale, o );
            core::write<float>( weight, o );
         }

         void read( std::istream& i )
         {
            core::read<Features>( features, i );
            core::read<value_type>( orientation1, i );
            core::read<value_type>( orientation2, i );
            position.read( i );
            core::read<size_t>( scale, i );
            core::read<float>( weight, i );
         }

         void print( std::ostream& o ) const
         {
            o << "position=" << position[ 0 ] << " " << position[ 1 ] << " " << position[ 2 ] << std::endl;
            o << "angle=" << orientation1 << " " << orientation2 << std::endl;
            o << "scale=" << scale << std::endl;
            features.print( o );
         }
      };

      typedef core::Buffer1D<Point> Points;

      /**
       @brief Expose the <point>'s <features> array as if it was stored as an array only
       */
      class PointsFeatureWrapper
      {
      public:
         typedef core::Buffer1D<SpeededUpRobustFeatures3d::value_type>  value_type;

      public:
         PointsFeatureWrapper( const Points& points ) : _points( points )
         {}

         size_t size() const
         {
            return _points.size();
         }

         const core::Buffer1D<SpeededUpRobustFeatures3d::value_type>& operator[]( size_t n ) const
         {
            return _points[ n ].features;
         }

      private:
         PointsFeatureWrapper( const PointsFeatureWrapper& );
         PointsFeatureWrapper& operator=( const PointsFeatureWrapper& );

      private:
         const Points& _points;
      };


      /**
       @brief Construct SURF
       @param octaves the number of octaves to analyse. This increases the range of filters exponentially
       @param intervals the number of intervals per octave This increase the filter linearly
       @param threshold the minimal threshold of the hessian. The lower, the more features (but less robust) will be detected
       */
      SpeededUpRobustFeatures3d( size_t octaves = 5, size_t intervals = 4, size_t init_step = 2, value_type threshold = 0.0000012 ) : _threshold( threshold )
      {
         size_t step = init_step;
         for ( size_t o = 1; o <= octaves; ++o )
         {
            for ( size_t i = 1; i <= intervals; ++i )
            {
               const size_t filterSize = core::round( 3 * ( std::pow( 2.0, (int)o ) * i + 1 ) );
               
               if ( _filterSizes.size() == 0 || *_filterSizes.rbegin() < filterSize )
               {
                  _filterSizes.push_back( filterSize );
                  _filterSteps.push_back( step );
               }
            }
            step *= 2;
         }  
      }

      /**
       @param this computes only points of interest (position and scaling) but doesn't compute the features (orientation, feature vector)
       */
      template <class Volume>
      Points computesPoints( const Volume& i )
      {
         std::stringstream ss;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "SURF point detection started..." );
         FastHessianDetPyramid3d pyramid;
         Points points = _computesPoints( i, pyramid );

         core::Timer angle;
         _computeAngle( pyramid.getIntegralImage(), points );
         std::cout << "Angle computation=" << angle.getCurrentTime() << std::endl;

         ss << "Number of points detected=" << points.size();
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         return points;
      }

      /**
       @brief Compute the SURF features
       */
      template <class Volume>
      Points computesFeatures( const Volume& i )
      {
         std::stringstream ss;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "SURF3D feature detection started..." );

         FastHessianDetPyramid3d pyramid;
         Points points = _computesPoints( i, pyramid );
         ss << "Number of points detected=" << points.size() << std::endl;

         core::Timer timeOrientation;
         _computeAngle( pyramid.getIntegralImage(), points );
         ss << "Orientation time=" << timeOrientation.getCurrentTime() << std::endl;

         core::Timer timeFeeatures;
         _computeFeatures( pyramid.getIntegralImage(), points );
         ss << "Features time=" << timeFeeatures.getCurrentTime();

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         return points;
      }

      /**
       @brief Given a list of points (position + scale), compute the features (orientation + feature)
       */
      template <class Volume>
      void computeFeatures( const Volume& i, Points& points )
      {
         FastHessianDetPyramid3d pyramid;
         pyramid.construct( i, _filterSizes, _filterSteps );

         _computeAngle( pyramid.getIntegralImage(), points );
         _computeFeatures( pyramid.getIntegralImage(), points );
      }

   private:
      void _computeFeatures( const IntegralImage3d& image, Points& points ) const
      {
         const int nbPoints = static_cast<int>( points.size() );
         const value_type area_size = NLL_SURF_2D_NB_AREA_PER_FEATURE_SIZE / NLL_SURF_2D_NB_AREA_PER_FEATURE;
         const value_type area_pos_min = - NLL_SURF_2D_NB_AREA_PER_FEATURE / 2 * area_size;
         const value_type area_pos_max =   NLL_SURF_2D_NB_AREA_PER_FEATURE / 2 * area_size;
         const value_type area_pos_center =   area_pos_min + area_size / 2;
         const value_type dd5x5 = (area_size / NLL_SURF_2D_NB_SUBAREA_PER_AREA);

         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            Point& point = points[ n ];

            const value_type scale = static_cast<value_type>( point.scale );

            const value_type x = point.position[ 0 ];
            const value_type y = point.position[ 1 ];
            const value_type z = point.position[ 2 ];

            RotationFromSpherical rotation( point.orientation1, point.orientation2 );
            RotationFromSpherical rotationInv = rotation.createInverse();

            const int size = (int)core::sqr( 2 * scale );

            Matrix cov1( 3, 3 );
            cov1( 0, 0 ) = core::sqr( 2.5 * scale * 2 );
            cov1( 1, 1 ) = core::sqr( 2.5 * scale * 2  );
            cov1( 2, 2 ) = core::sqr( 2.5 * scale * 2  );
            GaussianPdf gauss1( cov1, core::make_buffer1D<value_type>( 0, 0, 0 ) );

            Matrix cov2( 3, 3 );
            cov2( 0, 0 ) = core::sqr( 3.3 * scale * 2  );
            cov2( 1, 1 ) = core::sqr( 3.3 * scale * 2  );
            cov2( 2, 2 ) = core::sqr( 3.3 * scale * 2  );
            GaussianPdf gauss2( cov2, core::make_buffer1D<value_type>( 0, 0, 0 ) );

            size_t count = 0;
            value_type len = 0;

            // (i, j, k) the bottom left corners of the 4x4x4 area, in the unrotated space
            // (cx, cy, cz) the centers of the 4x4x4 area, in the unrotated space
            value_type cz = area_pos_center;
            for ( value_type k = area_pos_min; k < area_pos_max; k += area_size, cz += area_size )
            {
               value_type cy = area_pos_center;
               for ( value_type j = area_pos_min; j < area_pos_max; j += area_size, cy += area_size )
               {
                  value_type cx = area_pos_center;
                  for ( value_type i = area_pos_min; i < area_pos_max; i += area_size, cx += area_size )
                  {
                     // the feature for each 4x4 region
                     value_type dx = 0;
                     value_type dy = 0;
                     value_type dz = 0;
                     value_type mdx = 0;
                     value_type mdy = 0;
                     value_type mdz = 0;

                     // now compute the 5x5x5 points for each 4x4x4 region
                     for ( value_type dk = k; dk < k + area_size; dk += dd5x5 )
                     {
                        for ( value_type dj = j; dj < j + area_size; dj += dd5x5 )
                        {
                           for ( value_type di = i; di < i + area_size; di += dd5x5 )
                           {
                              // center on the rotated axis
                              const core::vector3d pointInRtotatedGrid = rotation.transform( core::vector3d( di * scale, dj * scale, dk * scale ) );
                              const core::vector3i center( static_cast<int>( x + pointInRtotatedGrid[ 0 ] ),
                                                           static_cast<int>( y + pointInRtotatedGrid[ 1 ] ),
                                                           static_cast<int>( z + pointInRtotatedGrid[ 2 ] ) );

                              //Get the gaussian weighted x and y responses
                              const value_type gauss_s1 = gauss1.eval( core::make_buffer1D<value_type>( di - cx, dj - cy, dk - cz ) );

                              core::vector3i bl( static_cast<int>( center[ 0 ] - scale ),
                                                 static_cast<int>( center[ 1 ] - scale ),
                                                 static_cast<int>( center[ 2 ] - scale ) );
                              core::vector3i tr( static_cast<int>( center[ 0 ] + scale ),
                                                 static_cast<int>( center[ 1 ] + scale ),
                                                 static_cast<int>( center[ 2 ] + scale ) );
                              if ( bl[ 0 ] >= 0 && bl[ 1 ] >= 0 && bl[ 2 ] >= 0 &&
                                   tr[ 0 ] < static_cast<int>( image.sizex() ) &&
                                   tr[ 1 ] < static_cast<int>( image.sizey() ) &&
                                   tr[ 2 ] < static_cast<int>( image.sizez() ) )
                              {
                                 const value_type drx = HaarFeatures3d::getValue( HaarFeatures3d::DX,
                                                                                  image,
                                                                                  center,
                                                                                  static_cast<int>( 2 * scale + 1 ) ) / size;
                                 const value_type dry = HaarFeatures3d::getValue( HaarFeatures3d::DY,
                                                                                  image,
                                                                                  center,
                                                                                  static_cast<int>( 2 * scale + 1 ) ) / size;
                                 const value_type drz = HaarFeatures3d::getValue( HaarFeatures3d::DZ,
                                                                                  image,
                                                                                  center,
                                                                                  static_cast<int>( 2 * scale + 1 ) ) / size;

                                 //Get the gaussian weighted x and y responses on the unrotated axis
                                 const core::vector3d rotatedInvFeature = rotationInv.transform( core::vector3d( drx, dry, drz ) );
                                 const value_type rrx = gauss_s1 * rotatedInvFeature[ 0 ];
                                 const value_type rry = gauss_s1 * rotatedInvFeature[ 1 ];
                                 const value_type rrz = gauss_s1 * rotatedInvFeature[ 2 ];

                                 dx += rrx;
                                 dy += rry;
                                 dz += rrz;
                                 mdx += fabs( rrx );
                                 mdy += fabs( rry );
                                 mdz += fabs( rrz );
                              }
                           }
                        }
                     }

                     //Add the values to the descriptor vector
                     const value_type gauss_s2 = gauss2.eval( core::make_buffer1D<value_type>( cx, cy, cz ) );
                     point.features[ count++ ] = dx * gauss_s2;
                     point.features[ count++ ] = dy * gauss_s2;
                     point.features[ count++ ] = dz * gauss_s2;
                     point.features[ count++ ] = mdx * gauss_s2;
                     point.features[ count++ ] = mdy * gauss_s2;
                     point.features[ count++ ] = mdz * gauss_s2;

                     len += ( dx * dx + dy * dy + dz * dz + mdx * mdx + mdy * mdy + mdz * mdz ) * gauss_s2 * gauss_s2;
                  }
               }
            }

            //Convert to Unit Vector
            len = (sqrt( len ) + 1e-7);
            for( size_t i = 0; i < point.features.size(); ++i )
               point.features[ i ] /= len;
         }
      }

      /**
       @brief assign a repeable orientation for each point       

         The dominant orientation is determined for each SURF 
         key point in order to guarantee the invariance of feature 
         description to image rotatio
       */
      
      static void _computeAngle( const IntegralImage3d& i, Points& points )
      {
         const int nbPoints = static_cast<int>( points.size() );

         Matrix cov1( 3, 3 );
         cov1( 0, 0 ) = core::sqr( 2.5 );
         cov1( 1, 1 ) = core::sqr( 2.5 );
         cov1( 2, 2 ) = core::sqr( 2.5 );
         GaussianPdf gaussEval( cov1, core::make_buffer1D<value_type>( 0, 0, 0 ) );

         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            const Point& point = points[ n ];
            const int scale = static_cast<int>( point.scale );

            std::vector<LocalPoint> localPoints;
            localPoints.reserve( 1331 );

            for ( int w = -6; w <= 6; ++w )
            {
               for ( int v = -6; v <= 6; ++v )
               {
                  for ( int u = -6; u <= 6; ++u )
                  {
                     if ( u * u + v * v + w * w < 36 )
                     {
                        // we need to weight the response so that it is more tolerant to the noise. Indeed, the further
                        // away from the centre, the more likely it is to be noisier
                        //const value_type gauss = gaussian( u, v, 2.5 );
                        const value_type gauss = gaussEval.eval( core::make_buffer1D<value_type>( u, v, w ) );
                        const int x = point.position[ 0 ] + u * scale;
                        const int y = point.position[ 1 ] + v * scale;
                        const int z = point.position[ 2 ] + w * scale;
                        const core::vector3i center( x, y, z );

                        const core::vector3i bl( x - 2 * scale, y - 2 * scale, z - 2 * scale );
                        const core::vector3i tr( x + 2 * scale, y + 2 * scale, z + 2 * scale );
                        if ( bl[ 0 ] >= 0 && bl[ 1 ] >= 0 && bl[ 2 ] >= 0 &&
                             tr[ 0 ] < static_cast<int>( i.sizex() ) &&
                             tr[ 1 ] < static_cast<int>( i.sizey() ) &&
                             tr[ 2 ] < static_cast<int>( i.sizez() ) )
                        {
                           const value_type dy = gauss * HaarFeatures3d::getValue( HaarFeatures3d::DY,
                                                                                   i,
                                                                                   center,
                                                                                   static_cast<int>( 4 * scale + 1 ) );
                           const value_type dx = gauss * HaarFeatures3d::getValue( HaarFeatures3d::DX,
                                                                                   i,
                                                                                   center,
                                                                                   static_cast<int>( 4 * scale + 1 ) );
                           const value_type dz = gauss * HaarFeatures3d::getValue( HaarFeatures3d::DZ,
                                                                                   i,
                                                                                   center,
                                                                                   static_cast<int>( 4 * scale + 1 ) );
                           localPoints.push_back( LocalPoint( dx, dy, dz ) );
                        }
                     }
                  }
               }
            }
           
            const int nbLocalPoints = static_cast<int>( localPoints.size() );

            
            //
            // simple averaging seems better than the technique described in the original paper...
            //  
            double dx = 0;
            double dy = 0;
            double dz = 0;
            for ( int nn = 0; nn < nbLocalPoints; ++nn )
            {
               dx += localPoints[ nn ].dx;
               dy += localPoints[ nn ].dy;
               dz += localPoints[ nn ].dz;
            }
            if ( nbLocalPoints )
            {
               dx /= nbLocalPoints;
               dy /= nbLocalPoints;
               dz /= nbLocalPoints;
            }

            double normNotUsed = 0;
            core::cartesianToSphericalCoordinate( dx, dy, dz, normNotUsed, points[ n ].orientation1, points[ n ].orientation2 );
         }
      }

      template <class VolumeT>
      Points _computesPoints( const VolumeT& i, FastHessianDetPyramid3d& pyramid )
      {
         // this constant is to find the gaussian's sigma
         // we know that a filter 9*9 corresponds to a gaussian's sigma = 1.2
         // so for a filter of size X, sigma = 1.2 / 9 * X
         static const value_type scaleFactor = 1.2 / 9;

         size_t nbPoints = 0;

         // each thread can work independently, so allocate an array og points that are not shared
         // between threads
         #ifndef NLL_NOT_MULTITHREADED
            const size_t maxNumberOfThread = omp_get_max_threads();
         #else
            const size_t maxNumberOfThread = 1;
         #endif
         std::vector< std::vector<Point> > bins( maxNumberOfThread );


         core::Timer timePyramid;
         pyramid.construct( i, _filterSizes, _filterSteps );

         {
            std::stringstream ss;
            ss << "Pyramid construction time=" << timePyramid.getCurrentTime();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         timePyramid.start();

         ensure( pyramid.getPyramidDetHessian().size() > 1, "too small!" );
         const size_t nbFilters = static_cast<size_t>( pyramid.getPyramidDetHessian().size() ) - 1; // we don't want the last filter, it will never be "maximal"
         for ( size_t filter = 1; filter < nbFilters; ++filter )
         {
            const FastHessianDetPyramid3d::Volume& f = pyramid.getPyramidDetHessian()[ filter ];
            const int sizex = static_cast<int>( f.sizex() ) - 1;
            const int sizey = static_cast<int>( f.sizey() ) - 1;
            const int sizez = static_cast<int>( f.sizez() ) - 1;

            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp parallel for reduction(+ : nbPoints)
            #endif
            for ( int z = 0; z < sizez; ++z )
            {
               for ( int y = 0; y < sizey; ++y )
               {
                  for ( int x = 0; x < sizex; ++x )
                  {
                     const value_type val = f( x, y, z );
                     if ( val > _threshold )
                     {
                        bool isMax = pyramid.isDetHessianMax( val, x, y, z, filter, filter ) &&
                                     pyramid.isDetHessianMax( val, x, y, z, filter, filter + 1 ) &&
                                     pyramid.isDetHessianMax( val, x, y, z, filter, filter - 1 );
                        if ( isMax )
                        {
                        
                           core::vector4d hessianGradient = pyramid.getHessianGradient( x, y, z, filter );
                        
                           Matrix hessianHessian = pyramid.getHessianHessian( x, y, z, filter );
                        
                           const bool inverted = core::inverse( hessianHessian );
                           core::StaticVector<double, 4> interpolatedPoint = core::mat4Mulv( hessianHessian, hessianGradient ) * -1;
                           if ( inverted /*&& fabs( interpolatedPoint[ 0 ] ) < 0.5 &&
                                            fabs( interpolatedPoint[ 1 ] ) < 0.5 &&
                                            fabs( interpolatedPoint[ 2 ] ) < 0.5*/ )
                           {
                              const int size = static_cast<int>( _filterSizes[ filter ] );
                              // here we need to compute the step between the two scales (i.e., their difference in size and not the step as for the position)
                              const int filterStep = static_cast<int>( _filterSizes[ filter + 1 ] - _filterSizes[ filter ] );

                              // get the actual point in 2D in the orginal space
                              const core::vector3f index = pyramid.getPositionPyramid2Integral( x + (float)interpolatedPoint[ 0 ], y + (float)interpolatedPoint[ 1 ], z + (float)interpolatedPoint[ 2 ], filter );

                              const int px = index[ 0 ];
                              const int py = index[ 1 ];
                              const int pz = index[ 2 ];
                              const int scale = core::round( ( size   + interpolatedPoint[ 3 ]   * filterStep ) * scaleFactor );

                              if ( scale <= 0 || px < 0 || py < 0 || pz < 0 || px >= i.sizex() || py >= i.sizey() || pz >= i.sizez() )
                                 continue;  // again check the boundaries as we might be out!

                              #ifndef NLL_NOT_MULTITHREADED
                              size_t threadId = omp_get_thread_num();
                              #else
                              size_t threadId = 0;
                              #endif
                              bins[ threadId ].push_back( Point( core::vector3i( px, py, pz ), scale ) );

                              ++nbPoints;
                           }
                        }
                     }
                  }
               }
            }
         }

         {
            std::stringstream ss;
            ss << "non maximal suppression and localization=" << timePyramid.getCurrentTime();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         Points points( nbPoints );
         size_t cur = 0;
         for ( size_t bin = 0; bin < bins.size(); ++bin )
         {
            for ( size_t p = 0; p < bins[ bin ].size(); ++p )
            {
               points[ cur++ ] = bins[ bin ][ p ];
            }
         }
         return points;
      }

   public:
      /**
       @brief Given spherical angles, compute the equivalent transformation matrix

       This is just a helper class, public just for the test...
       */
      class RotationFromSpherical
      {
      public:
         typedef double                      value_type;
         typedef core::Matrix<value_type>    Matrix;

      public:
         RotationFromSpherical( value_type phi, value_type theta )
         {
            //
            // TODO: very ugly hack... replace it with a direct computation
            // here we are simply creating a mapping (source points -> transformed points) and estimate the transformation from the point's mapping
            //
            algorithm::EstimatorTransformSimilarityIsometric estimator;
            std::vector<core::vector3d> source;
            std::vector<core::vector3d> target;

            source.push_back( core::vector3d( 1, 0, 0 ) );
            source.push_back( core::vector3d( 2, 0, 0 ) );
         
            std::for_each( source.begin(), source.end(), [&]( const core::vector3d& val )
            {
               core::vector3d spherical = core::cartesianToSphericalCoordinate( val );
               spherical[ 1 ] += phi;
               spherical[ 2 ] += theta;
               target.push_back( core::sphericalToCartesianCoordinate( spherical ) );
            });

            _rotation = estimator.compute( source, target );
         }

         /**
          @brief transform a point p into a new point
          */
         core::vector3d transform( const core::vector3d& p ) const
         {
            return core::transf4( _rotation, p );
         }

         RotationFromSpherical createInverse() const
         {
            Matrix tfm;
            tfm.clone( _rotation );
            const bool inv = core::inverse( tfm );
            ensure( inv, "tfm is not affine!" );
            return RotationFromSpherical( tfm );
         }

      private:
         RotationFromSpherical( const Matrix& m )
         {
            _rotation.clone( m );
         }

      private:
         Matrix _rotation;
      };

   private:
      std::vector<size_t> _filterSizes;
      std::vector<size_t> _filterSteps;
      value_type _threshold;
   };
}
}

#endif