#ifndef MVV_PARSER_INDENT_H_
# define MVV_PARSER_INDENT_H_

# include <ostream>

namespace mvv
{
namespace parse
{
   /// Increment the indentation.
  std::ostream& incindent(std::ostream& o);

  /// Decrement the indentation.
  std::ostream& decindent(std::ostream& o);

  /// Reset the indentation.
  std::ostream& resetindent(std::ostream& o);

  /// Print an end of line, then set the indentation.
  std::ostream& iendl(std::ostream& o);

  /// Increment the indentation, print an end of line, and set the indentation.
  std::ostream& incendl(std::ostream& o);

  /// Decrement the indentation, print an end of line, and set the indentation.
  std::ostream& decendl(std::ostream& o);
}
}

#endif