#include "stdafx.h"
#include <time.h>
#include <iostream>
#include <fstream>
#include <nll/nll.h>

# define GMM_TEST_PATH "../../nllTest/data/gmm/"

#pragma warning( push )
#pragma warning( disable:4996 ) // sscanf unsafe function

static const nll::i8* gmm_train[] =
{
   GMM_TEST_PATH "spk01-train.mfc",
   GMM_TEST_PATH "spk02-train.mfc",
   GMM_TEST_PATH "spk03-train.mfc",
   GMM_TEST_PATH "spk04-train.mfc",
   GMM_TEST_PATH "spk05-train.mfc",
   GMM_TEST_PATH "spk06-train.mfc",
   GMM_TEST_PATH "spk07-train.mfc",
   GMM_TEST_PATH "spk08-train.mfc",
   GMM_TEST_PATH "spk09-train.mfc",
   GMM_TEST_PATH "spk10-train.mfc",
};

static const nll::i8* gmm_sample[] =
{
   GMM_TEST_PATH "unk01-test.mfc",
   GMM_TEST_PATH "unk02-test.mfc",
   GMM_TEST_PATH "unk03-test.mfc",
   GMM_TEST_PATH "unk04-test.mfc",
   GMM_TEST_PATH "unk05-test.mfc",
   GMM_TEST_PATH "unk06-test.mfc",
   GMM_TEST_PATH "unk07-test.mfc",
   GMM_TEST_PATH "unk08-test.mfc",
   GMM_TEST_PATH "unk09-test.mfc",
   GMM_TEST_PATH "unk10-test.mfc",
};

static const nll::i32 gmm_results[] =
{
   2,
   0,
   1,
   7,
   9,
   6,
   4,
   5,
   3,
   8
};

class TestNllAlgorithmClassifierGmm
{
public:
   typedef std::vector<float>    Point;
   typedef std::vector<Point>    Points;

   void testReadWriteVector()
   {
      typedef std::vector<nll::ui32> TEST;
      TEST test1(5);
      TEST test2;
      test1[ 0 ] = 0;
      test1[ 1 ] = 10;
      test1[ 2 ] = 1;
      test1[ 3 ] = 2;
      test1[ 4 ] = 3;

      std::ofstream f( NLL_TEST_PATH "data/test1.test", std::ios_base::binary );
      nll::core::write<TEST> (test1, f);
      f.close();

      std::ifstream o( NLL_TEST_PATH "data/test1.test", std::ios_base::binary );
      nll::core::read<TEST> (test2, o);

      TESTER_ASSERT( test1 == test2 );
   }

   inline static Points load(const std::string& file)
   {
	   std::string buf;
	   Points	vs;
	   std::fstream f(file.c_str());

	   assert(f.is_open());
	   while (!f.eof())
	   {
		   Point v(13);
		   getline(f, buf);
		   nll::ui32 nb = sscanf(buf.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f %f", 
			   &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12]);
		   if (f.eof())
			   break;
		   assert(nb == 13); // else missing data
		   vs.push_back(v);
	   }
	   return vs;
   }

   void testGmmClassifier()
   {
      srand(1);
      typedef nll::algorithm::ClassifierGmm<Points>  CGmmTest;
      CGmmTest cgmm;
      CGmmTest::Database dat;

      for ( nll::ui32 n = 0; n < 10; ++n )
      {
         Points ps = load(gmm_train[n]);
         dat.add(CGmmTest::Database::Sample(ps, n, CGmmTest::Database::Sample::LEARNING));
      }
      for ( nll::ui32 n = 0; n < 10; ++n )
      {
         Points ps = load(gmm_sample[n]);
         dat.add(CGmmTest::Database::Sample(ps, gmm_results[ n ], CGmmTest::Database::Sample::TESTING));
      }
      nll::core::Buffer1D<double> param = nll::core::make_buffer1D<double>( 16, 2 );
      cgmm.learn(dat, param);


      CGmmTest::BaseClassifier* classifier = &cgmm;
      CGmmTest::Result result = classifier->test( dat );
      TESTER_ASSERT( result.learningError < 0.01 );
      TESTER_ASSERT( result.testingError < 0.01 );
   }
};

#ifndef DONT_RUN_TEST
   TESTER_TEST_SUITE(TestNllAlgorithmClassifierGmm);
#ifndef DONT_RUN_SLOW_TEST
   TESTER_TEST(testGmmClassifier);
#endif
   TESTER_TEST(testReadWriteVector);
   TESTER_TEST_SUITE_END();
#endif

#pragma warning( pop )
