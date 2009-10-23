#ifndef NLL_MATH_FLOOR_H_
# define NLL_MATH_FLOOR_H_

namespace nll
{
namespace core
{
#if defined( _MSC_VER ) && defined( _M_IX86 )
   int NLL_API floor( double x );
#else
   inline int NLL_API floor( double x )
   {
      return (int)std::floor( x );
   }
#endif
}
}

#endif
