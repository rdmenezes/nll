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
   class ResourceSet : public Resource< std::set< T > >
   {
   public:
      typedef std::set< T > Storage;
      typedef Resource< std::set< T > >  Base;
      typedef T   value_type;
      typedef typename Storage::iterator Iterator;

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

      Iterator begin()
      {
         return getValue().begin();
      }

      Iterator end()
      {
         return getValue().end();
      }

      void clear()
      {
         getValue().clear();
         notify();
      }

      ui32 size() const
      {
         const Storage& set = getValue();
         ui32 size = static_cast<ui32>( set.size() );
         return size;
      }
   };
}
}

#endif