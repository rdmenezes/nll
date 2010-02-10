#include "parser-context.h"

namespace mvv
{
namespace parser
{
   Ast* ParserContext::parseFile( const std::string& file )
   {
      _input = "";
      _filename = file;

      parse();

      return _root;
   }

   Ast* ParserContext::parseString( const std::string& string )
   {
      _input = string;
      _filename = "";

      parse();

      return _root;
   }
}
}