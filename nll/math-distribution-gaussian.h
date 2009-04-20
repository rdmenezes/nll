#ifndef NLL_MATH_DISTRIBUTION_GAUSSIAN_H_
# define NLL_MATH_DISTRIBUTION_GAUSSIAN_H_

# include <cmath>

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief generate a sample of a specific gaussian distribution
    @param mean the mean of the distribution
    @param variance the variance of the distribution
    @return a sample of this distribution
    @note 2 values instead of 1 could be generated for almost free
    */
   inline double NLL_API generateGaussianDistribution( const double mean, const double variance )
   {
      double u1 = static_cast<double> ( ( rand() ) + 1 ) / RAND_MAX;
      double u2 = static_cast<double> ( ( rand() ) + 1 ) / RAND_MAX;
      double t1 = sqrt( -2 * log( u1 ) ) * cos( 2 * core::PI * u2 );

      return mean + variance * t1;
   }
}
}

#endif
