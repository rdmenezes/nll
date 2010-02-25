#ifndef MVV_PARSER_INDENT_H_
# define MVV_PARSER_INDENT_H_

# include <ostream>
# include "mvvScript.h"

namespace mvv
{
namespace parser
{
   /// Increment the indentation.
  MVVSCRIPT_API std::ostream& incindent(std::ostream& o);

  /// Decrement the indentation.
  MVVSCRIPT_API std::ostream& decindent(std::ostream& o);

  /// Reset the indentation.
  MVVSCRIPT_API std::ostream& resetindent(std::ostream& o);

  /// Print an end of line, then set the indentation.
  MVVSCRIPT_API std::ostream& iendl(std::ostream& o);

  /// Increment the indentation, print an end of line, and set the indentation.
  MVVSCRIPT_API std::ostream& incendl(std::ostream& o);

  /// Decrement the indentation, print an end of line, and set the indentation.
  MVVSCRIPT_API std::ostream& decendl(std::ostream& o);
}
}

#endif