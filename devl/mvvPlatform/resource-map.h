#ifndef MVV_PLATFORM_RESOURCE_MAP_H_
# define MVV_PLATFORM_RESOURCE_MAP_H_

# include <map>
# include "resource.h"

namespace mvv
{
namespace platform
{
   template <class Key, class Value>
   class ResourceMap : public Resource< std::map<Key, Value> >
   {
   public:
      typedef Resource< std::map<Key, Value> >  Base;
      typedef std::map<Key, Value>              BaseValue;
      typedef Value                             value_type;
      typedef Key                               key;

   public:
      class Iterator
      {
         friend class ConstIterator;

      public:
         Iterator( typename BaseValue::iterator it ) : _it( it )
         {
         }

         bool operator==( Iterator rhs ) const
         {
            return _it == rhs._it;
         }

         bool operator!=( Iterator rhs ) const
         {
            return _it != rhs._it;
         }

         Iterator& operator++()
         {
            ++_it;
            return *this;
         }

         std::pair<Key, Value> operator*()
         {
            return *_it;
         }

      private:
         typename BaseValue::iterator  _it;
      };

      class ConstIterator
      {
      public:
         ConstIterator( typename BaseValue::const_iterator it ) : _it( it )
         {
         }

         ConstIterator( Iterator it ) : _it( it._it )
         {
         }

         bool operator==( ConstIterator rhs ) const
         {
            return _it == rhs._it;
         }

         bool operator!=( ConstIterator rhs ) const
         {
            return _it != rhs._it;
         }

         ConstIterator& operator++()
         {
            ++_it;
            return *this;
         }

         std::pair<Key, Value> operator*()
         {
            return *_it;
         }

      private:
         typename BaseValue::const_iterator  _it;
      };

      ResourceMap() : Resource( new BaseValue(), true )
      {
      }

      void insert( Key k, Value v )
      {
         getValue()[ k ] = v;
         notify();
      }

      void erase( Key k )
      {
         typename BaseValue::iterator it = getValue().find( k );
         if ( k != getValue().end() )
         {
            getValue().erase( k );
            notify();
         }
      }

      Iterator begin()
      {
         return Iterator( getValue().begin() );
      }

      Iterator end()
      {
         return Iterator( getValue().end() );
      }

      ConstIterator begin() const
      {
         return ConstIterator( getValue().begin() );
      }

      ConstIterator end() const
      {
         return ConstIterator( getValue().end() );
      }

      bool find( const Key k, Value& out )
      {
         typename BaseValue::iterator it = getValue().find( k );
         if ( it != getValue().end() )
         {
            out = (*it).second;
            return true;
         }
         return false;
      }
   };
}
}

#endif