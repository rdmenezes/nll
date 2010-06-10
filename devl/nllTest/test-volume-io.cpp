#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

class TestVolumeIo
{
public:

   void testBinaryIO()
   {
      typedef nll::imaging::VolumeSpatial<double> Volume;

      Volume::Matrix pst = nll::core::identityMatrix<Volume::Matrix>( 4 );
      pst( 0, 0 ) = 2;
      pst( 0, 3 ) = -10.5f;
      pst( 1, 3 ) = 10;
      pst( 2, 3 ) = 10.8f;

      nll::imaging::VolumeSpatial<double> vol( nll::core::vector3ui( 4, 8, 16 ), pst );

      double n = 0;
      for ( Volume::iterator it = vol.begin(); it != vol.end(); ++it, ++n )
         *it = n;

      bool saved = nll::imaging::writeVolumeBinary( vol, NLL_TEST_PATH "data/volumeExport" );
      TESTER_ASSERT( saved );

      nll::imaging::VolumeSpatial<double> vol2;
      bool loaded = nll::imaging::readVolumeBinary( vol2, NLL_TEST_PATH "data/volumeExport" );
      TESTER_ASSERT( loaded );

      TESTER_ASSERT( vol2.getSize() == vol.getSize() );
      TESTER_ASSERT( vol2.getPst().equal( vol.getPst() ) );
      
      for ( unsigned z = 0; z < vol.getSize()[ 2 ]; ++z )
      {
         for ( unsigned y = 0; y < vol.getSize()[ 1 ]; ++y )
         {
            for ( unsigned x = 0; x < vol.getSize()[ 0 ]; ++x )
            {
               TESTER_ASSERT( nll::core::equal( vol( x, y, z ), vol2( x, y, z ) ) );
            }
         }
      }
   }

   void testTextIO()
   {
      typedef nll::imaging::VolumeSpatial<double> Volume;

      Volume::Matrix pst = nll::core::identityMatrix<Volume::Matrix>( 4 );
      pst( 0, 0 ) = 2;
      pst( 0, 3 ) = -10.5f;
      pst( 1, 3 ) = 10;
      pst( 2, 3 ) = 10.8f;

      nll::imaging::VolumeSpatial<double> vol( nll::core::vector3ui( 4, 8, 16 ), pst );

      double n = 0;
      for ( Volume::iterator it = vol.begin(); it != vol.end(); ++it, ++n )
         *it = n;

      bool saved = nll::imaging::writeVolumeText( vol, NLL_TEST_PATH "data/volumeExportText" );
      TESTER_ASSERT( saved );

      nll::imaging::VolumeSpatial<double> vol2;
      bool loaded = nll::imaging::readVolumeText( vol2, NLL_TEST_PATH "data/volumeExportText" );
      TESTER_ASSERT( loaded );

      TESTER_ASSERT( vol2.getSize() == vol.getSize() );
      TESTER_ASSERT( vol2.getPst().equal( vol.getPst() ) );
      
      for ( unsigned z = 0; z < vol.getSize()[ 2 ]; ++z )
      {
         for ( unsigned y = 0; y < vol.getSize()[ 1 ]; ++y )
         {
            for ( unsigned x = 0; x < vol.getSize()[ 0 ]; ++x )
            {
               TESTER_ASSERT( nll::core::equal( vol( x, y, z ), vol2( x, y, z ) ) );
            }
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeIo);
TESTER_TEST(testBinaryIO);
TESTER_TEST(testTextIO);
TESTER_TEST_SUITE_END();
#endif