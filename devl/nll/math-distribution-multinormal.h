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
      typedef Matrix<double>     MatrixT;

   public:
      typedef Buffer1D<double>   Vector;

   public:
      /**
       @brief initialize the generator as we need to do a cholesky decomposition.
       */
      template <class Matrix, class Vector>
      NormalMultiVariateDistribution( const Vector& mean, const Matrix& covariance )
      {
         _triangularSub.import( covariance );
         _mean.clone( mean );
         _success = choleskyDecomposition( _triangularSub );
         ensure( _success, "the matrix is not a covariance matrix" );
      }

      /**
       @brief Generate samples using the mean/covariance matrix given.
              Just do sample = mean + triangular * vector, with vector=vector of N independent standard variables
       */
      Vector generate() const
      {
         MatrixT r( _mean.size(), 1 );
         for ( unsigned n = 0; n < _mean.size(); ++n )
            r[ n ] = generateGaussianDistribution( 0, 1 );
         MatrixT result = _triangularSub * r;
         Vector final( _mean.size() );
         for ( unsigned n = 0; n < _mean.size(); ++n )
            final[ n ] = result[ n ] + _mean[ n ];
         return final;
      }

   private:
      Vector   _mean;
      MatrixT  _triangularSub;
      bool     _success;
   };
}
}

#endif
