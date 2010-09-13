#include <tester/register.h>

#include <mvvScript/compiler.h>

using namespace mvv;
using namespace mvv::platform;
using namespace mvv::parser;

#define TEST_PATH    "../../mvvScriptTest/test/"


namespace mvv
{
namespace parser
{
   class Completion
   {
   public:
      enum ExpressionType
      {
         NORMAL,     // find within all global variable/typedef/func/classes
         DOT,        // find within all the class declaration a member
         DDCOLON     // find a class or typedef
      };

   public:
      Completion( CompilerFrontEnd& cmp ) : _cmp( cmp )
      {
      }

      /**
       @brief Preprocess the string:
        process:
         - find the first unmatched [ or (, before, everything is skipped
         - find the last interrupt symbol: = + - * / > < >= <= | || & && [ ] ( ) " ; ex: a + b.test... => only consider "b.test..."
         - now determine the type of completion:
             - if there is a "::"  => we want to find class/typedef => cut until "::", find the type, find the type matching the string after "::"
             - if "." => we want to find a member variable => find the type of the expression bewfore ".", match the declaration with the part after "."
             - else we want to find class/Typedef/val/func => match the declaration of the expression

        Completion on:
         - var name
         - function name
         - typedef
         - class name
       */
      std::string preprocess( const std::string& s )
      {
         int start = skipMatched( s );

      }

      /**
       @brief Find the position of the last unmatched bracket or parenthesis
       @brief return the first character to be considered
       */
      static unsigned int skipMatched( const std::string& s )
      {
         struct Match
         {
            Match( int p ) : pos( p )
            {}

            int pos;
         };
         typedef std::vector<Match> Matches;

         Matches  parenthesis;
         Matches  brackets;
         Matches  braces;

         for ( ui32 n = 0; n < s.size(); ++n )
         {
            if ( s[ n ] == '[' )
               brackets.push_back( Match( n ) );
            else  if ( s[ n ] == '(' )
               parenthesis.push_back( Match( n ) );
            else  if ( s[ n ] == '{' )
               braces.push_back( Match( n ) );
            else if ( s[ n ] == ')' )
               parenthesis.pop_back();
            else if ( s[ n ] == ']' )
               brackets.pop_back();
            else if ( s[ n ] == '}' )
               braces.pop_back();
         }

         int skip = 0;
         if ( brackets.size() )
            skip = std::max( skip, brackets.rbegin()->pos ) + 1;
         if ( parenthesis.size() )
            skip = std::max( skip, parenthesis.rbegin()->pos ) + 1;
         return skip;
      }

      static unsigned int findInterruptible( const std::string& s )
      {
         static const char* interruptibles[] =
         {
            "=", "+", "-", "*", "/",  ">", "<", ">=", "<=", "||", "&&", "!=", ",", ";", "<<", ">>", "==", "&"
         };

         static const ui32 size = nll::core::getStaticBufferSize( interruptibles );
         std::vector<size_t> interruptiblesPos = _findLastOf( s, interruptibles, size );

         ui32 posMax = 0;
         for ( ui32 n = 0; n < size; ++n )
            if ( interruptiblesPos[ n ] != std::string::npos )
               posMax = std::max( (ui32)interruptiblesPos[ n ], posMax );
         return posMax;
      }

      static ExpressionType findExpressionType( const std::string& s )
      {
         static const char* interruptibles[] =
         {
            "::", "."
         };

         static const ui32 size = nll::core::getStaticBufferSize( interruptibles );
         std::vector<size_t> interruptiblesPos = _findLastOf( s, interruptibles, size );

         ui32 index = 2;
         ui32 posMax = 0;
         for ( ui32 n = 0; n < size; ++n )
            if ( interruptiblesPos[ n ] != std::string::npos )
            {
               if ( (ui32)interruptiblesPos[ n ] > posMax )
               {
                  posMax = (ui32)interruptiblesPos[ n ];
                  index = n;
               }
            }
         switch ( index )
         {
         case 0:
            return DDCOLON;
         case 1:
            return DOT;
         case 2:
            return NORMAL;
         default:
            throw RuntimeException( "Unhandled case!" );
         }
      }

