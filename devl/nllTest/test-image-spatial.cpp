#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

class TestImageSpacial
{
   typedef core::Image<int>         ImageNormal;
   typedef core::Image<ui8>         ImageNormalc;
   typedef core::ImageSpatial<int>  Image;
   typedef core::ImageSpatial<ui8>  Imagec;
   typedef core::Matrix<float>      Matrix;
   typedef core::Matrix<double>     Matrixd;

public:
   void testBasic()
   {
      Matrix tfm = core::identityMatrix<Matrix>( 3 );
      Image i1;
      ImageNormal n1( 15, 30, 1 );

      Image i2( n1, tfm );

      Image i3( 2, 3, 4, tfm );

      TESTER_ASSERT( i2.sizex() == 15 );
   }

   void testResampling()
   {
      Matrix pst1 = core::identityMatrix<Matrix>( 3 );
      Matrix pst2 = core::identityMatrix<Matrix>( 3 );
      Matrix tfm = core::identityMatrix<Matrix>( 3 );
      Matrixd tfmd = core::identityMatrix<Matrixd>( 3 );;
      tfm( 0, 2 ) = 20;

      ImageNormalc i;
      core::readBmp( i, NLL_TEST_PATH "data/feature/i2.bmp" );
      Imagec i1( i, pst1 );
      Imagec i2( i.sizex(), i.sizey(), 3, pst2 );
      ImageNormalc ib( i.sizex(), i.sizey(), 3 );

      core::Timer t1;
      core::resampleNearest( i1, tfm, i2 );
      std::cout << "Time=" << t1.getCurrentTime() << std::endl;
      //core::writeBmp( i2, "c:/tmp/i1.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestImageSpacial);
TESTER_TEST(testBasic);
TESTER_TEST(testResampling);
TESTER_TEST_SUITE_END();
#endif
