#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;


class TestVolumeMapper
{
   typedef core::Matrix<float>            Matrix;
   typedef imaging::VolumeSpatial<float>  Volume;
   typedef imaging::InterpolatorTriLinear<Volume>  Interpolator;

public:
   void testSimpleMapping()
   {
       Matrix pstTarget = core::createTransformationAffine3D(core::vector3f( -10, -20, -30 ),
                                                             core::vector3f( 0.1, 0, 0 ),
                                                             core::vector3f( 0, 0, 0 ),
                                                             core::vector3f( 1, 1.05, 0.95 ) );
       Matrix pstResampled = core::createTransformationAffine3D(core::vector3f( 1, -2, -3 ),
                                                             core::vector3f( 0, 0.1, 0 ),
                                                             core::vector3f( 0, 0, 0 ),
                                                             core::vector3f( 1.1, 1, 0.95 ) );
       Matrix affineTfm = core::createTransformationAffine3D(core::vector3f(-5, -0, -1 ),
                                                             core::vector3f( 0, 0, -0.15 ),
                                                             core::vector3f( 0, 0, 0 ),
                                                             core::vector3f( 1, 1.1, 1.03 ) );

      Volume resampled( core::vector3ui( 256, 512, 300 ), pstResampled );
      Volume target( core::vector3ui( 256, 512, 300 ), pstTarget );

      for ( Volume::iterator it = target.begin(); it != target.end(); ++it )
      {
         *it = core::generateUniformDistribution( 100, 500 );
      }      

      imaging::VolumeTransformationProcessorResampler<Volume, Interpolator> resampler( target, resampled );
      imaging::VolumeTransformationMapper mapper;
      mapper.run( resampler, target, affineTfm, resampled );

      // these values were computed using this library and are for regression tests only (it is assumed correct...)
      {
         const float v = resampled( 0, 0, 0 );
         TESTER_ASSERT( core::equal<float>( v, 397.794, 1e-2 ) );
      }
      {
         const float v = resampled( 8, 0, 0 );
         TESTER_ASSERT( core::equal<float>( v, 328.952, 1e-2 ) );
      }
      {
         const float v = resampled( 0, 9, 0 );
         TESTER_ASSERT( core::equal<float>( v, 347.456, 1e-2 ) );
      }
      {
         const float v = resampled( 0, 0, 10 );
         TESTER_ASSERT( core::equal<float>( v, 313.788, 1e-2 ) );
      }
      {
         const float v = resampled( 12, 30, 40 );
         TESTER_ASSERT( core::equal<float>( v, 338.652, 1e-2 ) );
      }
      {
         const float v = resampled( 150, 80, 50 );
         TESTER_ASSERT( core::equal<float>( v, 231.93, 1e-2 ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeMapper);
TESTER_TEST(testSimpleMapping);
TESTER_TEST_SUITE_END();
#endif