#include <tester/register.h>
#include <mvvPlatform/types.h>
#include <mvvScript/parser-context.h>
#include <mvvScript/visitor-print.h>
#include <mvvScript/visitor-default.h>
#include <mvvScript/visitor-bind.h>

using namespace mvv;
using namespace mvv::platform;
using namespace mvv::parser;

#define TEST_PATH    "../../mvvScriptTest/test/"

struct TestBasic
{
   /**
    @brief basic input to test the lexer && parser only
    */
   void testDummy1()
   {
      ParserContext context;
      Ast* exp = 0;
      
      exp = context.parseString( " \" 123 asd  " );
      context.getError().clear();

      exp = context.parseString( " \" 123 asd  " );
      context.getError().clear();

      std::cout << "-----------------------------" << std::endl;
      exp = context.parseString( "987 123 asd  \"" );
      context.getError().clear();

      std::cout << " --------ast=" << exp << std::endl;

      std::cout << "msg=" << context.getError();
   }

   void testDummy2()
   {
       ParserContext context;
       Ast* exp = 0;
      
      exp = context.parseString( "if (1) { 2.5; 5; } else { 3; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "4;5;6;" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "if (1 && 1 + 1) { 2.5; } else { 3; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "if (1 * atest1.val[a[8]+3] ) { 2.5; 5; } else { 3; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "class test { var a; int n; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "call(4, 3); call(); call(4);" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int call( int n ){5;5;}" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "class Test{ int haha( test a){5;} }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int test = 0; int main(){ dsf haha; int test; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "class Test{ int aa; int haha( test a){5;} }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "class Test{ int aa; int haha( test a){5;} float test2; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int main(){test.print(\"asf\"); return 0;}" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "{var list; list = {5, 4, 3};}" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "var list = {3, 4};" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "{int a = 3 + test;}" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int[] list = {3, 4};" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int n = 1 * 2; int main( float a, int test = 2 * 3 ){return 0;} class A{ void init( int n = 0 ){} int n = 0; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "a[1].a[5] = 2*3+1;" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "int[] n = 2 * 3;" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "import \"test1.v\" include \"test2.v\" class Test{TTest test; int getVal( int a = 2 * 5, float b ); int getVal(){ int a; int b; string[] strings; strings[ 0 ] = \"test2\"; return a + b;} string str = \"test\"; }" );
      TESTER_ASSERT( exp );
      exp = context.parseString( "getVal(5);" );
      TESTER_ASSERT( exp );


      std::cout << "msg=" << context.getError();
   }

   void testFull1()
   {
      ParserContext context;
      Ast* exp = 0;
      
      exp = context.parseFile( TEST_PATH "test1.txt" );
      std::cout << "msg=" << context.getError();
      TESTER_ASSERT( exp );

      VisitorPrint p( std::cout );
      p( *exp );
   }
};

TESTER_TEST_SUITE(TestBasic);
//TESTER_TEST(testDummy2);
//TESTER_TEST(testDummy1);
TESTER_TEST(testFull1);
TESTER_TEST_SUITE_END();