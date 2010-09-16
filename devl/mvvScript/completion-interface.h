#ifndef MVV_PARSER_COMPLETION_INTERFACE_H_
# define MVV_PARSER_COMPLETION_INTERFACE_H_

# include <mvvPlatform/symbol.h>

namespace mvv
{
namespace parser
{
   class CompletionInterface
   {
   public:
      virtual std::set<mvv::Symbol> findMatch( const std::string& s, ui32& cutpoint ) = 0;
   };
}
}

#endif