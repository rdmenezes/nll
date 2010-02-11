#include <tester/register.h>
#include <mvvPlatform/types.h>
#include <mvvScript/parser-context.h>

using namespace mvv;
using namespace mvv::platform;
using namespace mvv::parser;

struct TestBasic
{
   void test1()
   {
      ParserContext context;
      Ast* exp = context.parseFile( "c:/tmp/test1.txt" );
      //Ast* exp = context.parseString( "1212 sttr " );
      std::cout << " --------ast=" << exp << std::endl;

      std::cout << "msg=" << context.getError();
   }

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
};

TESTER_TEST_SUITE(TestBasic);
TESTER_TEST(testDummy1);
TESTER_TEST_SUITE_END();