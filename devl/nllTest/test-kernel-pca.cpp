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

   private:
      double _degree;
   };

   /**
    @ingroup algorithm
    @brief Nonlinear Component Analysis as a Kernel Eigenvalue Problem. This is an extension of the PCA algorithm
           in a high dimentionality feaure space.

    It is a direct implementaion of the article from Bernhard Scholkopf, Alexander Smola and Klaus Robert Muller
    on kernel component analysis. See here for the details:http://www.face-rec.org/algorithms/Kernel/kernelPCA_scholkopf.pdf
    */
   template <class Point>
   class KernelPca
   {
      typedef core::Matrix<double>     Matrix;
      typedef core::Buffer1D<double>   Vector;

   public:
      /**
       @brief Compues the <code>nbFeatures</code> most important features in a high dimentional space defined
              by the kernel function. Memory consuption is 2 * point.size^2. If we don't have enough memory,
              just take a representative sample of the points.
       @param points the set of points. needs to define Point operator[](n) const and size()
       @param nbFeatures the number of features used for the feaure space. It could range from 1 to the number
              of points.
       @param kernel A kernel function. It must define the double operator(Point, Point) const
       @return false if any error occured
       */
      template <class Points, class Kernel>
      bool compute( const Points& points, ui32 nbFeatures, const Kernel& kernel )
      {
         ensure( points.size(), "no point to compute" );
         ensure( nbFeatures <= points.size() && nbFeatures > 0, "error in the number of features to be selected" );

         _centeredKernel = _computeKernelMatrix( points, kernel );
         Matrix eig;
         Vector eiv;
         bool diagonalization = _diagonalize( eig, eiv );
         if ( !diagonalization )
            return false;

         return true;
      }

      /**
       @return the centered kernel k_ij in the feature space. It makes sense to call this function
               only after <code>compute</code> method has been called.
       */
      const Matrix& getKernelMatrix() const
      {
         return _centeredKernel;
      }

   private:
      /**
       @brief Compute the kernel matrix. It is a size(points) * size(points) matrix, centered in the feature
              space.
       */
      template <class Points, class Kernel>
      Matrix _computeKernelMatrix( const Points& points, const Kernel& kernel ) const
      {
         Matrix kernelBase( static_cast<ui32>( points.size() ), static_cast<ui32>( points.size() ) );
         for ( ui32 i = 0; i < points.size(); ++i )
            for ( ui32 j = i; j < points.size(); ++j )
            {
               kernelBase( i, j ) = kernel( points[ i ], points[ j ] );
               kernelBase( j, i ) = kernelBase( i, j );
            }

         const double m1 = 1 / static_cast<double>( points.size() );
         Matrix mkernel( kernelBase.sizey(), kernelBase.sizex() );
         for ( ui32 i = 0; i < points.size(); ++i )
            for ( ui32 j = i; j < points.size(); ++j )
            {
               double sumA = 0;
               for ( ui32 m = 0; m < points.size(); ++m )
                  sumA += kernelBase( m, j );
               double sumB = 0;
               for ( ui32 m = 0; m < points.size(); ++m )
                  sumB += kernelBase( i, m );
               double sumC = 0;
               for ( ui32 m = 0; m < points.size() * points.size(); ++m )
                  sumC += kernelBase[ m ];
               mkernel( i, j ) = kernelBase( i, j ) - m1 * sumA - m1 * sumB + m1 * m1 * sumC;
               mkernel( j, i ) = mkernel( i, j );
            }

         mkernel.print( std::cout );
         kernelBase.unref();
         return mkernel;
      }

      /**
       compute the eigen vectors & values of the kernel matrix. Eigen vectors are normalized
       so that V.V = 1 
       
       1 = sum alpha_i * alpha_j H(v).H(v) = sum alpha_i * alpha_j * k_ij
         = lambda * (alpha . alpha)
       ||alpha|| = 1 / lambda, with alpha a eigen vector
       */
      bool _diagonalize( Matrix& outEigenVectors, Vector& outEigenValues )
      {

         Matrix copy;
         copy.clone( _centeredKernel );
         bool res = core::svdcmp( copy, outEigenValues, outEigenVectors );
         outEigenVectors.print( std::cout );

         //for ( ui32 n = 0; n < outEigenVectors.
         return res;
      }

   private:
      Matrix   _centeredKernel;  /// stores the centered kernel matrix K_ij in feature space
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
    */
   void testCentered()
   {
      typedef std::vector<double>               Point;
      typedef std::vector<Point>                Points;
      typedef nll::algorithm::KernelPca<Point>  KernelPca;

      Points points;
      points.push_back( nll::core::make_vector<double>( 1, 1 ) );
      points.push_back( nll::core::make_vector<double>( 2, 1.1 ) );
      points.push_back( nll::core::make_vector<double>( 3, 0.9 ) );
      points.push_back( nll::core::make_vector<double>( 30, 1.2 ) );

      KernelPca kpca;
      nll::algorithm::KernelPolynomial<Point> polynomialKernel( 1 );
      kpca.compute( points, 2, polynomialKernel );

      nll::core::Matrix<double> kernel = kpca.getKernelMatrix();
      for ( unsigned n = 0; n < points.size(); ++n )
      {
         double sum = 0;
         for ( unsigned nn = 0; nn < points.size(); ++nn )
            sum += kernel( n, nn );
         TESTER_ASSERT( nll::core::equal<double>( sum, 0, 1e-5 ) );
      }
   }
};

//#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestKernelPca);
TESTER_TEST(testCentered);
TESTER_TEST_SUITE_END();
//#endif
