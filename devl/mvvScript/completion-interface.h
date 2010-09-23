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
      /**
       @brief compute all the declarations that match with <code>s</code>
       @param s the string to match
       @param cutpoint the point from where the string must be replaced by the matching strings
       @param oneMatchPrototype if the findMatch finds only one symbol, then this returns all the declarations for this symbol
       */
      virtual std::set<mvv::Symbol> findMatch( const std::string& s, ui32& cutpoint, std::vector<std::string>& oneMatchPrototype ) = 0;
      virtual void getType( const std::string& s, ui32& cutpoint, std::vector<std::string>& oneMatchPrototype ) = 0;
   };
}
}

#endif