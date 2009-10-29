#ifndef NLL_MATH_FLOOR_H_
# define NLL_MATH_FLOOR_H_

# include <cmath>

namespace nll
{
namespace core
{
#if defined( _MSC_VER ) && defined( _M_IX86 )
   //
   // optimized implementation
   //

   /**
    * Calculates the floor of a value. 250% increase
    * @param x floating-point value.
    * @return the largest integer that is less than or equal to <em>x</em>.
    */
   int NLL_API floor( double x );

   /**
    * Calculates the integer value corresponding to the truncation of a value. 140% increase
    * @param x floating-point value.
    * @return the integer with the largest magnitude that is less than or equal to that of <em>x</em>, and with the same sign.
   */
   unsigned int NLL_API truncateu( double x );

#else
   //
   // default implementation
   //
   inline int NLL_API floor( double x )
   {
      return (int)std::floor( x );
   }

   inline unsigned int NLL_API truncateu( double x )
   {
      return (unsigned int)x;
   }
#endif
}
}

#endif
