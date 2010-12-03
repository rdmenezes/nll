#include <nll/nll.h>
#include <tester/register.h>
#include "database-builder.h"
#include "problem-builder-gmm.h"

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

   void testGmmProblem()
   {
      srand( 1 );
      ProblemBuilderGmm pbm;
      pbm.generate( 3, 10, 2, 15 );
      core::Image<ui8> i = pbm.generateMap();

      ProblemBuilderGmm::Database dat = pbm.generateSamples( 650 );
      pbm.printMap( i, dat );
      core::writeBmp( i, "c:/out.bmp" );

      typedef nll::algorithm::ClassifierRbf< ProblemBuilderGmm::Database::Sample::Input > Rbf;
      Rbf rbf;

      rbf.learn( dat, core::make_buffer1D<double>( 60, 0.5, 30 ) );
      core::Image<ui8> ii = pbm.printClassifier( rbf );
      pbm.printMap( ii, dat );
      core::writeBmp( ii, "c:/outc.bmp" );

      Rbf::Result r = rbf.test( dat );

      std::cout << "rbf testing error=" << r.testingError << std::endl;
      std::cout << "rbf learning error=" << r.learningError << std::endl;

      TESTER_ASSERT( r.testingError < 0.09 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRBFNetwork);
TESTER_TEST(testSimple);
TESTER_TEST(testApproximation);
TESTER_TEST(testGmmProblem);
TESTER_TEST_SUITE_END();
#endif
