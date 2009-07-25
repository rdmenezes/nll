#include "stdafx.h"
#include <nll/nll.h>

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Defines a polynomial kernel function K(x, y)=(x.y)^degree

    Point must define double operator[](unsigned) const and size()
    */
   template <class Point>
   class KernelPolynomial
   {
   public:
      KernelPolynomial( double degree ) : _degree( degree )
      {
         ensure( degree >= 1, "kernel degree error" );
      }

      double operator()( const Point& p1, const Point& p2 ) const
      {
         assert( p1.size() == p2.size() );
         double sum = 0;
         for ( ui32 n = 0; n < p1.size(); ++n )
            sum += p1[ n ] * p2[ n ];
         return pow( sum, _degree );
      }

      KernelPolynomial* clone() const
      {
         return new KernelPolynomial( _degree );
      }

   private:
      double _degree;
   };

   template <class Point>
   class KernelRbf
   {
   public:
      KernelRbf( double var ) : _var( var )
      {
      }

      double operator()( const Point& p1, const Point& p2 ) const
      {
         assert( p1.size() == p2.size() );
         double sum = 0;
         for ( ui32 n = 0; n < p1.size(); ++n )
         {
            double val = p1[ n ] - p2[ n ];
            sum += val * val;
         }
         return exp( - sum / ( _var ) );
      }

      KernelRbf* clone() const
      {
         return new KernelRbf( _var );
      }

   private:
      double _var;
   };

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
         _centeredKernel = _computeKernelMatrix( points, kernel );
         bool diagonalization = _diagonalize( _eig, _eiv, _x, nbFeatures, points );
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
         ensure( getEigenVectors().sizex() && getEigenVectors().sizey(), "compute first the model parameters" );
         ensure( p.size() == _x.sizey(), "point size error" );
         ensure( _kernel, "something wrong happened..." );

         Vector projected( getEigenVectors().sizex() );
         Point x( p.size() );
         Point t( p.size() );
         for ( ui32 nn = 0; nn < p.size(); ++nn )
            t[ nn ] = p[ nn ];

         for ( ui32 k = 0; k < getEigenVectors().sizex(); ++k )
         {
            double sum = 0;
            for ( ui32 i = 0; i < getEigenVectors().sizex(); ++i )
            {
               for ( ui32 nn = 0; nn < p.size(); ++nn )
                  x[ nn ] = _x( nn, i );
               const double kk = (*_kernel)( x, t );
               sum += kk * getEigenVectors()( i, k );
            }
            projected[ k ] = sum;
         }
         return projected;
      }

      /**
       @return the centered kernel k_ij in the feature space. It makes sense to call this function
               only after <code>compute</code> method has been called.
       */
      const Matrix& getKernelMatrix() const
      {
         return _centeredKernel;
      }

      /**
       @return the eigen vectors in feature space
       */
      const Vector& getEigenValues() const
      {
         return _eiv;
      }

      /**
       @return the eigen vectors in feature space
       */
      const Matrix& getEigenVectors() const
      {
         return _eig;
      }

   private:
      /**
       @brief Compute the kernel matrix. It is a size(points) * size(points) matrix, centered in the feature
              space.
       */
      template <class Points>
      Matrix _computeKernelMatrix( const Points& points, const Kernel& kernel ) const
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
         Vector sumA( static_cast<ui32>( points.size() ) );
         Vector sumB( static_cast<ui32>( points.size() ) );
         double sumC = 0;
         for ( ui32 m = 0; m < points.size() * points.size(); ++m )
            sumC += kernelBase[ m ];
         for ( ui32 i = 0; i < points.size(); ++i )
         {
            for ( ui32 m = 0; m < points.size(); ++m )
                  sumA[ i ] += kernelBase( m, i );
            for ( ui32 m = 0; m < points.size(); ++m )
                  sumB[ i ] += kernelBase( i, m );
         }
         const double m1 = 1 / static_cast<double>( points.size() );

         // center the kernel
         Matrix mkernel( kernelBase.sizey(), kernelBase.sizex() );
         for ( ui32 i = 0; i < points.size(); ++i )
            for ( ui32 j = i; j < points.size(); ++j )
            {               
               mkernel( i, j ) = kernelBase( i, j ) - m1 * sumA[ j ] - m1 * sumB[ i ] + m1 * m1 * sumC;
               mkernel( j, i ) = mkernel( i, j );
            }

         mkernel.print( std::cout );
         kernelBase.unref();
         return mkernel;
      }

      /**
       @brief compute the eigen vectors & values of the kernel matrix. Eigen vectors are normalized so that
       ( V.V = 1 )

       we compute eiv, eig of:
       M * eiv * eig = Kernel * eig
       */
      template <class Points>
      bool _diagonalize( Matrix& outEigenVectors, Vector& outEigenValues, Matrix& outVectors, ui32 nbFeatures, const Points& points  )
      {
         const ui32 size = _centeredKernel.sizex();
         const ui32 inputPointSize = static_cast<ui32>( points[ 0 ].size() );

         // compute eigen values, eigen vectors
         Matrix copyCov;
         Matrix eigenVectors;
         Vector eigenValues;
         copyCov.clone( _centeredKernel ); // we need to fully copy the kernel matrix as it is directly replaced in SVD decomposition
         bool res = core::svdcmp( copyCov, eigenValues, eigenVectors );
         copyCov.unref();

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
         outEigenValues = Vector( nbFeatures );
         outVectors = Matrix( inputPointSize, nbFeatures );

         for ( ui32 n = 0; n < nbFeatures; ++n )
         {
            const ui32 index = pairs[ n ].second;
            const double norm = sqrt( eigenValues[ index ] );
            for ( ui32 nn = 0; nn < size; ++nn )
               outEigenVectors( nn, n ) = eigenVectors( nn, index ) / norm;

            for ( ui32 nn = 0; nn < inputPointSize; ++nn )
               outVectors( nn, n ) = points[ index ][ nn ];

            outEigenValues[ n ] = eigenValues[ index ];
         }
         outVectors.print( std::cout );
         outEigenValues.print( std::cout );
         outEigenVectors.print( std::cout );
         return true;
      }

   private:
      Matrix   _centeredKernel;  /// stores the centered kernel matrix K_ij in feature space
      Matrix   _eig;             /// eigen vectors, stored column (1 col = 1 eigen vector)
      Matrix   _x;               /// the vectors associated with the principal component, 1 col = 1 vector
      Vector   _eiv;             /// eigen values
      Kernel*  _kernel;          /// the kernel used by the algorithm
   };
}
}

