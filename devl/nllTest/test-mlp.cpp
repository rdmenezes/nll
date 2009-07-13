#include "stdafx.h"
#include <nll/nll.h>



class TestMlp
{
public:
   void testMlp1()
   {
      srand( time( 0 ) );
      typedef nll::algorithm::Mlp<nll::algorithm::FunctionSimpleDifferenciableSigmoid> Mlp;

      typedef nll::core::Database< nll::core::ClassificationSample< std::vector<double>, std::vector<double> > > Database;
      Database dat;
      dat.add( Database::Sample( nll::core::make_vector<double>( 0, 0 ), 
                                 nll::core::make_vector<double>( 0 ),
                                 Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<double>( 1, 1 ), 
                                 nll::core::make_vector<double>( 0 ),
                                 Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<double>( 0, 1 ), 
                                 nll::core::make_vector<double>( 1 ),
                                 Database::Sample::LEARNING ) );
      dat.add( Database::Sample( nll::core::make_vector<double>( 1, 0 ), 
                                 nll::core::make_vector<double>( 1 ),
                                 Database::Sample::LEARNING ) );
      Mlp mlp1( nll::core::make_vector<nll::ui32>( 2, 3, 1 ) );

      nll::algorithm::StopConditionMlpThreshold c( 0.1 );
      nll::core::Timer t;
      std::cout << "l error=" <<mlp1.learn( dat, c, 10, 0.1, 0.00 ).learningError << std::endl;;
      std::cout << "time=" << t.getCurrentTime() << std::endl;

      nll::core::Buffer1D<double> val = mlp1.propagate( nll::core::make_vector<double>( 1, 1 ) );
      val.print( std::cout );
      mlp1.write( "test.dat" );

      Mlp mlp2( "test.dat" );
      mlp2.propagate( nll::core::make_buffer1D<double>( 1, 1 ) ).print( std::cout );
   }
};

//#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMlp);
TESTER_TEST(testMlp1);
TESTER_TEST_SUITE_END();
//#endif