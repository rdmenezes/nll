#include <nll/nll.h>
#include <tester/register.h>

class TestPca
{
public:
   void testPca()
   {
      double pointsRaw[][ 2 ] =
      {
         { 2.5, 2.4 },
         { 0.5, 0.7 },
         { 2.2, 2.9 },
         { 1.9, 2.2 },
         { 3.1, 3.0 },
         { 2.3, 2.7 },
         { 2, 1.6 },
         { 1, 1.1 },
         { 1.5, 1.6 },
         { 1.1, 0.9 }
      };

      double pointsTransformed[][ 2 ] =
      {
         { -0.82, -0.175 },
         { 1.77, 0.14 },
         { -0.99, 0.38 },
         { -0.27, 0.13 },
         { -1.67, -0.20 },
         { -0.91, 0.17 },
         { 0.09, -0.34 },
         { 1.14, 0.04 },
         { 0.43, 0.01 },
         { 1.22, -0.16 }
      };
      typedef std::vector<nll::core::Buffer1D<double> >  Points;
      typedef nll::algorithm::PrincipalComponentAnalysis<Points>  Pca;

      Points points( 10 );
      for ( unsigned n = 0; n < points.size(); ++n )
         points[ n ] = nll::core::Buffer1D<double>( pointsRaw[ n ], 2, false );
      Pca pca;
      bool res = pca.compute( points, 2 );
      TESTER_ASSERT( res );

      nll::core::Matrix<double> transf = pca.getProjection();
      nll::core::transpose( transf );
      for ( unsigned n = 0; n < 10; ++n )
      {
         // test with the expected results
         nll::core::Buffer1D<double> p = pca.process( points[ n ] );
         TESTER_ASSERT( fabs( p[ 0 ] - pointsTransformed[ n ][ 0 ] ) < 0.05 );
         TESTER_ASSERT( fabs( p[ 1 ] - pointsTransformed[ n ][ 1 ] ) < 0.05 );

         // reconstruct the points
         nll::core::Matrix<double> pp( p.getBuf(), 2, 1, false);
         nll::core::Matrix<double> tt = nll::core::mul( transf, pp );
         tt[ 0 ] += pca.getMean()[ 0 ];
         tt[ 1 ] += pca.getMean()[ 1 ];
         TESTER_ASSERT( fabs( tt[ 0 ] - pointsRaw[ n ][ 0 ] ) < 0.05 );
         TESTER_ASSERT( fabs( tt[ 1 ] - pointsRaw[ n ][ 1 ] ) < 0.05 );
      }
   }

   void testPcaFeature()
   {
      typedef std::vector<double>                              Point;
      typedef nll::algorithm::FeatureTransformationPca<Point>  Pca;
      typedef nll::algorithm::Classifier<Point>::Database      Database;
      //typedef Pca::Database                                    Database;

      Database dat;
      dat.add( Database::Sample( nll::core::make_vector<double>( 0, 0 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<double>( 1, 0 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<double>( 2, 0 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<double>( 3, 0 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<double>( 4, 0 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<double>( 0, 1 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<double>( 0, 2 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<double>( 0, 3 ), 0, Database::Sample::LEARNING ) );
      Pca pca;
      Pca pca2;

      nll::algorithm::FeatureTransformation<Point>* pca_t1 = &pca;
      nll::algorithm::FeatureTransformation<Point>* pca_t2 = &pca2;

      pca.compute( dat, 2 );
      pca_t1->write( "pca.bin" );
      pca_t2->read( "pca.bin" );

      nll::core::Matrix<double> mat = pca2.getProjection();
      TESTER_ASSERT( nll::core::equal( mat( 0, 0 ), -1.0 ) );
      TESTER_ASSERT( nll::core::equal( mat( 0, 1 ), 0.0 ) );

      TESTER_ASSERT( nll::core::equal( mat( 1, 0 ), 0.0 ) );
      TESTER_ASSERT( nll::core::equal( mat( 1, 1 ), -1.0 ) );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestPca);
TESTER_TEST(testPca);
TESTER_TEST(testPcaFeature);
TESTER_TEST_SUITE_END();
#endif
