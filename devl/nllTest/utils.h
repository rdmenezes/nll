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

      static void Average( Volume& v, const int kernelSize = 5 )
      {
         const int kernelSizeHalf = kernelSize / 2;

         for ( int z = kernelSizeHalf; z + kernelSizeHalf < v.size()[ 2 ]; ++z )
         {
            for ( int y = kernelSizeHalf; y + kernelSizeHalf < v.size()[ 1 ]; ++y )
            {
               for ( int x = kernelSizeHalf; x + kernelSizeHalf < v.size()[ 0 ]; ++x )
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
   };
}
}

#endif