#ifndef MVV_PLATFORM_RESOURCE_MAP_H_
# define MVV_PLATFORM_RESOURCE_MAP_H_

# include <map>
# include "resource.h"

namespace mvv
{
namespace platform
{
   /**
    @brief normal map converted with Resource behaviour (a modification of the Value, will not trigger an update of the container...)
    */
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
         if ( it != getValue().end() )
         {
            getValue().erase( k );
            notify();
         }
      }

      void clear()
      {
         getValue().clear();
         notify();
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

   /**
    @brief Resource map containing resources, where one modification of a resource implies the notification of the container...
    */
   template <class Key, class Value>
   class ResourceMapResource : public Resource< std::map<Key, Value> >
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

      ResourceMapResource() : Resource( new BaseValue(), true )
      {
      }

      void insert( Key k, Value v )
      {
         getValue()[ k ] = v;
         v.connect( *this );
         notify();
      }

      void erase( Key k )
      {
         typename BaseValue::iterator it = getValue().find( k );
         if ( it != getValue().end() )
         {
            it->second.disconnect( *this );
            getValue().erase( k );
            notify();
         }
      }

      void clear()
      {
         //
         // TODO test
         //
         BaseValue& base = getValue();
         while ( base.size() )
         {
            typename BaseValue::iterator it = base.begin();
            base.erase( it );
         }


         getValue().clear();
         notify();
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
         BaseValue& base = getValue();
         if ( it != getValue().end() )
         {
            ResourceState s = (*it).second.getState();
            (*it).second.setState( STATE_DISABLED );
            out = (*it).second;  // else if resource, it will force notification => we don't want that!
            (*it).second.forceNeedNotification( false );
            (*it).second.setState( s );

            return true;
         }
         return false;
      }
   };
}
}

#endif