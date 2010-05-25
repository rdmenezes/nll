#ifndef MVV_PLATFORM_RESOURCE_VALUE_H_
# define MVV_PLATFORM_RESOURCE_VALUE_H_

# include "mvvPlatform.h"
# include "resource.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Hold a simple value, notify() this object if the value is changed.
    */
   template <class T>
   class ResourceValue : public Resource<T>
   {
   public:
      ResourceValue( bool simple = false ) : Resource( new T(), true, simple )
      {
      }

      void setValue( T val )
      {
         if ( getValue() != val )
         {
            getValue() = val;
            notify();
         }
      }

      T& getValue()
      {
         return Resource<T>::getValue();
      }

      const T& getValue() const
      {
         return Resource<T>::getValue();
      }
   };
}
}

#endif