      static void analyse( const std::string& s, ExpressionType& type, std::string& prefix, std::string& needToMatch )
      {
         int skip = skipMatched( s );
         std::string sub1;
         if ( skip < s.size() )
         {
            sub1 = std::string( &s[ skip ] );
         } else { type = NORMAL; prefix = ""; needToMatch = ""; return; }

         int interruptible = findInterruptible( sub1 );
         std::string sub2;
         if ( interruptible < sub1.size() )
         {
            sub2 = std::string( &sub1[ interruptible ] );
         } else { type = NORMAL; prefix = ""; needToMatch = ""; return; }

         type = findExpressionType( sub2 );
         if ( type == DOT )
         {
            size_t dotpos = _findLastOf( sub2, "." );
            ensure( dotpos != std::string::npos, "error!" );
            prefix = sub2.substr( 0, dotpos - 1 );
            needToMatch = sub2.substr( dotpos, sub2.size() );
         } else if ( type == DDCOLON )
         {
            size_t dotpos = _findLastOf( sub2, "::" );
            ensure( dotpos != std::string::npos, "error!" );
            prefix = sub2.substr( 0, dotpos - 2 );
            needToMatch = sub2.substr( dotpos, sub2.size() );
         } else if ( type == NORMAL )
         {
            prefix = "";
            needToMatch = sub2;
         } else {ensure( 0, "not handled case!" ); }
      }

   private:
      static std::vector<size_t> _findLastOf( const std::string& s, const char** interruptibles, ui32 size )
      {
         std::vector<size_t> interruptiblesPos( size );
         for ( ui32 n = 0; n < size; ++n )
         {
            size_t init = 0;
            size_t lastFound = std::string::npos;
            while ( 1 )
            {
               size_t posFound = s.find( interruptibles[ n ], init );
               if ( posFound != std::string::npos )
               {
                  lastFound = posFound + strlen( interruptibles[ n ] );
                  init = lastFound;
               } else break;
            }

            interruptiblesPos[ n ] = lastFound;
         }

         return interruptiblesPos;
      }

      static size_t _findLastOf( const std::string& s, const char* interruptibles )
      {
         size_t init = 0;
         size_t lastFound = std::string::npos;
         while ( 1 )
         {
            size_t posFound = s.find( interruptibles, init );
            if ( posFound != std::string::npos )
            {
               lastFound = posFound + strlen( interruptibles );
               init = lastFound;
            } else break;
         }
         return lastFound;
      }

   private:
      // disabled copy
      Completion& operator=( const Completion& );
      Completion( const Completion& );

   private:
      CompilerFrontEnd& _cmp;
   };
}
}

struct TestCompletion
{
   void eval1()
   {
      TESTER_ASSERT( Completion::skipMatched( "a[b" ) == 2 );
      TESTER_ASSERT( Completion::skipMatched( "a(b" ) == 2 );
      TESTER_ASSERT( Completion::skipMatched( "a[SDF]b" ) == 0 );
      TESTER_ASSERT( Completion::skipMatched( "a(SDF)b" ) == 0 );
      TESTER_ASSERT( Completion::skipMatched( "a[b[x]." ) == 2 );
      TESTER_ASSERT( Completion::skipMatched( "a(b(x)." ) == 2 );
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
         Completion::ExpressionType type;
         std::string prefix;
         std::string match;

         Completion::analyse( "a[ a[3].test", type, prefix, match );
         TESTER_ASSERT( prefix == " a[3]" );
         TESTER_ASSERT( match  == "test" );
         TESTER_ASSERT( type  == Completion::DOT );
      }

      {
         Completion::ExpressionType type;
         std::string prefix;
         std::string match;

         Completion::analyse( "a[ a[3].", type, prefix, match );
         TESTER_ASSERT( prefix == " a[3]" );
         TESTER_ASSERT( match  == "" );
         TESTER_ASSERT( type  == Completion::DOT );
      }

      {
         Completion::ExpressionType type;
         std::string prefix;
         std::string match;

         Completion::analyse( "a[ a(3)::b", type, prefix, match );
         TESTER_ASSERT( prefix == " a(3)" );
         TESTER_ASSERT( match  == "b" );
         TESTER_ASSERT( type  == Completion::DDCOLON );
      }

      {
         Completion::ExpressionType type;
         std::string prefix;
         std::string match;

         Completion::analyse( "{ a::b + abc", type, prefix, match );
         TESTER_ASSERT( prefix == "" );
         TESTER_ASSERT( match  == " abc" );
         TESTER_ASSERT( type  == Completion::NORMAL );
      }
   }
};


TESTER_TEST_SUITE(TestCompletion);
//TESTER_TEST(eval1);
//TESTER_TEST(eval2);
//TESTER_TEST(eval3);
TESTER_TEST(eval4);
TESTER_TEST_SUITE_END();