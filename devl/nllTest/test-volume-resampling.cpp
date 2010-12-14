#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

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

   void testResamplingTranslation1()
   {
      typedef  nll::imaging::VolumeSpatial<float>  Volume;

      // target
      Volume::Matrix tfm1 = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tfm1( 0, 3 ) = 10;
      tfm1( 1, 3 ) = 5;
      tfm1( 2, 3 ) = 1;
      Volume target( nll::core::vector3ui( 16, 32, 4 ), tfm1 );

      target( 0, 0, 0 ) = 100;
      target( 10, 0, 0 ) = 10;
      target( 0, 5, 0 ) = 5;
      target( 0, 0, 1 ) = 1;

      // source
      Volume::Matrix tfm2 = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tfm2( 0, 3 ) = 10;
      tfm2( 1, 3 ) = 5;
      tfm2( 2, 3 ) = 1;
      Volume source( nll::core::vector3ui( 15, 20, 6 ), tfm2 );

      nll::imaging::resampleVolumeNearestNeighbour( target, source );

      for ( int y = source.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < source.size()[ 0 ]; ++x )
         {
            std::cout << source( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }

      TESTER_ASSERT( source( 0, 0, 0 ) == 100 );
      TESTER_ASSERT( source( 10, 0, 0 ) == 10 );
      TESTER_ASSERT( source( 0, 5, 0 ) == 5 );
      TESTER_ASSERT( source( 0, 0, 1 ) == 1 );
   }

   void testResamplingTranslation2()
   {
      typedef  nll::imaging::VolumeSpatial<float>  Volume;
      nll::core::vector3f tf( -1, -2, -3 );

      // target
      Volume::Matrix tfm1 = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tfm1( 0, 3 ) = 10;
      tfm1( 1, 3 ) = 5;
      tfm1( 2, 3 ) = 1;
      Volume target( nll::core::vector3ui( 16, 32, 4 ), tfm1 );

      target( 0, 0, 0 ) = 100;
      target( 10, 0, 0 ) = 10;
      target( 0, 5, 0 ) = 5;
      target( 0, 0, 1 ) = 1;

      // source
      Volume::Matrix tfm2 = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tfm2( 0, 3 ) = 10 + tf[ 0 ];
      tfm2( 1, 3 ) = 5 + tf[ 1 ];
      tfm2( 2, 3 ) = 1 + tf[ 2 ];
      Volume source( nll::core::vector3ui( 15, 20, 6 ), tfm2 );

      nll::imaging::resampleVolumeNearestNeighbour( target, source );
      TESTER_ASSERT( source( 0 - (int)tf[ 0 ], 0 - (int)tf[ 1 ], 0 - (int)tf[ 2 ] ) == 100 );
      TESTER_ASSERT( source( 10 - (int)tf[ 0 ], 0 - (int)tf[ 1 ], 0 - (int)tf[ 2 ] ) == 10 );
      TESTER_ASSERT( source( 0 - (int)tf[ 0 ], 5 - (int)tf[ 1 ], 0 - (int)tf[ 2 ] ) == 5 );
      TESTER_ASSERT( source( 0 - (int)tf[ 0 ], 0 - (int)tf[ 1 ], 1 - (int)tf[ 2 ] ) == 1 );
   }

   void testResamplingTranslationRotation()
   {
      typedef  nll::imaging::VolumeSpatial<float>  Volume;

      Volume::Matrix tf = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tf( 0, 3 ) = -1;
      tf( 1, 3 ) = -2;
      tf( 2, 3 ) = 0;
      nll::imaging::TransformationAffine tfm( tf );

      // target
      Volume::Matrix tfm1( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm1, static_cast<float>( nll::core::PI / 2 ) );

      tfm1( 0, 3 ) = 0;
      tfm1( 1, 3 ) = 0;
      tfm1( 2, 3 ) = 0;
      Volume target( nll::core::vector3ui( 16, 32, 4 ), tfm1 );

      target( 0, 0, 0 ) = 100;
      target( 10, 0, 0 ) = 10;
      target( 0, 5, 0 ) = 5;
      target( 0, 0, 1 ) = 0;

      // source
      Volume::Matrix tfm2( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm2, /*static_cast<float>( nll::core::PI / 2 )*/ 0 );
      tfm2( 0, 3 ) = 0;
      tfm2( 1, 3 ) = 0;
      tfm2( 2, 3 ) = 0;
      Volume source( nll::core::vector3ui( 15, 20, 6 ), tfm2 );

      nll::imaging::resampleVolumeNearestNeighbour( target, source, tfm );

      for ( int y = source.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < source.size()[ 0 ]; ++x )
         {
            std::cout << source( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      TESTER_ASSERT( source( 1, 2, 0 ) == 100 );
      TESTER_ASSERT( source( 6, 2, 0 ) == 5 );
   }

   void testResamplingTranslationRotation2()
   {
      typedef  nll::imaging::VolumeSpatial<float>  Volume;

      Volume::Matrix tf = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tf( 0, 3 ) = -5;
      tf( 1, 3 ) = 0;
      tf( 2, 3 ) = 0;
      nll::imaging::TransformationAffine tfm( tf );

      // target
      Volume::Matrix tfm1( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm1, static_cast<float>( -nll::core::PI / 2 ) );

      tfm1( 0, 3 ) = 0;
      tfm1( 1, 3 ) = 0;
      tfm1( 2, 3 ) = 0;
      Volume target( nll::core::vector3ui( 16, 32, 4 ), tfm1 );

      target( 0, 0, 0 ) = 100;
      target( 10, 0, 0 ) = 10;
      target( 0, 5, 0 ) = 5;
      target( 0, 0, 1 ) = 0;

      // source
      Volume::Matrix tfm2( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm2, /*static_cast<float>( nll::core::PI / 2 )*/ 0 );
      tfm2( 0, 3 ) = 0;
      tfm2( 1, 3 ) = 0;
      tfm2( 2, 3 ) = 0;
      Volume source( nll::core::vector3ui( 15, 20, 6 ), tfm2 );

      nll::imaging::resampleVolumeNearestNeighbour( target, source, tfm );

      for ( int y = source.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < source.size()[ 0 ]; ++x )
         {
            std::cout << source( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      TESTER_ASSERT( source( 0, 0, 0 ) == 5 );
      TESTER_ASSERT( source( 5, 0, 0 ) == 100 );
      TESTER_ASSERT( source( 5, 10, 0 ) == 10 );
   }

   void testResamplingTranslationRotation3()
   {
      typedef  nll::imaging::VolumeSpatial<float>  Volume;

      Volume::Matrix tf = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tf( 0, 3 ) = 0;
      tf( 1, 3 ) = 0;
      tf( 2, 3 ) = 0;
      nll::imaging::TransformationAffine tfm( tf );

      // target
      Volume::Matrix tfm1( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm1, static_cast<float>( -nll::core::PI / 2 ) );

      tfm1( 0, 3 ) = 0;
      tfm1( 1, 3 ) = 0;
      tfm1( 2, 3 ) = 0;
      Volume target( nll::core::vector3ui( 16, 32, 4 ), tfm1 );

      target( 0, 0, 0 ) = 100;
      target( 10, 0, 0 ) = 10;
      target( 0, 5, 0 ) = 5;
      target( 0, 0, 1 ) = 0;

      // source
      Volume::Matrix tfm2( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm2, /*static_cast<float>( nll::core::PI / 2 )*/ 0 );
      tfm2( 0, 3 ) = -5;
      tfm2( 1, 3 ) = 0;
      tfm2( 2, 3 ) = 0;
      Volume source( nll::core::vector3ui( 15, 20, 6 ), tfm2 );

      nll::imaging::resampleVolumeNearestNeighbour( target, source, tfm );

      for ( int y = source.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < source.size()[ 0 ]; ++x )
         {
            std::cout << source( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      TESTER_ASSERT( source( 0, 0, 0 ) == 5 );
      TESTER_ASSERT( source( 5, 0, 0 ) == 100 );
      TESTER_ASSERT( source( 5, 10, 0 ) == 10 );
   }

   void testResamplingTranslationRotation4()
   {
      typedef  nll::imaging::VolumeSpatial<float>  Volume;

      Volume::Matrix tf = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tf( 0, 3 ) = 0;
      tf( 1, 3 ) = 0;
      tf( 2, 3 ) = 0;
      nll::imaging::TransformationAffine tfm( tf );

      // target
      Volume::Matrix tfm1( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm1, static_cast<float>( -nll::core::PI / 2 ) );

      tfm1( 0, 3 ) = 5;
      tfm1( 1, 3 ) = 0;
      tfm1( 2, 3 ) = 0;
      Volume target( nll::core::vector3ui( 16, 32, 4 ), tfm1 );

      target( 0, 0, 0 ) = 100;
      target( 10, 0, 0 ) = 10;
      target( 0, 5, 0 ) = 5;
      target( 0, 0, 1 ) = 0;

      // source
      Volume::Matrix tfm2( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm2, /*static_cast<float>( nll::core::PI / 2 )*/ 0 );
      tfm2( 0, 3 ) = 0;
      tfm2( 1, 3 ) = 0;
      tfm2( 2, 3 ) = 0;
      Volume source( nll::core::vector3ui( 15, 20, 6 ), tfm2 );

      nll::imaging::resampleVolumeNearestNeighbour( target, source, tfm );

      for ( int y = source.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < source.size()[ 0 ]; ++x )
         {
            std::cout << source( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      TESTER_ASSERT( source( 0, 0, 0 ) == 5 );
      TESTER_ASSERT( source( 5, 0, 0 ) == 100 );
      TESTER_ASSERT( source( 5, 10, 0 ) == 10 );
   }

   void testResamplingSpacing()
   {
      typedef  nll::imaging::VolumeSpatial<float>  Volume;

      Volume::Matrix tf = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tf( 0, 3 ) = 0;
      tf( 1, 3 ) = 0;
      tf( 2, 3 ) = 0;
      nll::imaging::TransformationAffine tfm( tf );

      // target
      Volume::Matrix tfm1( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm1, static_cast<float>( 0 ) );

      tfm1( 0, 3 ) = 0;
      tfm1( 1, 3 ) = 0;
      tfm1( 2, 3 ) = 0;
      Volume target( nll::core::vector3ui( 16, 32, 4 ), tfm1 );

      target( 0, 0, 0 ) = 100;
      target( 10, 0, 0 ) = 10;
      target( 0, 5, 0 ) = 5;
      target( 0, 0, 1 ) = 0;

      // source
      Volume::Matrix tfm2( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm2, 0 );
      tfm2( 0, 3 ) = -1;
      tfm2( 1, 3 ) = -1;
      tfm2( 2, 3 ) = 0;

      tfm2( 0, 0 ) = 0.5;
      tfm2( 1, 1 ) = 0.5;
      tfm2( 2, 2 ) = 0.5;

      Volume source( nll::core::vector3ui( 15, 20, 6 ), tfm2 );

      nll::imaging::resampleVolumeNearestNeighbour( target, source, tfm );

      for ( int y = source.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < source.size()[ 0 ]; ++x )
         {
            std::cout << source( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      TESTER_ASSERT( source( 1, 1, 0 ) == 100 );
      TESTER_ASSERT( source( 2, 1, 0 ) == 100 );
      TESTER_ASSERT( source( 1, 2, 0 ) == 100 );
      TESTER_ASSERT( source( 2, 2, 0 ) == 100 );

      TESTER_ASSERT( source( 1, 2 * 5 + 1, 0 ) == 5 );
      TESTER_ASSERT( source( 2, 2 * 5 + 1, 0 ) == 5 );
      TESTER_ASSERT( source( 1, 2 * 5 + 1 + 1, 0 ) == 5 );
      TESTER_ASSERT( source( 2, 2 * 5 + 1 + 1, 0 ) == 5 );
   }

   void testResamplingSpacing2()
   {
      typedef  nll::imaging::VolumeSpatial<float>  Volume;

      Volume::Matrix tf = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tf( 0, 3 ) = 0;
      tf( 1, 3 ) = 0;
      tf( 2, 3 ) = 0;
      nll::imaging::TransformationAffine tfm( tf );

      // target
      Volume::Matrix tfm1( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm1, static_cast<float>( 0 ) );

      tfm1( 0, 3 ) = 0;
      tfm1( 1, 3 ) = 0;
      tfm1( 2, 3 ) = 0;

      tfm1( 0, 0 ) = 0.5;
      tfm1( 1, 1 ) = 0.5;
      tfm1( 2, 2 ) = 0.5;
      Volume target( nll::core::vector3ui( 16, 32, 4 ), tfm1 );

      target( 0, 0, 0 ) = 100;
      target( 10, 0, 0 ) = 10;
      target( 0, 5, 0 ) = 5;
      target( 0, 0, 1 ) = 0;

      // source
      Volume::Matrix tfm2( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm2, 0 );
      tfm2( 0, 3 ) = -1;
      tfm2( 1, 3 ) = -1;
      tfm2( 2, 3 ) = 0;

      tfm2( 0, 0 ) = 0.5;
      tfm2( 1, 1 ) = 0.5;
      tfm2( 2, 2 ) = 0.5;

      Volume source( nll::core::vector3ui( 15, 20, 6 ), tfm2 );

      nll::imaging::resampleVolumeNearestNeighbour( target, source, tfm );

      for ( int y = source.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < source.size()[ 0 ]; ++x )
         {
            std::cout << source( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      TESTER_ASSERT( source( 1, 1, 0 ) == 0 );
      TESTER_ASSERT( source( 2, 1, 0 ) == 0 );
      TESTER_ASSERT( source( 1, 2, 0 ) == 0 );
      TESTER_ASSERT( source( 2, 2, 0 ) == 100 );

      TESTER_ASSERT( source( 1, 5 + 1, 0 ) == 0 );
      TESTER_ASSERT( source( 2, 5 + 1, 0 ) == 0 );
      TESTER_ASSERT( source( 1, 5 + 1 + 1, 0 ) == 0 );
      TESTER_ASSERT( source( 2, 5 + 1 + 1, 0 ) == 5 );
   }

   void testResamplingTranslationRotation5()
   {
      typedef  nll::imaging::VolumeSpatial<float>  Volume;

      Volume::Matrix tf = nll::core::identityMatrix<Volume::Matrix>( 4 );
      nll::core::matrix4x4RotationZ( tf, static_cast<float>( nll::core::PI/2 ) );
      tf( 0, 3 ) = -7;
      tf( 1, 3 ) = 0;
      tf( 2, 3 ) = 0;
      nll::imaging::TransformationAffine tfm( tf );

      // target
      Volume::Matrix tfm1( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm1, static_cast<float>( 0 ) );

      tfm1( 0, 3 ) = -2;
      tfm1( 1, 3 ) = -1;
      tfm1( 2, 3 ) = 0;
      Volume target( nll::core::vector3ui( 16, 32, 4 ), tfm1 );

      target( 0, 0, 0 ) = 100;
      target( 10, 0, 0 ) = 10;
      target( 0, 5, 0 ) = 5;
      target( 0, 0, 1 ) = 0;

      // source
      Volume::Matrix tfm2( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm2, 0 );
      tfm2( 0, 3 ) = 0;
      tfm2( 1, 3 ) = -1;
      tfm2( 2, 3 ) = 0;
      Volume source( nll::core::vector3ui( 15, 20, 6 ), tfm2 );

      nll::imaging::resampleVolumeNearestNeighbour( target, source, tfm );

      for ( int y = source.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( unsigned x = 0; x < source.size()[ 0 ]; ++x )
         {
            std::cout << source( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
      TESTER_ASSERT( source( 0, 0, 0 ) == 5 );
      TESTER_ASSERT( source( 5, 0, 0 ) == 100 );
      TESTER_ASSERT( source( 5, 10, 0 ) == 10 );
   }


   void testResamplingOutput()
   {
      const std::string volname = NLL_TEST_PATH "data/medical/test1.mf2";
      typedef nll::imaging::VolumeSpatial<float>           Volume;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume>   Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator>       Mpr;

      nll::core::Matrix<float> tfm( 4, 4 );
      nll::core::matrix4x4RotationZ( tfm, nll::core::PI * 0.5);
      tfm( 1, 3 ) = -100;
      nll::imaging::TransformationAffine atfm( tfm );


      std::cout << "loadind..." << std::endl;
      Volume volume;
      nll::imaging::loadSimpleFlatFile( volname, volume );
      volume.setOrigin( nll::core::vector3f( 0, 0, 0 ) );

      Volume volumeResampled( nll::core::vector3ui( 8, 8, 8 ), volume.getPst() );
      volumeResampled.setOrigin( nll::core::vector3f( 0, 0, 0 ) );
      nll::imaging::resampleVolumeNearestNeighbour( volume, volumeResampled, atfm );

      std::cout << "loaded" << std::endl;
      Mpr mpr( volumeResampled );

      Mpr::Slice slice( nll::core::vector3ui( 1024, 1024, 1 ),
                        nll::core::vector3f( 1, 0, 0 ),
                        nll::core::vector3f( 0, 1, 0 ),
                        nll::core::vector3f( 0, 0, 0 ),
                        nll::core::vector2f( 1.0f, 1.0f ) );


      nll::core::Timer mprTime;
      mpr.getSlice( slice );
      mprTime.end();

      nll::core::Image<nll::i8> bmp( slice.size()[ 0 ], slice.size()[ 1 ], 1 );
      for ( unsigned y = 0; y < bmp.sizey(); ++y )
         for ( unsigned x = 0; x < bmp.sizex(); ++x )
            bmp( x, y, 0 ) = (nll::i8)NLL_BOUND( (double)slice( x, y, 0 ), 0, 255 );
      bmp( bmp.sizex() / 2, bmp.sizey() / 2, 0 ) = 255;
      bmp( bmp.sizex() / 2 - 1, bmp.sizey() / 2, 0 ) = 255;
      bmp( bmp.sizex() / 2 + 1, bmp.sizey() / 2, 0 ) = 255;
      bmp( bmp.sizex() / 2, bmp.sizey() / 2 - 1, 0 ) = 255;
      bmp( bmp.sizex() / 2, bmp.sizey() / 2 + 1, 0 ) = 255;
      nll::core::extend( bmp, 3 );
      nll::core::writeBmp( bmp, NLL_TEST_PATH "data/mprtfm.bmp" );

      nll::imaging::writeVolumeText( volumeResampled, "D:/Devel/sandbox/nllTest/data/test" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeResampling);
 TESTER_TEST(testResamplingOutput);
 
 TESTER_TEST(testResamplingTranslation);
 TESTER_TEST(testResamplingCropping);
 TESTER_TEST(testResamplingTranslation1);
 TESTER_TEST(testResamplingTranslation2);
 TESTER_TEST(testResamplingSpacing);
 TESTER_TEST(testResamplingSpacing2);
 /*
 // TODO: UPDATE THE ROTATION TEST
 TESTER_TEST(testResamplingTranslationRotation);
 TESTER_TEST(testResamplingTranslationRotation2);
 TESTER_TEST(testResamplingTranslationRotation3);
 TESTER_TEST(testResamplingTranslationRotation4);
 TESTER_TEST(testResamplingTranslationRotation5);
 */
 TESTER_TEST(testResamplingSpeed);
 
TESTER_TEST_SUITE_END();
#endif
