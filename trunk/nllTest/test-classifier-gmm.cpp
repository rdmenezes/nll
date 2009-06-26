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

   /**
    Check the probabilities using 1 gaussian
    */
   void testGmmPrecision1()
   {
      srand( 1 );
      const double precision = 5e-2;
      const unsigned nbPoints = 1000;
      const double mean = 15;
      const double variance = 1;
      std::vector< std::vector<double> > points( nbPoints );
      for ( unsigned n = 0; n < nbPoints; ++n )
      {
         points[ n ] = nll::core::make_vector<double>( nll::core::generateGaussianDistribution( mean, variance ) );
      }
      nll::algorithm::Gmm gmm;
      gmm.em( points, 1, 1, 100, 0.0001 );

      TESTER_ASSERT( nll::core::equal( gmm.getGaussians()[ 0 ].mean[ 0 ], mean, precision ) );
      TESTER_ASSERT( nll::core::equal( gmm.getGaussians()[ 0 ].covariance( 0, 0 ), variance, precision ) );

      const double mean_r = gmm.getGaussians()[ 0 ].mean[ 0 ];
      const double variance_r = gmm.getGaussians()[ 0 ].covariance( 0, 0 );
      for ( unsigned n = 0; n < nbPoints; ++n )
      {
         std::vector<double> val = nll::core::make_vector<double>( nll::core::generateGaussianDistribution( mean, variance ) );
         std::vector< std::vector<double> > point( 1 );
         point[ 0 ] = val;
         double p = exp( gmm.likelihood( point ) );
         const double constval = 1 / ( sqrt( 2 * nll::core::PI ) * sqrt( variance_r ) );
         const double p_ref = constval * exp( -0.5 * ( val[ 0 ] - mean_r ) * ( 1 / variance_r ) * ( val[ 0 ] - mean_r ) );
         TESTER_ASSERT( nll::core::equal( p, p_ref, 0.001 ) );
      }
   }

   void testGmmPrecision2()
   {
      srand( 2 );
      const double precision = 1e-1;
      const unsigned nbPoints = 1000;
      const double meanx = 15;
      const double meany = 15;
      const double variancex = 1;
      const double variancey = 1;
      std::vector< std::vector<double> > points( nbPoints );
      for ( unsigned n = 0; n < nbPoints; ++n )
      {
         const double x = nll::core::generateGaussianDistribution( meanx, variancex );
         const double y = nll::core::generateGaussianDistribution( meany, variancey );
         points[ n ] = nll::core::make_vector<double>( x, y );
      }
      nll::algorithm::Gmm gmm;
      gmm.em( points, 2, 1, 100, 0.0001 );

      TESTER_ASSERT( nll::core::equal( gmm.getGaussians()[ 0 ].mean[ 0 ], meanx, precision ) );
      TESTER_ASSERT( nll::core::equal( gmm.getGaussians()[ 0 ].mean[ 1 ], meany, precision ) );

      for ( unsigned n = 0; n < nbPoints; ++n )
      {
         const double x = nll::core::generateGaussianDistribution( meanx, variancex );
         const double y = nll::core::generateGaussianDistribution( meany, variancey );
         /*
         // in comment: test special distribution where likelihood = 3.1
         const double x = 0.4964;
         const double y = 0.2191;
         */

         std::vector<double> val = nll::core::make_vector<double>( x, y );
         std::vector< std::vector<double> > point( 1 );
         point[ 0 ] = val;
         double p = exp( gmm.likelihood( point ) );

         // compute the multivariate gaussian probability modelled by the gaussian
         /*
         nll::algorithm::Gmm::Matrix cov( 2, 2 );
         cov( 0, 0 ) = 0.067;
         cov( 0, 1 ) = -0.0115;
         cov( 1, 0 ) = -0.0115;
         cov( 1, 1 ) = 0.0395;

         nll::algorithm::Gmm::Vector mean( 2 );
         mean[ 0 ] = 0.5;
         mean[ 1 ] = 0.2328;
         */
         
         nll::algorithm::Gmm::Matrix cov;
         cov.clone( gmm.getGaussians()[ 0 ].covariance );
         nll::algorithm::Gmm::Vector mean;
         mean.clone( gmm.getGaussians()[ 0 ].mean );
         

         double det;
         nll::algorithm::Gmm::Matrix invcov( cov );
         nll::core::inverse( invcov, &det );

         const double constval = 1 / ( pow( 2 * nll::core::PI, 1 ) * sqrt( det ) );
         nll::algorithm::Gmm::Matrix vect( 1, 2 );
         vect( 0, 0 ) = val[ 0 ] - mean[ 0 ];
         vect( 0, 1 ) = val[ 1 ] - mean[ 1 ];
         nll::algorithm::Gmm::Matrix vec( 2, 1 );
         vec( 0, 0 ) = val[ 0 ] - mean[ 0 ];
         vec( 1, 0 ) = val[ 1 ] - mean[ 1 ];
         nll::algorithm::Gmm::Matrix res;
         res = vect * invcov * vec;
         const double p_ref = constval * exp( -0.5 * res( 0, 0 ) );

         TESTER_ASSERT( nll::core::equal( p, p_ref, 0.01 ) );
      }
   }
};

#ifndef DONT_RUN_TEST
   TESTER_TEST_SUITE(TestNllAlgorithmClassifierGmm);
#ifndef DONT_RUN_SLOW_TEST
   TESTER_TEST(testGmmClassifier);
#endif
   TESTER_TEST(testReadWriteVector);
   TESTER_TEST(testGmmPrecision2)
   TESTER_TEST(testGmmPrecision1)
   TESTER_TEST_SUITE_END();
#endif

#pragma warning( pop )
