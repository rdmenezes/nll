#ifndef NLL_MATH_H_
# define NLL_MATH_H_

# include "nll.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Define the PI constant
    */
   extern const double NLL_API PI;

   /**
    @ingroup core
    @brief Define the square root function to be used
    */
   template <class T>
   inline T sqr( const T val ){ return val * val; } 

   /**
    @ingroup core
    @brief Round a value to its closes integer
    */
   inline int round( double x )
   {
      return core::floor( x + 0.5 );
   }

   /**
    @brief return the sign of a value
    @return 1 for val >= 0, else -1
    */
   template <class T>
   T sign( const T v )
   {
      return ( v >= 0 ) ? 1 : -1;
   }
}
}

#endif
