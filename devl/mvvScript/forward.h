#ifndef MVV_PARSER_FORWARD_H_
# define MVV_PARSER_FORWARD_H_

# include <mvvPlatform/types.h>
# include "mvvScript.h"

# define YY_NO_UNISTD_H
# define isatty(x) false

namespace yy
{
  // From parsetiger.yy.
  class parser;
  class location;
}

// From scantiger.cc.
union YYSTYPE;

// From scantiger.cc.
struct yy_buffer_state;

// Define our location type
struct MVVSCRIPT_API YYLTYPE
{
   YYLTYPE() : filename( mvv::Symbol::create("(input)") )
   {
      first_line = 1;
      first_column = 0;
      last_line = 1;
      last_column = 0;
   }

  int first_line;
  int first_column;
  int last_line;
  int last_column;
  mvv::Symbol filename;
};

// define the printing of our location
inline std::ostream& operator<<( std::ostream& o, const YYLTYPE& loc )
{
   o << loc.filename.getName() << ": L"  << loc.first_line << "." << loc.first_column
                               << "-L"   << loc.last_line  << "." << loc.last_column
                               << " ";
   return o;
}

# define YYLTYPE_IS_DECLARED 1

namespace mvv
{
namespace parser
{
   class ParserContext;
}
}

#endif
