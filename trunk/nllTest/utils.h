#ifndef NLLTEST_UTILS_H_
# define NLLTEST_UTILS_H_

namespace nll
{
namespace test
{
   /**
    @brief Extract orthogonal MPR at the specified location
    */
   template <class Volume>
   std::vector< core::Image<ui8> > visualizeRegistration( const Volume& source,
                                                          const imaging::LookUpTransformWindowingRGB& lutSource,
                                                          const Volume& target,
                                                          const imaging::LookUpTransformWindowingRGB& lutTarget,
                                                          const imaging::TransformationAffine& tfmSourceToTarget,
                                                          const core::vector3f& positionMprCenterToVisualize )
   {
      typedef typename Volume::value_type value_type;
      std::vector< core::Image<ui8> > vector;

      Volume targetResampled( source.size(), source.getPst() );
      imaging::resampleVolumeTrilinear( target, tfmSourceToTarget, targetResampled );

      const core::vector3f axisX[] =
      {
         core::vector3f( 1, 0, 0 ),
         core::vector3f( 1, 0, 0 ),
         core::vector3f( 0, 1, 0 ),
      };

      const core::vector3f axisY[] =
      {
         core::vector3f( 0, 1, 0 ),
         core::vector3f( 0, 0, 1 ),
         core::vector3f( 0, 0, 1 ),
      };
      const size_t nbAxis = core::getStaticBufferSize( axisX );

      core::vector3f sizeMm( source.getSize()[ 0 ] * source.getSpacing()[ 0 ],
                             source.getSize()[ 1 ] * source.getSpacing()[ 1 ],
                             source.getSize()[ 2 ] * source.getSpacing()[ 2 ] );
      const ui32 maxSizeMm = 2 * (ui32) *std::max_element( sizeMm.getBuf(), sizeMm.getBuf() + 3 );

      for ( size_t n = 0; n < nbAxis; ++n )
      {
         core::Image<ui8>  sourceMpr = test::GetMprForDisplay( source,
                                                               core::vector3ui( maxSizeMm, maxSizeMm, 1 ),
                                                               axisX[ n ],
                                                               axisY[ n ],
                                                               positionMprCenterToVisualize,
                                                               lutSource );
         core::Image<ui8>  targetMpr = test::GetMprForDisplay( targetResampled,
                                                               core::vector3ui( maxSizeMm, maxSizeMm, 1 ),
                                                               axisX[ n ],
                                                               axisY[ n ],
                                                               positionMprCenterToVisualize,
                                                               lutTarget );

         core::Image<ui8>  result( sourceMpr.sizex(), sourceMpr.sizey(), 3 );
         for ( size_t n = 0; n < sourceMpr.size(); ++n )
         {
            const value_type v = sourceMpr[ n ] + targetMpr[ n ];
            result[ n ] = NLL_BOUND( v, 0, 255 );
         }


         vector.push_back( result );
      }

      return vector;
   }

   /**
    @brief Extract a Volume MPR for display
    */
   template <class Volume>
   core::Image<ui8> GetMprForDisplay( const Volume& volume,
                                      const core::vector3ui& size,
                                      const core::vector3f& axisX,
                                      const core::vector3f& axisY,
                                      const core::vector3f& origin,
                                      const imaging::LookUpTransformWindowingRGB& lut )
   {
      typedef typename Volume::value_type             value_type;
      typedef imaging::InterpolatorTriLinear<Volume>  Interpolator;
      typedef imaging::Slice<value_type>              Slice;

      ensure( lut.getNbComponents() == 3, "we need a RGB LUT" );

      core::vector2f spacing( axisX.norm2(), axisY.norm2() );
      imaging::Slice<float> slice( size,
                                   axisX,
                                   axisY,
                                   origin, 
                                   spacing );
      imaging::Mpr<Volume, Interpolator> mpr( volume );
      mpr.getSlice( slice );


      core::Image<ui8> slicei( slice.sizex(), slice.sizey(), 3 );
      for ( size_t y = 0; y < slicei.sizey(); ++y )
      {
         for ( size_t x = 0; x < slicei.sizex(); ++x )
         {
            const float* val = lut.transform( slice( x, y, 0 ) );
            slicei( x, y, 0 ) = (ui8)NLL_BOUND( val[ 0 ], 0, 255 );
            slicei( x, y, 1 ) = (ui8)NLL_BOUND( val[ 1 ], 0, 255 );
            slicei( x, y, 2 ) = (ui8)NLL_BOUND( val[ 2 ], 0, 255 );
         }
      }

      return slicei;
   }

