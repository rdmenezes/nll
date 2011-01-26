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

    @note the memory consuption is around size * size * (1 + nb_eig/nbPoint)
    */
   template <class Point, class Kernel>
   class KernelPca
   {
   public:
      typedef core::Matrix<double>     Matrix;
      typedef core::Buffer1D<double>   Vector;

   public:
      KernelPca()
      {
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
      bool compute( const Points& points, ui32 nbFeatures, const Kernel& kernel, double minEigenValueToSelect = 1e-4 )
      {
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "kernel PCA, learning started" );
         ensure( points.size(), "no point to compute" );
         ensure( nbFeatures <= points.size() && nbFeatures > 0, "error in the number of features to be selected" );

         _kernel = std::auto_ptr<Kernel>( kernel.clone() );
         Matrix centeredKernel = _computeKernelMatrix( points, kernel );

         bool diagonalization = _diagonalize( centeredKernel, _eig, nbFeatures, minEigenValueToSelect );
         if ( !diagonalization )
         {
            core::LoggerNll::write( core::LoggerNll::ERROR, " kernel PCA failed: cannot diagonalize the covariance matrix" );
            return false;
         }

         const ui32 size = static_cast<ui32>( points.size() );
         _points = std::vector<Point>( size );
         for ( ui32 n = 0; n < size; ++n )
         {
            _points[ n ] = points[ n ];
         }
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
         ensure( _points.size(), "no support vectors!" );
         ensure( p.size() == _points[ 0 ].size(), "point size error" );
         ensure( _kernel.get(), "something wrong happened..." );

         // convert to a point
         Point t;
         core::convert( p, t );
      
         double sumA = 0;
         Vector kernel( static_cast<ui32>( _points.size() ) );
         for ( ui32 n = 0; n < kernel.size(); ++n )
         {
            kernel[ n ] = (*_kernel)( t, _points[ n ] );
            sumA += kernel[ n ];
         }

         Vector projected( nbEigenVectors );
         for ( ui32 k = 0; k < nbEigenVectors; ++k )
         {
            double sum = 0;
            for ( ui32 i = 0; i < kernel.size(); ++i )
            {
               sum += _eig( i, k ) * kernel[ i ];
            }
            projected[ k ] = sum;
         }

         const double cte = sumA / kernel.size() - _sumC;
         for ( ui32 k = 0; k < nbEigenVectors; ++k )
         {
            projected[ k ] -= _sumB[ k ] * cte + _sumA[ k ];
         }

         return projected;
      }

      bool write( std::ostream& o ) const
      {
         _eig.write( o );

         core::write<ui32>( static_cast<ui32>( _points.size() ), o );
         for ( ui32 n = 0; n < _points.size(); ++n )
         {
            Vector p;
            core::convert( _points[ n ], p );
            core::write<Vector>( p, o );
         }

         _kernel->write( o );
         core::write<Vector>( _sumA, o );
         core::write<Vector>( _sumB, o );
         core::write<double>( _sumC, o );
         return true;
      }

      bool read( std::istream& i )
      {
         _eig.read( i );

         ui32 nbPoints;
         core::read<ui32>( nbPoints, i );
         _points = std::vector<Point>( nbPoints );
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            Vector p;
            core::read<Vector>( p, i );
            core::convert( p, _points[ n ] );
         }

         _kernel = std::auto_ptr<Kernel>( new Kernel( i ) );
         core::read<Vector>( _sumA, i );
         core::read<Vector>( _sumB, i );
         core::read<double>( _sumC, i );
         return true;
      }
   private:
      KernelPca& operator=( const KernelPca& );
      KernelPca( const KernelPca& );

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
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "kernel=" );
         std::stringstream ss;
         kernelBase.print( ss );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );


         // precompute the sums
         _sumA = Vector( static_cast<ui32>( points.size() ) );
         _sumC = 0;
         for ( ui32 m = 0; m < points.size() * points.size(); ++m )
            _sumC += kernelBase[ m ];
         for ( ui32 i = 0; i < points.size(); ++i )
         {
            for ( ui32 m = 0; m < points.size(); ++m )
                  _sumA[ i ] += kernelBase( m, i );
         }
         const double m1 = 1 / static_cast<double>( points.size() );

         for ( ui32 i = 0; i < points.size(); ++i )
         {
            _sumA[ i ] *= m1;
         }
         _sumC *= m1 * m1;

         // center the kernel
         Matrix mkernel( kernelBase.sizey(), kernelBase.sizex() );
         for ( ui32 i = 0; i < points.size(); ++i )
         {
            for ( ui32 j = i; j < points.size(); ++j )
            {               
               mkernel( i, j ) = kernelBase( i, j ) - _sumA[ j ] - _sumA[ i ] + _sumC;
               mkernel( j, i ) = mkernel( i, j );
            }
         }
         kernelBase.unref();

         std::stringstream ss2;
         mkernel.print( ss2 );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "centred kernel=" );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss2.str() );

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
      bool _diagonalize( Matrix& centeredKernel, Matrix& outEigenVectors, ui32 nbFeatures, double minEigenValueToSelect )
      {
         const ui32 size = centeredKernel.sizex();

         // compute eigen values, eigen vectors
         Matrix eigenVectors;
         Vector eigenValues;
         bool res = core::svdcmp( centeredKernel, eigenValues, eigenVectors );

         {
            std::stringstream ss;
            eigenValues.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "kernel eigen values=" );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

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
         const ui32 initialNbFeatures = nbFeatures;
         for ( ui32 n = 0; n < initialNbFeatures; ++n )
            if ( pairs[ n ].first >= minEigenValueToSelect )
            {
               // do nothing, the feature is selected
            } else {
               --nbFeatures;
            }
         if ( nbFeatures == 0 )
            return false;

         // export the eigen vectors we are interested in
         outEigenVectors = Matrix( size, nbFeatures );
         for ( ui32 n = 0; n < nbFeatures; ++n )
         {
            const ui32 index = pairs[ n ].second;
            const double norm = sqrt( eigenValues[ index ] );
            for ( ui32 nn = 0; nn < size; ++nn )
               outEigenVectors( nn, n ) = eigenVectors( nn, index ) / norm;
         }

         {
            std::stringstream ss;
            outEigenVectors.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "selected kernel eigen vectors=" + core::val2str( nbFeatures )  );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         Vector sumA( static_cast<ui32>( _eig.sizey() ) );
         Vector sumB( static_cast<ui32>( _eig.sizey() ) );
         for ( ui32 k = 0; k < _eig.sizex(); ++k )
         {
            double sum = 0;
            double sumalpha = 0;
            for ( ui32 i = 0; i < _eig.sizey(); ++i )
            {
               sumalpha += _eig( i, k );
               sum += _eig( i, k ) * _sumA[ i ];
            }
            sumA[ k ] = sum;
            sumB[ k ] = sumalpha;
         }
         _sumA = sumA;
         _sumB = sumB;

         return true;
      }

   private:
      Matrix   _eig;                   /// eigen vectors, stored column (1 col = 1 eigen vector)
      std::auto_ptr<Kernel>  _kernel;  /// the kernel function  used by the algorithm
      std::vector<Point>     _points;  /// support vectors

      // variables needed to precompute the feature extraction
      Vector   _sumA;
      Vector   _sumB;
      double   _sumC;
   };
}
}

#endif
