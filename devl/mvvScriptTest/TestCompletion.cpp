#include <tester/register.h>

#include <mvvScript/completion.h>

using namespace mvv;
using namespace mvv::platform;
using namespace mvv::parser;

#define TEST_PATH    "../../mvvScriptTest/test/"


namespace mvv
{
namespace parser
{

   
}
}

struct TestCompletion
{
   void eval1()
   {
      std::string hided;
      TESTER_ASSERT( Completion::skipMatched( "a[b", hided ) == 2 );
      TESTER_ASSERT( Completion::skipMatched( "a(b", hided ) == 2 );
      TESTER_ASSERT( Completion::skipMatched( "a[SDF]b", hided ) == 0 );
      TESTER_ASSERT( Completion::skipMatched( "a(SDF)b", hided ) == 0 );
      TESTER_ASSERT( Completion::skipMatched( "a[b[x].", hided ) == 2 );
      TESTER_ASSERT( Completion::skipMatched( "a(b(x).", hided ) == 2 );
   }

   void eval2()
   {
      TESTER_ASSERT( Completion::findInterruptible( "a+b=c" ) == 4 );
      TESTER_ASSERT( Completion::findInterruptible( "a+b+c" ) == 4 );
      TESTER_ASSERT( Completion::findInterruptible( "a<<b>>c" ) == 6 );
   }

   void eval3()
   {
      TESTER_ASSERT( Completion::findExpressionType( "a::b" ) == Completion::DDCOLON );
      TESTER_ASSERT( Completion::findExpressionType( "a.a::b" ) == Completion::DDCOLON );
      TESTER_ASSERT( Completion::findExpressionType( "a::b.b" ) == Completion::DOT );
      TESTER_ASSERT( Completion::findExpressionType( " bb" ) == Completion::NORMAL );
   }

   void eval4()
   {

      {
         ui32 cut;
         Completion::ExpressionType type;
         std::string prefix;
         std::string match;

         Completion::analyse( "a[ a[3].test", type, prefix, match, cut );
         TESTER_ASSERT( prefix == " a[3]" );
         TESTER_ASSERT( match  == "test" );
         TESTER_ASSERT( type  == Completion::DOT );
         TESTER_ASSERT( cut  == 8 );
      }

      {
         ui32 cut;
         Completion::ExpressionType type;
         std::string prefix;
         std::string match;

         Completion::analyse( "a[ a[3].", type, prefix, match, cut );
         TESTER_ASSERT( prefix == " a[3]" );
         TESTER_ASSERT( match  == "" );
         TESTER_ASSERT( type  == Completion::DOT );
         TESTER_ASSERT( cut  == 8 );
      }

      {
         ui32 cut;
         Completion::ExpressionType type;
         std::string prefix;
         std::string match;

         Completion::analyse( "a[ a(3)::b", type, prefix, match, cut );
         TESTER_ASSERT( prefix == " a(3)" );
         TESTER_ASSERT( match  == "b" );
         TESTER_ASSERT( type  == Completion::DDCOLON );
         TESTER_ASSERT( cut  == 9 );
      }

      {
         ui32 cut;
         Completion::ExpressionType type;
         std::string prefix;
         std::string match;

         Completion::analyse( "{ a::b + abc", type, prefix, match, cut );
         TESTER_ASSERT( prefix == "" );
         TESTER_ASSERT( match  == "abc" );
         TESTER_ASSERT( type  == Completion::NORMAL );
         TESTER_ASSERT( cut  == 8 );
      }

      {
         ui32 cut;
         Completion::ExpressionType type;
         std::string prefix;
         std::string match;

         Completion::analyse( "t[ 2 * 3].n13", type, prefix, match, cut );
         TESTER_ASSERT( prefix == "t[ 2 * 3]" );
         TESTER_ASSERT( match  == "n13" );
         TESTER_ASSERT( type  == Completion::DOT );
         TESTER_ASSERT( cut  == 10 );
      }

      {
         ui32 cut;
         Completion::ExpressionType type;
         std::string prefix;
         std::string match;

         Completion::analyse( "[ t[ 2 * 3].n13", type, prefix, match, cut );
         TESTER_ASSERT( prefix == " t[ 2 * 3]" );
         TESTER_ASSERT( match  == "n13" );
         TESTER_ASSERT( type  == Completion::DOT );
         TESTER_ASSERT( cut  == 12 );
      }
   }

   void eval5()
   {
      std::vector<std::string> v;

      {
         ui32 cut;
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "int n123; int ba; int n1234; int bab(){return 0;} int n12345(){return 0;} class Test{} class n123456{} typedef int n1234567; typedef int HAHA;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         Completion completion( fe );
         std::set<mvv::Symbol> match = completion.findMatch( "n12", cut, v );
         TESTER_ASSERT( match.size() == 5 );
         TESTER_ASSERT( cut  == 0 );
         TESTER_ASSERT( match.find( mvv::Symbol::create( "n123" ) ) != match.end() );
         TESTER_ASSERT( match.find( mvv::Symbol::create( "n1234" ) ) != match.end() );
         TESTER_ASSERT( match.find( mvv::Symbol::create( "n12345" ) ) != match.end() );
         TESTER_ASSERT( match.find( mvv::Symbol::create( "n123456" ) ) != match.end() );
         TESTER_ASSERT( match.find( mvv::Symbol::create( "n1234567" ) ) != match.end() );
      }

      
      {
         ui32 cut;
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ class Test2{} typedef int Test3; typedef int TT1; class TT2{} }" );
         TESTER_ASSERT( result == Error::SUCCESS );

         Completion completion( fe );
         std::set<mvv::Symbol> match = completion.findMatch( "Test::Te", cut, v );
         TESTER_ASSERT( match.size() == 2 );
         TESTER_ASSERT( cut  == 6 );
         TESTER_ASSERT( match.find( mvv::Symbol::create( "Test2" ) ) != match.end() );
         TESTER_ASSERT( match.find( mvv::Symbol::create( "Test3" ) ) != match.end() );
      }

      {
         ui32 cut;
         CompilerFrontEnd fe;
         Error::ErrorType result = fe.run( "class Test{ Test(){} int bb; void aa(){} int n132; int n1342; void n132run(){} class n132Test{} typedef int n123Typedef;} Test t[ 5 ]; int n = 0;" );
         TESTER_ASSERT( result == Error::SUCCESS );

         Completion completion( fe );
         std::set<mvv::Symbol> match = completion.findMatch( "t[ 2 ].n13", cut, v );
         TESTER_ASSERT( match.size() == 3 );
         TESTER_ASSERT( cut  == 7 );
         TESTER_ASSERT( match.find( mvv::Symbol::create( "n132" ) ) != match.end() );
         TESTER_ASSERT( match.find( mvv::Symbol::create( "n1342" ) ) != match.end() );
         TESTER_ASSERT( match.find( mvv::Symbol::create( "n132run" ) ) != match.end() );
      }
   }
};


TESTER_TEST_SUITE(TestCompletion);

TESTER_TEST(eval1);
TESTER_TEST(eval2);
TESTER_TEST(eval3);
TESTER_TEST(eval4);
TESTER_TEST(eval5);
TESTER_TEST_SUITE_END();