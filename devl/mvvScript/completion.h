#ifndef MVV_PARSER_COMPLETION_H_
# define MVV_PARSER_COMPLETION_H_

# include "mvvScript.h"
# include "compiler.h"
# include "completion-interface.h"
# include "visitor-print-simple.h"
# include <iterator>

namespace mvv
{
namespace parser
{
   class Completion : public CompletionInterface
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
       @brief Analyse the current string: determine the type of the last expression
       */
      virtual void getType( const std::string& s, ui32& cutpoint, std::vector<std::string>& oneMatchPrototype )
      {
         std::set<AstDecl*> match = _process( s, cutpoint );
         mvv::Symbol actualString = mvv::Symbol::create( &s[ cutpoint ] );

         std::stringstream ss;
         for ( std::set<AstDecl*>::iterator it = match.begin(); it != match.end(); ++it )
         {
            if ( (*it)->getName() == actualString )
            {
               VisitorPrintSimple visitor( ss );
               visitor( **it );
            }
         }

         std::string str;
         while ( 1 )
         {
            std::getline( ss, str );
            if ( ss.eof() )
               break;
            if ( str != "" )
               oneMatchPrototype.push_back( str );
         }
      }

      virtual std::set<AstDecl*> _process( const std::string& s, ui32& cutpoint )
      {
         std::set<AstDecl*> match;

         ExpressionType type;
         std::string prefix;
         std::string needToMatch;
         analyse( s, type, prefix, needToMatch, cutpoint );

         if ( type == NORMAL )
         {
            // check the variables
            std::set<AstDeclVar*> variables1 = _cmp.getVariables().findMatch( needToMatch );
            std::copy( variables1.begin(), variables1.end(), std::inserter( match, match.begin() ) );

            // check the functions
            std::set<AstDeclFun*> variables2 = parser::findMatch( _cmp.getFunctions(), needToMatch );
            std::copy( variables2.begin(), variables2.end(), std::inserter( match, match.begin() ) );

            // check classes
            std::set<AstDeclClass*> variables3 = _cmp.getClasses().findMatch( needToMatch );
            std::copy( variables3.begin(), variables3.end(), std::inserter( match, match.begin() ) );

            // check typedefs
            std::set<AstTypedef*> variables4 = _cmp.getTypedefs().findMatch( needToMatch );
            std::copy( variables4.begin(), variables4.end(), std::inserter( match, match.begin() ) );
         } else if ( type == DOT )
         {
            bool sandbox = _cmp.getSandbox();
            _cmp.setSandbox( true );

            Error::ErrorType result = _cmp.run( prefix + ";" );
            if ( result == Error::SUCCESS )
            {
               // here we try to go back in the tree to find a typed expression. Once we have it, it must be a named type
               // we then just compare the members...
               const Ast* exp = _cmp.getLastParsedExpression();
               const AstStatements* stmts = dynamic_cast<const AstStatements*>( exp );
               if ( stmts && stmts->getStatements().size() == 1 )
               {
                  const AstExp* e = dynamic_cast<const AstExp*>( *stmts->getStatements().rbegin() );
                  if ( e )
                  {
                     const TypeNamed* t = dynamic_cast<const TypeNamed*>( e->getNodeType() );
                     if ( t )
                     {
                        std::set<AstDecl*> variables = t->getDecl()->matchMember( needToMatch );
                        std::copy( variables.begin(), variables.end(), std::inserter( match, match.begin() ) );
                     }
                  }
               }
            }

            _cmp.setSandbox( sandbox );
         } else if ( type == DDCOLON )
         {
            const AstDeclClass* c = _cmp.getClass( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( prefix ) ) );
            if ( c )
            {
               std::set<AstDecl*> variables = c->matchType( needToMatch );
               std::copy( variables.begin(), variables.end(), std::inserter( match, match.begin() ) );
            }
         }

         return match;
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
      virtual std::set<mvv::Symbol> findMatch( const std::string& s, ui32& cutpoint, std::vector<std::string>& oneMatchPrototype )
      {
         std::set<AstDecl*> match = _process( s, cutpoint );

         std::set<mvv::Symbol> choices;
         for ( std::set<AstDecl*>::iterator it = match.begin(); it != match.end(); ++it )
         {
            choices.insert( (*it)->getName() );
         }

         // we have only one choice => get the corresponding declarations
         if ( choices.size() == 1 )
         {
            std::stringstream ss;
            for ( std::set<AstDecl*>::iterator it = match.begin(); it != match.end(); ++it )
            {
               VisitorPrintSimple visitor( ss );
               visitor( **it );
            }

            std::string s;
            while ( 1 )
            {
               std::getline( ss, s );
               if ( ss.eof() )
                  break;
               if ( s != "" )
                  oneMatchPrototype.push_back( s );
            }
         }
         return choices;
      }

      /**
       @brief Find the position of the last unmatched bracket or parenthesis
       @brief return the first character to be considered
       */
      static unsigned int skipMatched( const std::string& s, std::string& hidedMatch )
      {
         hidedMatch = s;

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
            {
               for ( ui32 nn = parenthesis.rbegin()->pos + 1; nn < n; ++nn )
                  hidedMatch[ nn ] = 'X';
               parenthesis.pop_back();
            }
            else if ( s[ n ] == ']' )
            {
               for ( ui32 nn = brackets.rbegin()->pos + 1; nn < n; ++nn )
                  hidedMatch[ nn ] = 'X';
               brackets.pop_back();
            }
            else if ( s[ n ] == '}' )
            {
               for ( ui32 nn = braces.rbegin()->pos + 1; nn < n; ++nn )
                  hidedMatch[ nn ] = 'X';
               braces.pop_back();
            }
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
            throw std::runtime_error( "Unhandled case!" );
         }
      }

      static void analyse( const std::string& s, ExpressionType& type, std::string& prefix, std::string& needToMatch, ui32& cut )
      {
         std::string hided;
         int skip = skipMatched( s, hided );
         std::string sub1;
         std::string subHided;
         if ( skip < (int)s.size() )
         {
            sub1 = std::string( &s[ skip ] );
            subHided = std::string( &hided[ skip ] );
         } else { type = NORMAL; prefix = ""; needToMatch = ""; return; }

         int interruptible = findInterruptible( subHided );
         std::string sub2;
         if ( interruptible < (int)sub1.size() )
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

            cut = (ui32)dotpos + (ui32)skip + (ui32)interruptible;
         } else if ( type == DDCOLON )
         {
            size_t dotpos = _findLastOf( sub2, "::" );
            ensure( dotpos != std::string::npos, "error!" );
            prefix = sub2.substr( 0, dotpos - 2 );
            needToMatch = sub2.substr( dotpos, sub2.size() );

            cut = (ui32)dotpos + (ui32)skip + (ui32)interruptible;
         } else if ( type == NORMAL )
         {
            prefix = "";
            needToMatch = sub2;

            cut = (ui32)skip + (ui32)interruptible;
         } else {ensure( 0, "not handled case!" ); }

         // strip the prefix from charachers such as " "
         int n = 0;
         while ( n < (int)needToMatch.size() )
         {
            if ( needToMatch[ n ] == ' ' || needToMatch[ n ] == '\t' || needToMatch[ n ] == '\n' )
               ++n;
            else
            {
               needToMatch = std::string( &needToMatch[ n ] );
               break;
            }
         }
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

#endif