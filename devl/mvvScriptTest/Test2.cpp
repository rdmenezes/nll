#include <tester/register.h>

#include <mvvScript/compiler.h>

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
         Error::ErrorType result = fe.run( "class Test2{ int ttt; Test2 tt; Test2(){} } Test2 t; t.ttt = 42; int nn; nn = t.ttt;" );
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
         Error::ErrorType result = fe.run( "class Test2{ Test2 tt; int ttt; Test2(){ttt=42;} } Test2 t; t.tt = t; t.tt.ttt = 43; int nn = t.tt.ttt;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "nn" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 43 );
      }
      
      

      {
         try
         {
            CompilerFrontEnd fe;
            fe.run( "class Test2{ Test2 tt; Test2(){}} class Test{int n; Test2 tt; Test ttt; Test(){}} Test t; Test2 ttt; t.tt.tt = ttt; t.n = 5; " );
            TESTER_ASSERT( 0 );
         } catch ( RuntimeException e )
         {
            // good!
         }
      }
      

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{class Test2{}}" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const Type* rt = fe.getClass( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Test" ), mvv::Symbol::create( "Test2" ) ) );
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
         const AstDeclFun* fn5 = fe.getFunction( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Fun1" ) ), nll::core::make_vector<const Type*>( fe.getClass( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Test" ) ) ) )  );
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
            
            Error::ErrorType result = fe.run( "import \"core\" class Test{ int a[]; Test(){ a[ 1 ] = 0; } } Test t;" );
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
/*
      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int& n; Test( int& nn ){ n = nn; } ~Test(){ n = 1; } } int res = 0; {Test test( res );}" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "res" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 1 );
      }
*/
   }


};

TESTER_TEST_SUITE(TestEval);
TESTER_TEST(eval1);
TESTER_TEST_SUITE_END();