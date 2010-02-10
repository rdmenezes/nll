#include "parser-context.h"

namespace mvv
{
namespace parser
{
   Ast* ParserContext::parseFile( const std::string& file )
   {
      _input = "";
      _filename = file;

      _parse();

      return _root;
   }

   Ast* ParserContext::parseString( const std::string& string )
   {
      _input = string;
      _filename = "";

      _parse();

      return _root;
   }

   void ParserContext::_parse()
   {
      _root = 0;
      _scanOpen();

      int result = 1;
      if ( !_error.getStatus() )
      {
         // parse a file only if the context doesn't have any error
         result = yyparse( *this );
      }
      _scanClose();

      // if parsing fails, deallocate the current AST if any
      // we won't need it...
      if ( result )
      {
         delete _root;
         _root = 0;
      }
   }
}
}