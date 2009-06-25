#ifndef NLL_CORE_DISTRIBUTION_MULTINORMAL_H_
# define NLL_CORE_DISTRIBUTION_MULTINORMAL_H_

namespace nll
{
namespace core
{
   /**
    @brief generate normal multivariate distribution samples
           according to http://en.wikipedia.org/wiki/Multivariate_normal_distribution
    */
   class NormalMultiVariateDistribution
   {
      typedef Buffer1D<double>   Vector;
      typedef Matrix<double>     MatrixT;

   public:
      /**
       @brief initialize the generator as we need to do a cholesky decomposition.
       */
      template <class Matrix, class Vector>
      NormalMultiVariateDistribution( const Vector& mean, const Matrix& covariance )
      {
         _triangularSub.import( covariance );
         _success = choleskyDecomposition( _triangularSub );
         ensure( success, "the matrix is not a covariance matrix" );
      }

      /**
       @brief Generate samples using the mean/covariance matrix given.
              Just do sample = mean + triangular * vector, with vector=vector of N independent standard variables
       */
      template <class Vector>
      MatrixT generate() const
      {
         MatrixT r( _mean.size(), 1 );
         for ( unsigned n = 0; n < _mean.size(); ++n )
            r[ n ] = generateGaussianDistribution( 0, 1 );
         MatrixT result = _triangularSub * r;
         for ( unsigned n = 0; n < _mean.size(); ++n )
            result[ n ] += _mean[ n ];
         return result;
      }

   private:
      Vector   _mean;
      MatrixT  _triangularSub;
      bool     _success;
   };
}
}

#endif
