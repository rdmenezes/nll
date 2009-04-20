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
   template <class T>
   inline T round( const T val, T thres = 0.5 )
   {
      i32 vali = static_cast<i32>( val );
      return (val - vali >= thres ) ? ( vali + 1 ) : vali;
   }
}
}

#endif
