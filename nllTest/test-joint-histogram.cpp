#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace algorithm
{
}
}

class TestJointHistogram
{
public:
   typedef imaging::VolumeSpatial<ui8>                   Volume;
   typedef imaging::InterpolatorTriLinearDummy<Volume>   Interpolator;

   void testWeigthsInterpolatorDummy()
   {
      Volume v;
      Interpolator interpolator( v );

      float weights[ 8 ];
      int ix, iy, iz;

      //
      // test each corner against expected return index
      //
      {
         interpolator.computeWeights( core::vector3f( 0, 0, 0 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 0;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0.999f, 0, 0 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 1;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0.999f, 0.999f, 0 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 2;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0, 0.999f, 0 ).getBuf(), weights, ix, iy, iz );
         size_t expected = 3;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      // other level
      {
         interpolator.computeWeights( core::vector3f( 0, 0, 0.999f ).getBuf(), weights, ix, iy, iz );
         size_t expected = 4;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0.999f, 0, 0.999f ).getBuf(), weights, ix, iy, iz );
         size_t expected = 5;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0.999f, 0.999f, 0.999f ).getBuf(), weights, ix, iy, iz );
         size_t expected = 6;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      {
         interpolator.computeWeights( core::vector3f( 0, 0.999f, 0.999f ).getBuf(), weights, ix, iy, iz );
         size_t expected = 7;

         for ( size_t n = 0; n < 8; ++n )
         {
            TESTER_ASSERT( core::equal<float>( expected == n, weights[ n ], 1e-2f ) );
         }
      }

      //
      // test the sum is 1
      //
      for ( ui32 n = 0; n < 1000; ++n )
      {
         core::vector3f pos( core::generateUniformDistributionf(0, 1),
                             core::generateUniformDistributionf(0, 1),
                             core::generateUniformDistributionf(0, 1) );
         interpolator.computeWeights( core::vector3f( 0, 0.999f, 0 ).getBuf(), weights, ix, iy, iz );

         const double val = core::generic_norm2<float*, float, 8>( weights );
         TESTER_ASSERT( core::equal<double>( val, 1, 1e-3 ) );
      }
   }

   void testJointHistogramPartial()
   {
      core::Matrix<float> tfmMat = core::identityMatrix<core::Matrix<float>>( 4 );
      tfmMat( 0, 3 ) = 0;
      imaging::TransformationAffine tfm( tfmMat );

      Volume v( core::vector3ui( 512, 512, 256), core::identityMatrix<core::Matrix<float>>( 4 ) );
      //Volume v( core::vector3ui( 8, 8, 8), core::identityMatrix<core::Matrix<float>>( 4 ) );
      for ( Volume::iterator it = v.begin(); it != v.end(); ++it )
      {
         *it = rand() % 8;
      }
      algorithm::JointHistogram jh( 8 );

      for ( ui32 n = 0; n < 10; ++n )
      {
         jh.reset();   

         core::Timer timer;
         algorithm::computeHistogram_partialInterpolation( v, tfm, v, jh );

         
         std::cout << "JointHistogramTime=" << timer.getCurrentTime() << std::endl;
         
         algorithm::SimilarityFunctionSumOfSquareDifferences ssd;
         std::cout << "similarity=" << ssd.evaluate( jh ) << std::endl;

         jh.print( std::cout );

         std::cout << "samples=" << jh.getNbSamples() << std::endl;
         
         TESTER_ASSERT( ( v.getSize()[ 0 ] - 1 ) * ( v.getSize()[ 1 ] - 1 ) * ( v.getSize()[ 2 ] - 1 ) == jh.getNbSamples() );

         for ( size_t y = 0; y < jh.getNbBins(); ++y )
         {
            for ( size_t x = 0; x < jh.getNbBins(); ++x )
            {
               if ( x != y )
               {
                  TESTER_ASSERT( jh( x, y ) == 0 );
               }
            }
         }
      }
   }

   void testJointHistogramNn()
   {
      core::Matrix<float> tfmMat = core::identityMatrix<core::Matrix<float>>( 4 );
      tfmMat( 0, 3 ) = 0;
      imaging::TransformationAffine tfm( tfmMat );

      //Volume v( core::vector3ui( 8, 8, 8), core::identityMatrix<core::Matrix<float>>( 4 ) );
      Volume v( core::vector3ui( 512, 512, 256), core::identityMatrix<core::Matrix<float>>( 4 ) );
      for ( Volume::iterator it = v.begin(); it != v.end(); ++it )
      {
         *it = rand() % 8;
      }
      algorithm::JointHistogram jh( 8 );

      for ( ui32 n = 0; n < 10; ++n )
      {
         jh.reset();   

         core::Timer timer;
         algorithm::computeHistogram_nearestNeighbor( v, tfm, v, jh );

         
         std::cout << "JointHistogramTime=" << timer.getCurrentTime() << std::endl;
         
         algorithm::SimilarityFunctionSumOfSquareDifferences ssd;
         std::cout << "similarity=" << ssd.evaluate( jh ) << std::endl;

         jh.print( std::cout );

         std::cout << "samples=" << jh.getNbSamples() << std::endl;

         TESTER_ASSERT( ( v.getSize()[ 0 ] ) * ( v.getSize()[ 1 ] ) * ( v.getSize()[ 2 ] ) == jh.getNbSamples() );

         for ( size_t y = 0; y < jh.getNbBins(); ++y )
         {
            for ( size_t x = 0; x < jh.getNbBins(); ++x )
            {
               if ( x != y )
               {
                  TESTER_ASSERT( jh( x, y ) == 0 );
               }
            }
         }
      }

   }
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestJointHistogram);
TESTER_TEST(testWeigthsInterpolatorDummy);
TESTER_TEST(testJointHistogramNn);
TESTER_TEST(testJointHistogramPartial);
TESTER_TEST_SUITE_END();
#endif