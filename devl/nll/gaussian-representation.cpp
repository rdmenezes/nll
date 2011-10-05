#include "nll.h"

namespace nll
{
namespace algorithm
{
   GaussianMultivariateCanonical GaussianMultivariateMoment::toGaussianCanonical() const
   {
      const Matrix& covInv = getCovInv();
      ensure( getCovDet() > 0, "determinant K must be > 0" );
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
      return GaussianMultivariateCanonical( h, k, g, id );
   }
}
}