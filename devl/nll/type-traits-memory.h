#ifndef NLL_TYPE_TRAITS_MEMORY_H_
# define NLL_TYPE_TRAITS_MEMORY_H_

# include "type-traits.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief value == 1 if a Plain Old Data type. The client can extend this with its own structures. Certain
           optimizations will then be available.
    */
   template <typename T>
   struct IsPOD
   {
      enum{ value=IsNativeType<T>::value };
   };

   /**
    @ingroup core
    @brief value == 0 is T can throw an exception on copy. The client can extend this with its own structures. Certain
           optimizations will then be available.
    */
   template <typename T>
   struct CopyThrows
    {
       enum { value = IsNativeType<T>::value == 0 && IsPOD<T>::value == 0 };
    };

   /**
    @ingroup core
    @brief value == 1 if T is copyable (meaning it is a plain structure where we can call memcpy to copy the
           the data). The client can extend this with its own structures. Certain
           optimizations will then be available.
    */
   template <class T>
   struct IsMemCopyable
   {
      enum{ value = IsNativeType<T>::value || !CopyThrows<T>::value || IsPOD<T>::value };
   };

   /**
    @ingroup core
    @brief value == 1 if the data type is cheap to copy (ie doesn't throw and small enough). The client can extend this with its own structures. Certain
           optimizations will then be available.
    */
   template <typename T>
   struct CheapToCopy
   {
      enum { value = CopyThrows<T>::value == 0 && sizeof(T) <= 4 * sizeof(T*) };
   };
}
}

#endif
