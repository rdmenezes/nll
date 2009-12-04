#include "stdafx.h"
#include <nll/nll.h>

class TestVolumeResampling
{
public:
   void testResamplingTranslation()
   {
      typedef  nll::imaging::VolumeSpatial<int>  Volume;

      Volume::Matrix tfm1 = nll::core::identityMatrix<Volume::Matrix>( 4 );
      Volume v1( nll::core::vector3ui( 8, 8, 4 ), tfm1 );

      Volume::Matrix tfm2 = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tfm2( 0, 3 ) = -1;
      tfm2( 2, 3 ) = -1;
      Volume v2( nll::core::vector3ui( 8, 8, 4 ), tfm2 );

      int n = 0;
      for ( Volume::iterator it = v1.begin(); it != v1.end(); ++it, ++n )
         *it = n;
      nll::imaging::resampleVolumeNearestNeighbour( v1, v2 );

      int vec[] = {-1, 0, -1};
      for ( int z = 0; z < (int)v2.getSize()[ 2 ]; ++z )
      {
         for ( int y = 0; y < (int)v2.getSize()[ 1 ]; ++y )
         {
            for ( int x = 0; x < (int)v2.getSize()[ 0 ]; ++x )
            {
               if ( x + vec[ 0 ] >= 0 && y + vec[ 1 ] >= 0 && z + vec[ 2 ] >= 0 )
               {
                  TESTER_ASSERT( v1( x + vec[ 0 ], y + vec[ 1 ], z + vec[ 2 ] ) == v2( x, y, z ) );
               }
            }
         }
      }
      nll::imaging::writeVolumeText( v1, NLL_TEST_PATH "data/volume1-resampled-translation-before" );
      nll::imaging::writeVolumeText( v2, NLL_TEST_PATH "data/volume1-resampled-translation" );
   }

   void testResamplingCropping()
   {
      typedef  nll::imaging::VolumeSpatial<int>  Volume;

      Volume::Matrix tfm1 = nll::core::identityMatrix<Volume::Matrix>( 4 );
      Volume v1( nll::core::vector3ui( 8, 8, 4 ), tfm1 );

      Volume::Matrix tfm2 = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tfm2( 0, 3 ) = 0;
      tfm2( 2, 3 ) = 3;
      Volume v2( nll::core::vector3ui( 4, 4, 1 ), tfm2 );

      int n = 0;
      for ( Volume::iterator it = v1.begin(); it != v1.end(); ++it, ++n )
         *it = n;
      nll::imaging::resampleVolumeNearestNeighbour( v1, v2 );

      int vec[] = {0, 0, 3};
      for ( int z = 0; z < (int)v2.getSize()[ 2 ]; ++z )
      {
         for ( int y = 0; y < (int)v2.getSize()[ 1 ]; ++y )
         {
            for ( int x = 0; x < (int)v2.getSize()[ 0 ]; ++x )
            {
               if ( x + vec[ 0 ] >= 0 && y + vec[ 1 ] >= 0 && z + vec[ 2 ] >= 0 )
               {
                  TESTER_ASSERT( v1( x + vec[ 0 ], y + vec[ 1 ], z + vec[ 2 ] ) == v2( x, y, z ) );
               }
            }
         }
      }
      nll::imaging::writeVolumeText( v2, NLL_TEST_PATH "data/volume1-resampled-cropping" );
   }

   void testResamplingSpeed()
   {
      typedef  nll::imaging::VolumeSpatial<float>  Volume;

      Volume::Matrix tfm1 = nll::core::identityMatrix<Volume::Matrix>( 4 );
      Volume v1( nll::core::vector3ui( 512, 512, 386 ), tfm1 );

      Volume::Matrix tfm2 = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tfm2( 0, 0 ) = -2;
      tfm2( 0, 3 ) = 50;
      tfm2( 2, 3 ) = -10;
      Volume v2( nll::core::vector3ui( 512, 512, 386 ), tfm2 );

      float n = 0;
      for ( Volume::iterator it = v1.begin(); it != v1.end(); ++it, ++n )
         *it = n;

      nll::core::Timer t1;
      nll::imaging::resampleVolumeTrilinear( v1, v2 );
      std::cout << "resamplingTrilinearTime(512*512*386)=" << t1.getCurrentTime() << std::endl;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeResampling);
 TESTER_TEST(testResamplingSpeed);
 TESTER_TEST(testResamplingTranslation);
 TESTER_TEST(testResamplingCropping);
TESTER_TEST_SUITE_END();
#endif
