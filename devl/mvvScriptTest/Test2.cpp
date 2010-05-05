#include <tester/register.h>

#include <mvvScript/compiler.h>
#include <mvvPlatform/context-volumes.h>
#include <mvvPlatform/context-tools.h>
#include <mvvPlatform/engine-handler-impl.h>
#include <mvvPlatform/order-dispatcher-impl.h>
#include <mvvPlatform/order-provider-impl.h>
#include <mvvPlatform/order-manager-thread-pool.h>
#include <boost/thread/thread.hpp>

using namespace mvv;
using namespace mvv::platform;
using namespace mvv::parser;

#define TEST_PATH    "../../mvvScriptTest/test/"

struct TestEval
{
   void eval1()
   {
      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "float n = 5.5;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::FLOAT );
         TESTER_ASSERT( rt.floatval == 5.5 );
      }
      
      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int nn = 8; int n = 5; n = nn; n = 7;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 7 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 8 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int nn = 8; int& n = nn; nn = 5;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 5 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 5 );
      }


      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int fn( int n ){ return n; } int nn = fn( 42 );" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int& fn( int& n ){ return n; } int n = 5; int nn = fn( n );" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 5 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int& fn( int& n ){ return n; } int n = 5; int nn = fn( n ); nn = 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 3 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 5 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int& fn( int& n ){ return n; } int n = 5; int& nn = fn( n ); nn = 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 3 );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 3 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "string n = \"123456\";" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::STRING );
         TESTER_ASSERT( rt.stringval == "123456" );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int test( int n ){return n;} int n = test(5);" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 5 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int test( int n, int nn = 11 ){return nn;} int n = test(5);" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 11 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int n = 1;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      

      {
         CompilerFrontEnd fe;
         try
         {
            TESTER_ASSERT( fe.run( "import \"HAHAH\"" ) != Error::SUCCESS );
         } catch( RuntimeException e )
         {
            TESTER_ASSERT(0);
         }
      }


      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int nn1 = 8 + 2;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 10 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int nn1 = 8; int nn2 = 2; int nn; nn = 2 + nn1 + nn2;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 12 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int nn1 = 8; int nn2 = nn1; int n; n = nn2;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 8 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( " int test = 5;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "test" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 5 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int& fn( int& n ){ return n; } int n = 5; int nn = fn( n ); nn = 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 5 );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 3 );
      }
      
      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test2{ int ttt; Test2 tt = NULL; Test2(){} } Test2 t; t.ttt = 42; int nn; nn = t.ttt;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test2{ int ttt; Test2(){ttt=42;} } Test2 t; int nn = t.ttt;" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );
      }


      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test2{ Test2 tt = NULL; int ttt; Test2(){ttt=42;} } Test2 t; t.tt = t; t.tt.ttt = 43; int nn = t.tt.ttt;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 43 );
      }
      
      
      /*
      // TODO should throw
      {
         try
         {
            CompilerFrontEnd fe;
            fe.run( "class Test2{ Test2 tt; Test2(){}} class Test{int n; Test2 tt = NULL; Test ttt = NULL; Test(){}} Test t; Test2 ttt; t.tt.tt = ttt; t.n = 5; " );
            TESTER_ASSERT( 0 );
         } catch ( RuntimeException e )
         {
            // good!
         }
      }*/
      

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{class Test2{}}" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const Type* rt = fe.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Test" ), mvv::Symbol::create( "Test2" ) ) );
         TESTER_ASSERT( dynamic_cast<const TypeNamed*>( rt ) );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int Fun1( string s ){return 5;} class Test{int Fun1(int n, float nn){return 0;} } int Fun1( Test t ){ return 1;}" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const AstDeclFun* fn0 = fe.getFunction( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Fun1" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) )  );
         const AstDeclFun* fn1 = fe.getFunction( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Fun1" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) )  );
         const AstDeclFun* fn2 = fe.getFunction( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Test" ), mvv::Symbol::create( "Fun1" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) )  );
         const AstDeclFun* fn3 = fe.getFunction( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Test" ), mvv::Symbol::create( "Fun2" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) )  );
         const AstDeclFun* fn4 = fe.getFunction( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Test" ), mvv::Symbol::create( "Fun2" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) )  );
         const AstDeclFun* fn5 = fe.getFunction( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Fun1" ) ), nll::core::make_vector<const Type*>( fe.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Test" ) ) ) )  );
         TESTER_ASSERT( fn0 );
         TESTER_ASSERT( !fn1 );
         TESTER_ASSERT( fn2 );
         TESTER_ASSERT( !fn3 );
         TESTER_ASSERT( !fn4 );
         TESTER_ASSERT( fn5 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int nn = 1 == 2;" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 0 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int nn = 1 == 1;" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int nn1 = 1; int nn2 = 1; int nn = nn1 == nn2;" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int n; Test( int nn ){ n = nn; } } Test n1( 5 );" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::TYPE );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test{int n; Test(){n = 4;}} int plus( Test t1, Test t2 ){ return t1.n + t2.n; } Test t1; Test t2; int nn = plus( t1, t2 );" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 8 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test{int n; Test(){n = 4;}} int operator+( Test t1, Test t2 ){ return t1.n + t2.n; } Test t1; Test t2; int nn = t1 + t2;" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 8 );
      }
      

      

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test{ int n; Test( int nn ){ n = nn; } int operator+( Test t ){ return n + t.n; } } Test n1( 5 ); Test n2( 3 ); int nn = n1 + n2;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 8 );
      }
      

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test2{ int nn; int nnn; int n; Test2( int nx ){ n = nx; nn = 123; nnn = 0; }} Test2 n2( 3 ); int nn1 = n2.nn;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "nn1" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 123 );
      }


      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test2{ int nn; int nnn; int n; Test2( int nx ){ n = nx; nn = 123; nnn = 0; }} class Test{ int n; Test( int nn ){ n = nn; } int operator+( Test2 t ){ return n + t.n; } } Test n1( 5 ); Test2 n2( 3 ); int nn = n1 + n2; int nn1 = n2.nn;" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 8 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "nn1" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 123 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int n1 = 1; { { int n2 = 2; int n3 = 3;} int n4 = 4; n1 = n4; }" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 4 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int test( int n ){ int nn = 4; int nn2 = 1; nn = nn + nn2; return nn + n; } int nn = test( 5 );" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 10 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int test( int n ){ { int nn = 4; int nn2 = 1; nn = nn + nn2; return nn + n; } } int nn = test( 5 );" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 10 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int n; Test( int arg ){ n = arg; }} Test t1( 5 ); Test t2 = t1; t1.n = 42; int n1 = t1.n; int n2 = t2.n;" );
         TESTER_ASSERT( result == Error::SUCCESS );


         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n2" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 42 );
      }
      

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int n; Test( int arg ){ n = arg; }} Test t1( 5 ); t1.n = 42; int n1 = t1.n;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );
      }


      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{int a; Test(int n){ a = n; }} Test t = Test(42); int n1 = t.a;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int a = 42;" );
         TESTER_ASSERT( result == Error::SUCCESS );
         result = fe.run( "int b = 43;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "a" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "b" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 43 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{int a; Test(int n){ a = n; }} void create(Test& tt){ tt = Test(43);} Test t = Test(42); create(t); int n1 = t.a;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 43 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{int a; Test(int n){ a = n; }} void create(Test& tt){ tt = Test(43);} Test t = Test(42); Test t2 = t; create(t); int n1 = t.a; int n2 = t2.a;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 43 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n2" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 42 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{class Test2{int n; Test2(int nn){ n = 44;}}} Test::Test2 t2 = typename Test::Test2(); int n1 = t2.n;" );
         TESTER_ASSERT( result == Error::TYPE );
      }


      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int n; Test( int nn ){ n = nn; }} Test t( 5 ); Test tt = t; t.n = 44; int n1 = tt.n;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 44 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int n = 5; int n1; if ( n == 5 ) { n1 = 42;} else { n1 = 43; }" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int n = 6; int n1; if ( n == 5 ) { n1 = 42;} else { n1 = 43; }" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 43 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int n = 6; int n1 = 4 + 2 == 6;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         nll::core::Timer tt;
         Error::ErrorType result = fe.run( "import \"core\" int n = 0; while ( 1 ){ if ( n == 150000){break;} n = n + 1;}" );
         std::cout << "Time 150000 loops=" << tt.getCurrentTime() << std::endl;
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 150000 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" print(\"test123456\"); println(\"789\"); " );
         TESTER_ASSERT( result == Error::SUCCESS );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" string s = \"123\" + \"456\";" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "s" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::STRING );
         TESTER_ASSERT( rt.stringval == "123456" );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int n = 1 * 2 + 4 / 2;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 4 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test{ int n; Test(){ n = 42; } void inc(){ this.n = this.n + 4;}} Test test; int n1 = test.n; test.inc(); int n2 = test.n;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n2" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 46 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int n[] = {1, 2, 3, 4, 5}; int n1 = n[ 0 ]; int n2 = n[ 4 ];" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n2" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 5 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int nn[]; int n[] = {1, 2, 3, 4, 5}; nn = n; int n1 = nn[ 0 ]; int n2 = nn[ 4 ];" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n2" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 5 );

      }

      {
         CompilerFrontEnd fe;
         try
         {
            
            Error::ErrorType result = fe.run( "import \"core\" class Test{ int a[]; Test(){ a[ 1 ] = 0; } }" );
            TESTER_ASSERT( result == Error::SUCCESS );
         } catch ( RuntimeException e ){
            TESTER_ASSERT( 0 );
         }
      }

      {
         CompilerFrontEnd fe;
         try
         {
            
            fe.run( "import \"core\" class Test{ int a[]; Test(){ a[ 1 ] = 0; } } Test t;" );
            TESTER_ASSERT( 0 );
         } catch ( RuntimeException e ){
            // good
         }
      }

      

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int n; Test( int arg ){ n = arg; }} Test t1( 5 ); t1.n = 42; int n1 = t1.n;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );

      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int n[] = { 4, 5, 6, 7, 8}; int nn = n[ 3 ];" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::TYPE );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 7 );
      }

      {
         CompilerFrontEnd fe;
         try
         {
            fe.run( "int n[] = { 4, 5, 6, 7, 8}; int nn = n[ 13 ];" );
            TESTER_ASSERT( 0 );
         } catch( RuntimeException e )
         {
         }
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int n[] = { 4, 5, 6, 7, 8}; int nn = n[ 3 ];" );
         TESTER_ASSERT( result == Error::SUCCESS );       

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 7 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{int n; Test(){ n = 42; }} Test n[5]; int nn = n[ 1 ].n;" );
         TESTER_ASSERT( result == Error::SUCCESS );       

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 42 );
      }
      
      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int n[5][4]; n[0][0] = 1; n[4][3] = 2; int n1 = n[ 0 ][ 0 ]; int n2 = n[ 4 ][ 3 ];" );
         TESTER_ASSERT( result == Error::SUCCESS );       

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n2" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 2 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" class Test{ int a[]; Test(){ int array[ 5 ]; array[ 0 ] = 5; a = array;} } Test t; int n = t.a[ 0 ];" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 5 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" float f = 3.5 + 2;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::FLOAT );
         TESTER_ASSERT( fabs( rt.floatval - 5.5 ) < 1e-6 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" float f = ((3 + 2.5));" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::FLOAT );
         TESTER_ASSERT( fabs( rt.floatval - 5.5 ) < 1e-6 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" float f = ((3.5 + 2.5));" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::FLOAT );
         TESTER_ASSERT( fabs( rt.floatval - 6 ) < 1e-6 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3 == 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3.000 == 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3 == 3.000;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3.000 == 3.000;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3 != 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 0 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3.000 != 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 0 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3 != 3.000;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 0 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3.000 != 3.000;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 0 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3 <= 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3.000 <= 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3 <= 3.000;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3.000 <= 3.000;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         
         Error::ErrorType result = fe.run( "import \"core\" int f = 3 >= 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int f = 3.000 >= 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int f = 3 >= 3.000;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int f = 3.000 >= 3.000;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int f = 3 && 2;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" int f = 0 || -1;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "f" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int Test(){}}" );
         TESTER_ASSERT( result == Error::BIND );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ import ~Test(); }" );
         TESTER_ASSERT( result == Error::SUCCESS );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ ~Test(){} }" );
         TESTER_ASSERT( result == Error::SUCCESS );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ ~Test(){} ~Test(){} }" );
         TESTER_ASSERT( result == Error::TYPE );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ ~Test( int n ){} }" );
         TESTER_ASSERT( result == Error::PARSE );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ import int ~Test(); }" );
         TESTER_ASSERT( result == Error::PARSE );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int& n; Test( int& nn ){ n = nn; } void inc(){ n = 1;}} int n8 = 0; Test test( n8 ); test.inc();" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n8" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int& n; Test( int& nn ){ n = nn; } void inc(){ n = 1;}} int n8 = 0; Test test( n8 ); n8 = 5; int n9 = test.n;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n9" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 5 );
      }

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int& n; Test(){ } } Test test; int n9 = 3; test.n = n9; n9 = 4; int n8 = test.n;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n8" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 4 );
      }

      {
         // check destructor call!
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test{ int n1; int& n; int n2; Test( int& nn ){ n = nn; } ~Test(){ n = n + 1; } } int res = 0; {Test test( res );}" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "res" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         // check destructor call!
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test{Test(){} ~Test(){print(\"Destroyed\");}} Test t; int n = 3;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         fe.clear();
      }

      std::stringstream save;
      {
         // interactive test simulation
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test{ int n1; int& n; int n2; Test( int& nn ){ n = nn; } ~Test(){ n = n + 1; println(\"Destroyed1\"); } } int res = 0;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "res" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 0 );

         result = fe.run( "res = 5;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "res" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 5 );

         result = fe.run( "{ Test t( res ); }" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt3 = fe.getVariable( mvv::Symbol::create( "res" ) );
         TESTER_ASSERT( rt3.type == RuntimeValue::INT );
         TESTER_ASSERT( rt3.intval == 6 );

         result = fe.run( "int res = 18;" );
         TESTER_ASSERT( result == Error::BIND );

         const RuntimeValue& rt4 = fe.getVariable( mvv::Symbol::create( "res" ) );
         TESTER_ASSERT( rt4.type == RuntimeValue::INT );
         TESTER_ASSERT( rt4.intval == 6 );

         result = fe.run( "import \"core\"" );
         TESTER_ASSERT( result == Error::SUCCESS );

         result = fe.run( "include \"core\"" );
         TESTER_ASSERT( result == Error::SUCCESS );
         

         result = fe.run( "res = 2 * res;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt5 = fe.getVariable( mvv::Symbol::create( "res" ) );
         TESTER_ASSERT( rt5.type == RuntimeValue::INT );
         TESTER_ASSERT( rt5.intval == 12 );

         result = fe.run( "int size = 10; int array[ size ]; int tmp = 0; while ( tmp < size ){ array[ tmp ] = tmp; tmp = tmp + 1; }" );
         TESTER_ASSERT( result == Error::SUCCESS );

         result = fe.run( "int n0 = array[ 0 ]; int n1 = array[ 1 ]; int n9 = array[ 9 ];" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt6 = fe.getVariable( mvv::Symbol::create( "n0" ) );
         TESTER_ASSERT( rt6.type == RuntimeValue::INT );
         TESTER_ASSERT( rt6.intval == 0 );

         const RuntimeValue& rt7 = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt7.type == RuntimeValue::INT );
         TESTER_ASSERT( rt7.intval == 1 );

         const RuntimeValue& rt8 = fe.getVariable( mvv::Symbol::create( "n9" ) );
         TESTER_ASSERT( rt8.type == RuntimeValue::INT );
         TESTER_ASSERT( rt8.intval == 9 );

         fe.exportCode( save );
         fe.exportCode( std::cout );
      }

      {
         // interactive test simulation
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( save.str() );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt5 = fe.getVariable( mvv::Symbol::create( "res" ) );
         TESTER_ASSERT( rt5.type == RuntimeValue::INT );
         TESTER_ASSERT( rt5.intval == 12 );

         const RuntimeValue& rt6 = fe.getVariable( mvv::Symbol::create( "n0" ) );
         TESTER_ASSERT( rt6.type == RuntimeValue::INT );
         TESTER_ASSERT( rt6.intval == 0 );

         const RuntimeValue& rt7 = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt7.type == RuntimeValue::INT );
         TESTER_ASSERT( rt7.intval == 1 );

         const RuntimeValue& rt8 = fe.getVariable( mvv::Symbol::create( "n9" ) );
         TESTER_ASSERT( rt8.type == RuntimeValue::INT );
         TESTER_ASSERT( rt8.intval == 9 );
      }

      {
         // check destructor call!
         CompilerFrontEnd fe;
         try
         {
            fe.run( "import \"core\" class Test{Test(){} ~Test(){print(\"Destroyed\");}} Test t; int a[ 4 ]; a[ 5 ] = 0; int n = 3;" );
            TESTER_ASSERT( 0 );
         } catch ( RuntimeException e )
         {
            // true
         }
      }

      {
         // check code export
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int n2; Test(){ } ~Test(){ } } int res = 0; int res2 = 0; {Test test( res ); }" );
         TESTER_ASSERT( result == Error::TYPE );
      }

      {
         // check code export
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int a[] = {1, 2}; } " );
         TESTER_ASSERT( result == Error::PARSE );
      }


      {
         // check code export
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int n2 = 6; Test(){ } ~Test(){ } } int res = 0; int res2 = 0; {Test test; res2 = test.n2; }" );
         TESTER_ASSERT( result == Error::SUCCESS );
      }

      {
         // check code export
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Vector2i{ int vals; Vector2i( int x = 9 ){ vals = x; } } class Test{ Vector2i pos; Test( Vector2i a = Vector2i( 8 ) ){ pos = a; } } Test t; int n = t.pos.vals;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 8 );
      }

      {
         // double constructor
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Vector2i{ int vals; Vector2i( int x = 9 ){ vals = x; } } class Test{ Vector2i pos;  Test(){} } Test t; int n = t.pos.vals;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 9 );
      }

      {
         // double constructor
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Vector2i{ int vals = 9; Vector2i(){} } class Test{ Vector2i pos;  Test(){} } Test t; int n = t.pos.vals;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 9 );
      }

      {
         // double constructor
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Vector2i{ int vals[ 3 ]; Vector2i(){ vals[ 0 ] = 3; vals[ 1 ] = 4; vals[ 2 ] = 5; } } class Test{ Vector2i pos;  Test(){} } Test t; int n = t.pos.vals[ 0 ];" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 3 );
      }

      {
         // double constructor (constructor after automatic construction)
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Vector2i{ int vals = 9; Vector2i( int nn ){ vals = nn; } } Vector2i v = Vector2i( 11 ); int n = v.vals;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 11 );
      }

      {
         // double constructor (init by copy)
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Vector2i{ int vals = 9; Vector2i(){} } Vector2i v = Vector2i(); int n = v.vals;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 9 );
      }

      {
         // double constructor (init array)
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Vector2i{ int vals = 9; Vector2i(){} } Vector2i v[ 4 ]; int n = v[ 2 ].vals;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 9 );
      }

      {
         // double constructor (init array)
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int haha; class Vector2i{ int n = haha; Vector2i(){} } " );
         TESTER_ASSERT( result == Error::BIND );
      }

      {
         // double constructor (init array)
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{} Test t = NULL;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "t" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::NIL );
      }

      {
         // double constructor (init array)
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ Test t = NULL; Test(){} } Test t;" );
         TESTER_ASSERT( result == Error::SUCCESS );
      }

      
      {
         // double constructor (init array)
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ Test t; Test(){} } Test t;" );
         TESTER_ASSERT( result == Error::TYPE );
      }

      {
         // multiple destructor
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test{ int& n; Test(){} ~Test(){ n = n + 1; println(\"haha\");} } int n = 0; {Test t; t.n = n;}" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }

      {
         // multiple destructor
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int n; Test(){} ~Test(){ n = 1; } } {Test t[ 1 ]; t[ 0 ].n = 0;}" );
         TESTER_ASSERT( result == Error::SUCCESS );
      }
      
      {
         // multiple destructor
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test{ int& n; Test(){} ~Test(){ n = n + 1; println(\"=====\");} } int n = 0; {Test t[ 3 ]; t[ 0 ].n = n; t[ 1 ].n = n; t[ 2 ].n = n;}" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 3 );
      }

      {
         // multiple destructor
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" class Test{ int& n; Test(){} ~Test(){ n = n + 1; int nn = 0; println(\"=====\");} } int n = 0; {Test t[ 3 ]; int ba;  t[ 0 ].n = n; t[ 1 ].n = n; t[ 2 ].n = n; int ab[4];}" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 3 );
      }

      {
         // multiple destructor
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Vector3f{ float vals[ 3 ]; Vector3f(){} float& operator[]( int index ){ return vals[ index ]; } } Vector3f v; v[ 1 ] = 16; v[ 0 ] = 15; v[ 2 ] = 17; int n0 = v[ 0 ]; int n1 = v[ 1 ]; int n2 = v[ 2 ];" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt1 = fe.getVariable( mvv::Symbol::create( "n0" ) );
         TESTER_ASSERT( rt1.type == RuntimeValue::INT );
         TESTER_ASSERT( rt1.intval == 15 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::INT );
         TESTER_ASSERT( rt2.intval == 16 );

         const RuntimeValue& rt3 = fe.getVariable( mvv::Symbol::create( "n2" ) );
         TESTER_ASSERT( rt3.type == RuntimeValue::INT );
         TESTER_ASSERT( rt3.intval == 17 );
      }

      {
         // multiple destructor
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Vector3fT{ float vals[ 3 ]; Vector3fT(){}  float& operator()( int index ){ return vals[ index ]; } } Vector3fT v; v( 1 ) = 16.0;  v( 0 ) = 15.0; v( 2 ) = 17.0; int n0 = v( 0 ); int n1 = v( 1 ); int n2 = v( 2 );" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt1 = fe.getVariable( mvv::Symbol::create( "n0" ) );
         TESTER_ASSERT( rt1.type == RuntimeValue::FLOAT );
         TESTER_ASSERT( rt1.floatval == 15 );

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt2.type == RuntimeValue::FLOAT );
         TESTER_ASSERT( rt2.floatval == 16 );

         const RuntimeValue& rt3 = fe.getVariable( mvv::Symbol::create( "n2" ) );
         TESTER_ASSERT( rt3.type == RuntimeValue::FLOAT );
         TESTER_ASSERT( rt3.floatval == 17 );
      }
   }

   static void wait( float seconds )
   {
     clock_t endwait;
     endwait = (clock_t)(clock () + seconds * CLOCKS_PER_SEC);
     while (clock() < endwait) {}
   }

   class Launcher
   {
   public:
      Launcher( CompilerFrontEnd& fe, const std::string& cmd ) : _fe( fe ), _cmd( cmd )
      {}

      void operator()()
      {
         std::cout << "running scripting thread" << std::endl;
         Error::ErrorType result = _fe.run( _cmd );
         std::cout << "end scripting running" << std::endl;
         TESTER_ASSERT( result == Error::SUCCESS );
      }


   private:
      CompilerFrontEnd&    _fe;
      std::string          _cmd;
   };

   void eval2()
   {
      
      {
         //
         // test volume loading
         //

         // handler setup
         EngineHandlerImpl handler;
         OrderProviderImpl provider;
         OrderDispatcherImpl dispatcher;

         // context setup
         platform::Context context;
         context.add( new platform::ContextVolumes() );
         context.add( new platform::ContextTools( context.get<platform::ContextVolumes>()->volumes, handler, provider, dispatcher ) );

         CompilerFrontEnd fe;
         fe.setContextExtension( mvv::platform::RefcountedTyped<Context>( &context, false ) );

         Error::ErrorType result = fe.run( "import \"core\"  VolumeID vid1 = loadVolumeMF2( \"../../nllTest/data/medical/pet.mf2\"); Volume vol1 = getVolume( vid1 );" );
         TESTER_ASSERT( result == Error::SUCCESS );
         TESTER_ASSERT( context.get<platform::ContextVolumes>()->volumes.size() == 1 );   // check we have correctly loaded the volume
      }

      {
         //
         // test volume loading asynchronous
         //

         // set up a pool that will run asynchronous order
         OrderManagerThreadPool pool( 4 );
         EngineHandlerImpl handler;


         // context setup
         platform::Context context;
         context.add( new platform::ContextVolumes() );
         context.add( new platform::ContextTools( context.get<platform::ContextVolumes>()->volumes, handler, pool, pool ) );


         // create a front end and run it in another thread
         CompilerFrontEnd fe;
         fe.setContextExtension( mvv::platform::RefcountedTyped<Context>( &context, false ) );
         Launcher launcher( fe, "import \"core\"  VolumeID vid1 = loadVolumeAsynchronous( \"../../nllTest/data/medical/pet.mf2\"); Volume vol1 = getVolume( vid1 ); Vector3i size = vol1.getSize(); int x = size[ 0 ];" );
         boost::thread dispatchThread( boost::ref( launcher ) );

         // in the meantime, for this thread, wait a bit to receive the orders
         wait( 1 );

         // run the orders
         pool.run();

         wait( 7 );
         // dispatch the results
         pool.run();

         // final check
         wait( 2 );
         TESTER_ASSERT( context.get<platform::ContextVolumes>()->volumes.size() == 1 );   // check we have correctly loaded the volume

         const RuntimeValue& rt1 = fe.getVariable( mvv::Symbol::create( "x" ) );
         TESTER_ASSERT( rt1.type == RuntimeValue::INT );
         TESTER_ASSERT( rt1.intval == 128 );
      }
   }
      
/*
      
      {
         // check code export
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "import \"core\" Segment s;" );
         TESTER_ASSERT( result == Error::SUCCESS );
      }
*/
};

TESTER_TEST_SUITE(TestEval);
//TESTER_TEST(eval1);
TESTER_TEST(eval2);
TESTER_TEST_SUITE_END();