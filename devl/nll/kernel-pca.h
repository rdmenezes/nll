#ifndef NLL_ALGORITHM_KERNEL_PCA_H_
# define NLL_ALGORITHM_KERNEL_PCA_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Nonlinear Component Analysis as a Kernel Eigenvalue Problem. This is an extension of the PCA algorithm
           in a high dimentionality feaure space.

    It is a direct implementaion of the article from Bernhard Scholkopf, Alexander Smola and Klaus Robert Muller
    on kernel component analysis. See here for the details:http://www.face-rec.org/algorithms/Kernel/kernelPCA_scholkopf.pdf

    The kernel must be serializable (read, write) and operator()( Point, Point )
    */
   template <class Point, class Kernel>
   class KernelPca
   {
   public:
      typedef core::Matrix<double>     Matrix;
      typedef core::Buffer1D<double>   Vector;

   public:
      KernelPca() : _kernel( 0 )
      {
      }

      ~KernelPca()
      {
         delete _kernel;
      }

      /**
       @brief Compues the <code>nbFeatures</code> most important features in a high dimentional space defined
              by the kernel function. Memory consuption is 2 * point.size^2. If we don't have enough memory,
              just take a representative sample of the points.

              first we compute the centered kernel matrix kernel_ij = kernel( x_i, x_j ), x the input vectors

              Then this matrix is centered in the feature space

              we want to solve: M * lambda = kernel * lambda (1) so we compute eigen values/vectors of kernel
                                                                 and renormalize the eigen vectors
              we defined a feature V = sum (lambda_i * H(x_i) ) (2)  | same lambda in (1) and (2)

       @param points the set of points. needs to define Point operator[](n) const and size()
       @param nbFeatures the number of features used for the feaure space. It could range from 1 to the number
              of points.
       @param kernel A kernel function. It must define the double operator(Point, Point) const.
       @return false if any error occured
       */
      template <class Points>
      bool compute( const Points& points, ui32 nbFeatures, const Kernel& kernel )
      {
         ensure( points.size(), "no point to compute" );
         ensure( nbFeatures <= points.size() && nbFeatures > 0, "error in the number of features to be selected" );

         _kernel = kernel.clone();
         Matrix centeredKernel = _computeKernelMatrix( points, kernel );
         bool diagonalization = _diagonalize( centeredKernel, _eig, _x, nbFeatures, points );
         if ( !diagonalization )
            return false;

         return true;
      }

      /**
       @brief project the point on the main components found
       */
      template <class Point2>
      Vector transform( const Point2& p ) const
      {
         const ui32 nbEigenVectors = _eig.sizex();
         ensure( nbEigenVectors && _eig.sizey(), "compute first the model parameters" );
         ensure( p.size() == _x.sizey(), "point size error" );
         ensure( _kernel, "something wrong happened..." );


         Vector kernel( nbEigenVectors );
         Vector centeredKernel( nbEigenVectors );
         Vector projected( nbEigenVectors );
         Point x( p.size() );
         Point t( p.size() );
         for ( ui32 nn = 0; nn < p.size(); ++nn )
            t[ nn ] = p[ nn ];

         // precompute the projection
         double sumA = 0;
         for ( ui32 k = 0; k < nbEigenVectors; ++k )
         {
             for ( ui32 nn = 0; nn < p.size(); ++nn )
               x[ nn ] = _x( nn, k );
            kernel[ k ] = (*_kernel)( x, t );
            sumA += kernel[ k ];
         }

         // center the test kernel matrix
         const double constVal = 1.0 / nbEigenVectors;
         for ( ui32 j = 0; j < nbEigenVectors; ++j )
         {
            centeredKernel[ j ] = kernel[ j ] - constVal * sumA
                                              - constVal * _sumA[ j ]
                                              + constVal * constVal;
         }

         // project the input on the features
         for ( ui32 k = 0; k < nbEigenVectors; ++k )
         {
            double sum = 0;
            for ( ui32 i = 0; i < nbEigenVectors; ++i )
            {
               sum += _eig( i, k ) * centeredKernel[ i ];
            }
            projected[ k ] = sum;
         }
         return projected;
      }

   private:
      /**
       @brief Compute the kernel matrix. It is a size(points) * size(points) matrix, centered in the feature
              space.
       */
      template <class Points>
      Matrix _computeKernelMatrix( const Points& points, const Kernel& kernel )
      {
         // compute the not centered kernel matrix
         Matrix kernelBase( static_cast<ui32>( points.size() ), static_cast<ui32>( points.size() ) );
         for ( ui32 i = 0; i < points.size(); ++i )
            for ( ui32 j = i; j < points.size(); ++j )
            {
               kernelBase( i, j ) = kernel( points[ i ], points[ j ] );
               kernelBase( j, i ) = kernelBase( i, j );
            }

         // precompute the sums
         _sumA = Vector( static_cast<ui32>( points.size() ) );
         Vector sumB( static_cast<ui32>( points.size() ) );
         _sumC = 0;
         for ( ui32 m = 0; m < points.size() * points.size(); ++m )
            _sumC += kernelBase[ m ];
         for ( ui32 i = 0; i < points.size(); ++i )
         {
            for ( ui32 m = 0; m < points.size(); ++m )
                  _sumA[ i ] += kernelBase( m, i );
            for ( ui32 m = 0; m < points.size(); ++m )
                  sumB[ i ] += kernelBase( i, m );
         }
         const double m1 = 1 / static_cast<double>( points.size() );

         // center the kernel
         Matrix mkernel( kernelBase.sizey(), kernelBase.sizex() );
         for ( ui32 i = 0; i < points.size(); ++i )
            for ( ui32 j = i; j < points.size(); ++j )
            {               
               mkernel( i, j ) = kernelBase( i, j ) - m1 * _sumA[ j ] - m1 * sumB[ i ] + m1 * m1 * _sumC;
               mkernel( j, i ) = mkernel( i, j );
            }
         kernelBase.unref();
         return mkernel;
      }

      /**
       @brief compute the eigen vectors & values of the kernel matrix. Eigen vectors are normalized so that
       ( V.V = 1 )

       we compute eiv, eig of:
       M * eiv * eig = Kernel * eig

       @note The centeredKernel matrix is not valid after the call of this function as the <code>svdcmp</code> will
       modify it
       */
      template <class Points>
      bool _diagonalize( Matrix& centeredKernel, Matrix& outEigenVectors, Matrix& outVectors, ui32 nbFeatures, const Points& points  )
      {
         const ui32 size = centeredKernel.sizex();
         const ui32 inputPointSize = static_cast<ui32>( points[ 0 ].size() );

         // compute eigen values, eigen vectors
         Matrix eigenVectors;
         Vector eigenValues;
         bool res = core::svdcmp( centeredKernel, eigenValues, eigenVectors );

         // check if error
         if ( !res )
            return false;

         // sort the eigen values from highest to lowest
         typedef std::pair<double, ui32>  Pair;
         typedef std::vector<Pair>        Pairs;
         Pairs pairs( size );
         for ( ui32 n = 0; n < size; ++n )
            pairs[ n ] = std::make_pair( eigenValues[ n ], n );
         std::sort( pairs.rbegin(), pairs.rend() );

         // compute the number of eigen values according to the number of features & feature space dim
         for ( ui32 n = 0; n < nbFeatures; ++n )
            if ( pairs[ n ].first <= 1e-3 )
            {
               nbFeatures = n;
               break;
            }
         if ( nbFeatures == 0 )
            return false;

         // export the eigen vectors/values we are interested in
         outEigenVectors = Matrix( size, nbFeatures );
         outVectors = Matrix( inputPointSize, nbFeatures );

         for ( ui32 n = 0; n < nbFeatures; ++n )
         {
            const ui32 index = pairs[ n ].second;
            const double norm = sqrt( eigenValues[ index ] );
            for ( ui32 nn = 0; nn < size; ++nn )
               outEigenVectors( nn, n ) = eigenVectors( nn, index ) / norm;

            for ( ui32 nn = 0; nn < inputPointSize; ++nn )
               outVectors( nn, n ) = points[ index ][ nn ];
         }
         return true;
      }

   private:
      Matrix   _eig;             /// eigen vectors, stored column (1 col = 1 eigen vector)
      Matrix   _x;               /// the vectors associated with the principal component, 1 col = 1 vector
      Kernel*  _kernel;          /// the kernel function  used by the algorithm

      // variables needed to precompute the feature extraction
      Vector   _sumA;
      double   _sumC;
   };
}
}

#endif
