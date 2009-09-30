#ifndef MVV_SYMBOL_H_
# define MVV_SYMBOL_H_

# include <set>
# include <string>
# include "mvv.h"

namespace mvv
{
   class MVV_API Symbol
   {
      typedef std::set<std::string> Strings;

   public:
      static Symbol create( const std::string& s )
      {
         // return the address contained in the set, guaranteing its unicity
         std::pair<Strings::iterator, bool> it = _strings.insert( s );
         return Symbol( it.first->c_str() );
      }

      bool operator==( const Symbol& rhs ) const
      {
         return _s == rhs._s;
      }

      bool operator!=( const Symbol& rhs ) const
      {
         return _s != rhs._s;
      }

      bool operator<( const Symbol& rhs ) const
      {
         return _s < rhs._s;
      }

   private:
      Symbol( const char* s ) : _s( s )
      {
      }

   private:
      const char* _s;

   private:
      static Strings _strings;
   };
}

#endif