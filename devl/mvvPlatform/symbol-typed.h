#ifndef MVV_SYMBOL_PLATFORM_TYPED_H_
# define MVV_SYMBOL_PLATFORM_TYPED_H_

# include <set>
# include <string>
# include <utility>
# include "mvvPlatform.h"

namespace mvv
{
namespace platform
{
   /**
    @brief Class representing a lightweight string. 
    */
   template <class T>
   class SymbolTyped
   {
      struct Value
      {
         Value( const std::string& n, const T v ) : name( n ), value( v )
         {}
         const std::string name;
         const T           value;

         bool operator<( const Value& v ) const
         {
            return name.c_str() < v.name.c_str();
         }

      private:
         Value& operator=( const Value& );
      };
      typedef std::set<Value> Strings;

   public:
      typedef T   value_type;

   public:
      static SymbolTyped create( const std::string& s, const T value )
      {
         // return the address contained in the set, guaranteing its unicity
         std::pair<Strings::iterator, bool> it = _strings.insert( Value( s, value ) );
         return SymbolTyped( it.first->name.c_str(), it.first->value );
      }

      static bool get( const std::string& s, SymbolTyped& out )
      {
         // return the address contained in the set, guaranteing its unicity
         std::pair<Strings::iterator, bool> it = _strings.insert( Value( s, value ) );
         if ( !it.second )
            return false;
         out = *it.first;
         return true;
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
      SymbolTyped( const char* s, const T value ) : _s( s ), _v( value )
      {
      }

   private:
      const char* _s;
      T _v;

   private:
      static Strings _strings;
   };
}
}

#endif