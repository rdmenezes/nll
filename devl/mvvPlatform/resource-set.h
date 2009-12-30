#ifndef MVV_PLATFORM_RESOURCE_SET_H_
# define MVV_PLATFORM_RESOURCE_SET_H_

# include <set>
# include "resource.h"
# include "types.h"
# include "order.h"

namespace mvv
{
namespace platform
{
   /**
    @brief Hold a set of values. If the set is modified, the resource is notified
    @note be carreful that we hold a resource on resources!! Watch out for side effects in case of multithreading!
    */
   template <class T>
   class ResourceSetRef : public Resource< std::set< RefcountedTyped<T> > >
   {
      typedef std::set< RefcountedTyped<T> > Storage;
      typedef Resource< std::set< RefcountedTyped<T> > >  Base;

   public:
      typedef T   value_type;

   public:
      class Iterator
      {
      public:
         Iterator( typename Storage::iterator it ) : _it( it )
         {}

         bool operator==( const Iterator& rhs ) const
         {
            return _it == rhs._it;
         }

         bool operator!=( const Iterator& rhs ) const
         {
            return _it != rhs._it;
         }

         RefcountedTyped<T> operator*()
         {
            return *_it;
         }

         Iterator& operator++()
         {
            ++_it;
            return *this;
         }

      //private:
         typename Storage::iterator _it;
      };

      class ConstIterator
      {
      public:
         ConstIterator( typename Storage::const_iterator it ) : _it( it )
         {}

         ConstIterator( Iterator it ) : _it( it._it )
         {}

         bool operator==( const ConstIterator& rhs ) const
         {
            return _it == rhs._it;
         }

         bool operator!=( const ConstIterator& rhs ) const
         {
            return _it != rhs._it;
         }

         RefcountedTyped<T> operator*() const
         {
            return *_it;
         }

         ConstIterator& operator++()
         {
            ++_it;
            return *this;
         }

      private:
         typename Storage::const_iterator _it;
      };

   public:
      ResourceSetRef() : Base( new Base::value_type(), true )
      {}

      void insert( RefcountedTyped<T> val )
      {
         if ( getValue().insert( val ).second )
         {
            notify();
         }
      }

      void erase( RefcountedTyped<T> val )
      {
         Storage::iterator it = getValue().find( val );
         if ( it != getValue().end() )
         {
            getValue().erase( it );
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

      ui32 size() const
      {
         const Storage& set = getValue();
         ui32 size = static_cast<ui32>( set.size() );
         return size;
      }
   };

   /**
    @brief Hold a set of values. If the set is modified, the resource is notified
    */
   template <class T>
   class ResourceSet : public Resource< std::set< T > >
   {
      typedef std::set< T > Storage;
      typedef Resource< std::set< T > >  Base;

   public:
      typedef T   value_type;

   public:
      ResourceSet() : Base( new Base::value_type(), true )
      {}

      void insert( T val )
      {
         if ( getValue().insert( val ).second )
         {
            notify();
         }
      }

      void erase( T val )
      {
         Storage::iterator it = getValue().find( val );
         if ( it != getValue().end() )
         {
            getValue().erase( it );
            notify();
         }
      }

      void clear()
      {
         getValue().clear();
         notify();
      }
   };
}
}

#endif