#include <tester/register.h>
#include <mvvPlatform/font.h>

using namespace mvv::platform;
using namespace mvv;

struct TestFont
{
   void test1()
   {
      std::vector<char> chars;
      for ( int n = 0; n < 256; ++n )
         chars.push_back( static_cast<char>( n ) );
      FontBitmapMatrix font( "../../nllTest/data/font/bitmapfont1_24.bmp", nll::core::vector2ui( 16, 16 ), nll::core::vector2ui( 16, 16 ), chars );
      font.setSize( 32 );
      font.setColor( nll::core::vector3uc( 0, 255, 255 ) );
      
      std::string v = "         ";
      double time = 0;
      for ( ui32 n = 0; n < 256 - 10; ++n )
      {
         Image i( font.getSize() * 8, 64, 3 );
         std::cout << "write=" << n << std::endl;
         v[ 0 ] = static_cast<char>( n + 0 );
         v[ 1 ] = static_cast<char>( n + 1 );
         v[ 2 ] = static_cast<char>( n + 2 );
         v[ 3 ] = static_cast<char>( n + 3 );
         v[ 4 ] = static_cast<char>( n + 4 );
         v[ 5 ] = static_cast<char>( n + 5 );
         v[ 6 ] = static_cast<char>( n + 6 );
         v[ 7 ] = static_cast<char>( n + 7 );
         v[ 8 ] = static_cast<char>( n + 8 );
         nll::core::Timer t;
         font.write( v, nll::core::vector2ui( 0, 0 ), i );
         time += t.getCurrentTime();
         nll::core::writeBmp( i, "c:/tmp/font-" + nll::core::val2str( n ) + ".bmp" );
      }
      std::cout << "Total time=" << time << std::endl;
   }

};

TESTER_TEST_SUITE(TestFont);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();