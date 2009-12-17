#ifndef MVV_SYMBOL_PLATFORM_TYPED_H_
# define MVV_SYMBOL_PLATFORM_TYPED_H_

# include <set>
# include <string>
# include <utility>
# include "mvvPlatform.h"
# include "symbol.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Class representing a lightweight string. Only one instance for each string can be created,
           so we can compare directly string internal buffer pointer for comparison

           In addition, it is specific to a type, so we can't convert a SymbolTyped<Volume> to SymbolTyped<Lut>, adding
           compile time safety checks.
    */
   template <class T>
   class SymbolTyped
   {
      typedef std::set<std::string> Strings;

   public:
      typedef T   value_type;

   public:
      static SymbolTyped create( const std::string& s )
      {
         // return the address contained in the set, guaranteing its unicity
         std::pair<Strings::iterator, bool> it = _strings.insert( s );
         return SymbolTyped( it.first->c_str() );
      }

      bool operator==( const SymbolTyped& rhs ) const
      {
         return _s == rhs._s;
      }

      bool operator!=( const SymbolTyped& rhs ) const
      {
         return _s != rhs._s;
      }

      bool operator<( const SymbolTyped& rhs ) const
      {
         return _s < rhs._s;
      }

      const char* getName() const
      {
         return _s;
      }

   private:
      // to be created internally only!
      SymbolTyped( const char* s ) : _s( s )
      {
      }

   private:
      const char* _s;

   private:
      static Strings _strings;
   };

   template <class T>
   typename SymbolTyped<T>::Strings SymbolTyped<T>::_strings = SymbolTyped<T>::Strings();
}
}

#endif