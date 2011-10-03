#include "nll.h"

namespace nll
{
namespace algorithm
{
   GaussianMultivariateCanonical GaussianMultivariateMoment::toGaussianCanonical() const
   {
      const Matrix& covInv = getCovInv();

      Matrix k;
      k.clone( covInv );

      Vector h = k * Matrix( _mean, _mean.size(), 1 );

      // g = log (alpha) - 0.5 * log (2*PI*det(cov)) - 0.5 mean^t * covinv * mean
      const value_type cte1 = log ( 2 * core::PI * getCovDet() );
      const value_type cte2 = - 0.5 * cte1 - 0.5 * core::fastDoubleMultiplication( _mean, covInv );
      const value_type g = std::log( getAlpha() ) + cte2;

      VectorI id;
      id.clone( _id );

      return GaussianMultivariateCanonical( h, k, g, id );
   }
}
}