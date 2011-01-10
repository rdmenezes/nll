#ifndef NLL_CORE_MATH_STATISTIC_INDEPENDENCE_H_
# define NLL_CORE_MATH_STATISTIC_INDEPENDENCE_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Assume point is a set of random variable at t=0...size,
           computes E(f(x))E(f(y)) - E(f(x)(f(y)) (1)
    @note if 2 random variable are independent, they must satisfy (1) == 0 for any function f
    @return if the 2 features are statistically independent returns 0 for any function f
    @TODO this is not good at all! it is just testing with func id, but should be with much more funcs...
    */
   template <class Points>
   double checkStatisticalIndependence( const Points& points, ui32 v1, ui32 v2 )
   {
      double xa = 0;
      double xb = 0;
      double xc = 0;
      for ( ui32 n = 0; n < points.size(); ++n )
      {
         xa += ( points[ n ][ v1 ] );
         xb += ( points[ n ][ v2 ] );
         xc += ( points[ n ][ v1 ] ) * ( points[ n ][ v2 ] );
      }
      return xa / points.size() * xb / points.size() - xc / points.size();
   }
}
}

#endif