class TestKernelPca
{
public:
   /**
    Compute a dummy kpca. Check the kernel matrix is correctly centered in feature space.
    We want sum H(x_k) = 0
    => sum H(x_k)H(x_i) = 0, for i=0..size
    => sum kernel(x_k, x_i) = 0 for i=0..size

    //Test if kernel matrix is centered in feature space
    Eigen vectors in feature space are normalized
    */
   void testCentered()
   {
      typedef std::vector<double>               Point;
      typedef std::vector<Point>                Points;
      //typedef nll::algorithm::KernelPolynomial<Point>   Kernel;
      typedef nll::algorithm::KernelRbf<Point>   Kernel;
      typedef nll::algorithm::KernelPca<Point, Kernel>  KernelPca;

      // we want to keep the values small as we are using a polynomial kernel of degree 10
      Points points;
      points.push_back( nll::core::make_vector<double>( 2, 1.1 ) );
      points.push_back( nll::core::make_vector<double>( 1, 1 ) );
      points.push_back( nll::core::make_vector<double>( -1, 0.9 ) );
      points.push_back( nll::core::make_vector<double>( 1.5, 1.2 ) );
      //points.push_back( nll::core::make_vector<double>( 1.8, 1.01 ) );
      //points.push_back( nll::core::make_vector<double>( 2, 1.2 ) );

      KernelPca kpca;
      Kernel rbfKernel( 0.1 );
      kpca.compute( points, 3, rbfKernel );

      // test kernel is centered
      nll::core::Matrix<double> kernel = kpca.getKernelMatrix();
      for ( unsigned n = 0; n < points.size(); ++n )
      {
         double sum = 0;
         for ( unsigned nn = 0; nn < points.size(); ++nn )
            sum += kernel( n, nn );
         std::cout << "sum=" << sum << std::endl;
         TESTER_ASSERT( nll::core::equal<double>( sum, 0, 1e-5 ) );
      }

      // test orthogonal eig
      for ( unsigned nb = 0; nb < kpca.getEigenValues().size(); ++nb )
      {
         for ( unsigned nb2 = nb + 1; nb2 < kpca.getEigenValues().size(); ++nb2 )
         {
            Point p( kpca.getEigenVectors().sizey() );
            for ( unsigned n = 0; n < kpca.getEigenVectors().sizey(); ++n )
            {
               p[ n ] = kpca.getEigenVectors()( n, nb );
            }

            Point p2( kpca.getEigenVectors().sizey() );
            for ( unsigned n = 0; n < kpca.getEigenVectors().sizey(); ++n )
            {
               p2[ n ] = kpca.getEigenVectors()( n, nb2 );
            }
            TESTER_ASSERT( nll::core::equal<double>( rbfKernel( p, p2 ), 0, 1e-5 ) );
         }
      }

      nll::core::Buffer1D<float> point = nll::core::make_buffer1D<float>( 1.5f, 0.9f );
      KernelPca::Vector v = kpca.transform( point );
      v.print( std::cout );
   }

   void test2()
   {
      nll::core::Matrix<double> points(3, 3);
      points( 0, 0 ) = 4;
      points( 0, 2 ) = 10;
      points( 0, 1 ) = 1;

      points( 1, 0 ) = 8;
      points( 1, 2 ) = 10.1;
      points( 1, 1 ) = 1;

      points( 2, 0 ) = 6;
      points( 2, 2 ) = 9.9;
      points( 2, 1 ) = 1;
      nll::core::Matrix<double> cov = nll::core::covariance( points, 0, 2 );

      nll::core::Buffer1D<double> eiv;
      nll::core::Matrix<double> eig;

      nll::core::svdcmp( cov, eiv, eig );
      eig.print( std::cout );
      eiv.print( std::cout );
   }
};

//#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestKernelPca);
TESTER_TEST(testCentered);
//TESTER_TEST(test2);
TESTER_TEST_SUITE_END();
//#endif
