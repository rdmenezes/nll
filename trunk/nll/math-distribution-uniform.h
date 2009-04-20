#ifndef NLL_MATH_DISTRIBUTION_UNIFORM_H_
# define NLL_MATH_DISTRIBUTION_UNIFORM_H_

# include <assert.h>

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief generate a sample of a specific uniform distribution
    @param min the min of the distribution
    @param max the max of the distribution
    @return a sample of this distribution
    */
   inline double NLL_API generateUniformDistribution( const double min, const double max )
   {
      assert( min <= max );
      return static_cast<double>( rand() ) / RAND_MAX * ( max - min ) + min;
   }
}
}

#endif
