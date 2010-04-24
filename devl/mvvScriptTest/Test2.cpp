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

         const RuntimeValue& rt2 = fe.getVariable( mvv::Symbol::create( "t" ) );
         std::cout << "t val="; VisitorEvaluate::_debug( *(RuntimeValue*)&rt2 );

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
            Error::ErrorType result = fe.run( "class Test2{ Test2 tt; Test2(){}} class Test{int n; Test2 tt; Test ttt; Test(){}} Test t; Test2 ttt; t.tt.tt = ttt; t.n = 5; " );
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



/*
      {
         CompilerFrontEnd fe;
         try
         {
            // we don't run it so no runtime error...
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
            // we don't run it so no runtime error...
            Error::ErrorType result = fe.run( "import \"core\" class Test{ int a[]; Test(){ a[ 1 ] = 0; } } Test t;" );
            TESTER_ASSERT( 0 );
         } catch ( RuntimeException e ){
            // good
         }
      }
*/
      

      {
         /*
         std::cout << "Size=" << sizeof(RuntimeValue) << std::endl;
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ int n; Test( int arg ){ n = arg; }} Test t1( 5 ); t1.n = 42; int n1 = t1.n;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n1" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::INT );
         TESTER_ASSERT( rt.intval == 42 );
*/
      }
/*

      {
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int n[] = { 4, 5, 6, 7, 8}; int nn = n[ 3 ];" );
         TESTER_ASSERT( result == Error::SUCCESS );

         const RuntimeValue& rt = fe.getVariable( mvv::Symbol::create( "n" ) );
         TESTER_ASSERT( rt.type == RuntimeValue::ARRAY );

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

      
*/
/*
      {
         CompilerFrontEnd fe;
         // we don't run it so no runtime error...
         Error::ErrorType result = fe.run( "import \"core\" class Test{ int a[]; Test(){ int array[ 5 ]; array[ 0 ] = 5;} } Test t;" );
         TESTER_ASSERT( result == Error::SUCCESS );
      }
      */
/*
      {
         CompilerFrontEnd fe;
         // we don't run it so no runtime error...
         Error::ErrorType result = fe.run( "import \"core\" class Test{ int a[]; Test(){ int array[ 5 ]; a = array; a[ 0 ] = 5; a[ 1 ] = 4; a[ 2 ] = 3; a[ 3 ] = 2; a[ 4 ] = 1; } } Test t;" );
         TESTER_ASSERT( result == Error::SUCCESS );
      }
*/

      /*
      // TODO activate
      try
      {
         EVAL( "int n[] = { 4, 5, 6, 7, 8}; int nn = n[ -13 ];" );
         EVAL_END;
         TESTER_ASSERT( 0 );
      } catch ( RuntimeException e )
      {
      }
      */
   }


};

TESTER_TEST_SUITE(TestEval);
TESTER_TEST(eval1);
TESTER_TEST_SUITE_END();