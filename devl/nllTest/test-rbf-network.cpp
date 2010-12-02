#include <nll/nll.h>
#include <tester/register.h>
#include "database-builder.h"

using namespace nll;



class TestRBFNetwork
{
   typedef nll::core::Buffer1D<double>             Point;
   typedef nll::algorithm::ClassifierMlp<Point>    Mlp;
   typedef Mlp::Database                           Database;

public:
   void testSimple()
   {
      srand( 1 );
      typedef core::Database< core::ClassificationSample<Point, Point> > DatabaseRbf;

      Database dat = loadDatabaseSpect<Point>();
      const ui32 nbClass = core::getNumberOfClass( dat );

      // encode the class to an array [0..nbClass]
      DatabaseRbf datRbf;
      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         Point out( nbClass );
         for ( ui32 nn = 0; nn < nbClass; ++nn )
         {
            out[ nn ] = ( dat[ n ].output == nn );               
         }
         datRbf.add( DatabaseRbf::Sample( dat[ n ].input, out, (DatabaseRbf::Sample::Type)dat[ n ].type ) );
      }

      
      //algorithm::RadialBasisNetwork<algorithm::FunctionSimpleIdentity> rbf;
      algorithm::RadialBasisNetwork<> rbf;
      const double param = 0.51;
      TESTER_ASSERT( rbf.learn( datRbf, 10, param, param , param, 10, 15 ) <= 10 );

      // IO
      std::stringstream ss;
      rbf.write( ss );

      algorithm::RadialBasisNetwork<> rbf2;
      rbf2.read( ss );
   }

   void testApproximation()
   {
      srand( 1 );
      typedef core::Database< core::ClassificationSample<Point, Point> > DatabaseRbf;

      // encode the class to an array [0..nbClass]
      DatabaseRbf datRbf;
      int nn = 0;
      for ( double n = 0; n < 1000; ++n, ++nn )
      {
         const double val = n / 1000;
         datRbf.add( DatabaseRbf::Sample( core::make_buffer1D<double>( val ), 
                                          core::make_buffer1D<double>( cos( val ) ),
                                          (DatabaseRbf::Sample::Type)(nn%2) ) );
      }

      //algorithm::RadialBasisNetwork<algorithm::FunctionSimpleIdentity> rbf;
      algorithm::RadialBasisNetwork<> rbf;

      const double param = 0.4;
      TESTER_ASSERT( rbf.learn( datRbf, 15, param, param, param, 0.1, 15 ) <= 0.1 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRBFNetwork);
TESTER_TEST(testSimple);
TESTER_TEST(testApproximation);
TESTER_TEST_SUITE_END();
#endif
