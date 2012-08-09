#ifndef NLLTEST_UTILS_H_
# define NLLTEST_UTILS_H_

namespace nll
{
namespace test
{
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