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
    */
   template <class T>
   class ResourceSetRef : public Resource< std::set< RefcountedTyped<T> > >
   {
      typedef std::set< RefcountedTyped<T> > Storage;
      typedef Resource< std::set< RefcountedTyped<T> > >  Base;

   public:
      typedef T   value_type;

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
   };
}
}

#endif