   class VolumeUtils
   {
   public:
      typedef core::Matrix<float>            Matrix;
      typedef imaging::VolumeSpatial<float>  Volume;
      typedef imaging::InterpolatorTriLinear<Volume>  Interpolator;
      typedef imaging::Slice<float>          Slicef;

      typedef imaging::TransformationDenseDeformableField Ddf;
      typedef core::DeformableTransformationRadialBasis<> RbfTransform;
      typedef std::vector<RbfTransform::Rbf> Rbfs;
      typedef RbfTransform::Rbf Rbf;

      static Ddf createDdf( const Matrix& affineTfm, const Matrix& pstTarget, const Rbfs& rbfs, const core::vector3ui ddfSize, const core::vector3f ddfSizeMm )
      {
         RbfTransform tfmRbf( affineTfm, rbfs );
         Ddf ddf = imaging::TransformationDenseDeformableField::create( tfmRbf, pstTarget, ddfSizeMm, ddfSize );
         return ddf;
      }

      static void Average( Volume& v, const int kernelSize = 5 )
      {
         const int kernelSizeHalf = kernelSize / 2;

         for ( int z = kernelSizeHalf; z + kernelSizeHalf < (int)v.size()[ 2 ]; ++z )
         {
            for ( int y = kernelSizeHalf; y + kernelSizeHalf < (int)v.size()[ 1 ]; ++y )
            {
               for ( int x = kernelSizeHalf; x + kernelSizeHalf < (int)v.size()[ 0 ]; ++x )
               {

                  float sum = 0;
                  for ( int dz = -kernelSizeHalf; dz <= kernelSizeHalf; ++dz )
                  {
                     for ( int dy = -kernelSizeHalf; dy <= kernelSizeHalf; ++dy )
                     {
                        for ( int dx = -kernelSizeHalf; dx <= kernelSizeHalf; ++dx )
                        {
                           sum += v( x + dx, y + dy, z + dz );
                        }
                     }
                  }
                  sum /= kernelSize * kernelSize * kernelSize;

                  v( x, y, z ) = sum;
               }
            }
         }
      }

      static void AverageFull( Volume& v, const int kernelSize = 5 )
      {
         const int kernelSizeHalf = kernelSize / 2;

         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int z = 0; z < (int)v.size()[ 2 ]; ++z )
         {
            for ( int y = 0; y < (int)v.size()[ 1 ]; ++y )
            {
               for ( int x = 0; x < (int)v.size()[ 0 ]; ++x )
               {

                  float sum = 0;
                  size_t nbSamples = 0;
                  for ( int dz = -kernelSizeHalf; dz <= kernelSizeHalf; ++dz )
                  {
                     for ( int dy = -kernelSizeHalf; dy <= kernelSizeHalf; ++dy )
                     {
                        for ( int dx = -kernelSizeHalf; dx <= kernelSizeHalf; ++dx )
                        {
                           const size_t xx = x + dx;
                           const size_t yy = y + dy;
                           const size_t zz = z + dz;
                           if ( xx >= 0 && yy >= 0 && zz >= 0 &&
                                xx < v.size()[ 0 ] &&
                                yy < v.size()[ 1 ] &&
                                zz < v.size()[ 2 ] )
                           {
                              sum += v( xx, yy, zz );
                              ++nbSamples;
                           }
                        }
                     }
                  }
                  if ( nbSamples )
                     sum /= nbSamples;

                  v( x, y, z ) = sum;
               }
            }
         }
      }
   };
}
}

#endif