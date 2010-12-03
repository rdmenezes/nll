#ifndef NLL_MATH_DISTRIBUTION_LOGISTIC_H_
# define NLL_MATH_DISTRIBUTION_LOGISTIC_H_

# include <assert.h>

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief generate a sample from a logistic distribution
    @return a sample of this distribution
    @see http://en.wikipedia.org/wiki/Cumulative_distribution_function
    */
   inline double NLL_API generateLogisticDistribution( double mu, double s )
   {
      // generate a random value in [0..1]
      double x = generateUniformDistribution( 0, 1 );

      // compute y = F^-1( x )
      return mu + s * log( x / ( 1 - x ) );
   }

   /**
    @ingroup core
    @brief return the logistic of a value
    */
   inline double NLL_API  logistic( double x, double mu, double s )
   {
      return 1 / ( 1 + exp( - ( x - mu ) / s ) );
   }
}
}

#endif
