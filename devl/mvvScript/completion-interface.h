#ifndef MVV_PARSER_COMPLETION_INTERFACE_H_
# define MVV_PARSER_COMPLETION_INTERFACE_H_

# include <mvvPlatform/symbol.h>

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API CompletionInterface
   {
   public:
      virtual std::set<mvv::Symbol> findMatch( const std::string& s ) = 0;
   };
}
}

#endif