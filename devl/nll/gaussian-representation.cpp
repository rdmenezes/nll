#include "nll.h"

namespace nll
{
namespace algorithm
{
   GaussianMultivariateCanonical GaussianMultivariateMoment::toGaussianCanonical() const
   {
      const Matrix& covInv = getCovInv();
      const value_type detk = 1.0 / getCovDet(); // K = cov^-1

      Matrix k;
      k.clone( covInv );

      Vector h = k * Matrix( _mean, _mean.size(), 1 );

      const value_type cte1 = 0.5 * ( log( detk )
                                    - _mean.size() * log ( 2 * core::PI )
                                    - core::fastDoubleMultiplication( _mean, covInv ) );
      const value_type g = std::log( getAlpha() ) + cte1;

      VectorI id;
      id.clone( _id );

      const double debug1 = core::fastDoubleMultiplication( _mean, covInv );
      const double debug2 = _mean.size() * log ( 2 * core::PI );

      return GaussianMultivariateCanonical( h, k, g, id );
   }
}
}