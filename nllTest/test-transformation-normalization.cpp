#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>

class TestNormalization
{
public:
   /**
    */
   void simpleTest()
   {
      typedef std::vector<float> Point;
      typedef nll::algorithm::FeatureTransformationNormalization<Point> Transform;
      typedef nll::algorithm::Classifier<Point>::Database   Database;

      Database dat;
      dat.add( Database::Sample( nll::core::make_vector<float>( 1 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<float>( 3 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<float>( 2 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<float>( 3.5 ), 0, Database::Sample::VALIDATION ) );
      dat.add( Database::Sample( nll::core::make_vector<float>( 3 ), 0, Database::Sample::VALIDATION ) );
      dat.add( Database::Sample( nll::core::make_vector<float>( 3000 ), 0, Database::Sample::TESTING ) );
      dat.add( Database::Sample( nll::core::make_vector<float>( 3000 ), 0, Database::Sample::TESTING ) );

      Transform tfm;
      tfm.compute( dat );
      TESTER_ASSERT( fabs( tfm.getMean()[ 0 ] - 2.5 ) < 1e-10 );
      TESTER_ASSERT( fabs( tfm.getStddev()[ 0 ] - std::sqrt( 0.8 ) ) < 1e-10 );

      Point p( 1 );
      p[ 0 ] = 10;

      Point pres = tfm.process( p );
      TESTER_ASSERT( fabs( pres[ 0 ] - (10 - 2.5) / std::sqrt( 0.8 ) ) < 1e-10 );

      std::stringstream buf;
      tfm.write( buf );

      Transform tfm2;
      tfm2.read( buf );
      Point pres2 = tfm2.process( p );
      TESTER_ASSERT( fabs( pres2[ 0 ] - (10 - 2.5) / std::sqrt( 0.8 ) ) < 1e-10 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNormalization);
TESTER_TEST(simpleTest);
TESTER_TEST_SUITE_END();
#endif
