#include <nll/nll.h>
#include <iostream>
#include <tester/register.h>

namespace nll
{
namespace algorithm
{
   template <class Points>
   class SammonProjection
   {
   public:
      typedef typename Points::value_type Point;

   public:
      std::vector<Point> project( const Points& points, double learningRate = 0.5, double epsilon = 1e-8, ui32 nbDimension = 2 ) const
      {
         // initialize the mapping with the PCA projections as initial guess
         PrincipalComponentAnalysis<Points> pca;
         bool res = pca.compute( points, nbDimension );
         if ( !res )
            throw std::runtime_error( "PCA failed to compute the initial points for SammonProjection" );
         std::vector<Point> projections;
         projections.reserve( points.size() );

         const ui32 nbPoints = static_cast<ui32>( points.size() );
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            projections.push_back( pca.process( points[ n ] ) );
         }
         
         // follow the gradient
         Distances d;         // input space
         Distances dproj;     // projected space

         double erro = 1;
         double err = 10;
         const double error = 1e-10;
         ui32 nbIter = 0;
         while ( fabs( err - erro ) > epsilon )
         {
            const double c = d.compute( points ) + error;
            dproj.compute( projections );

            erro = err;
            err = 0;
            for ( ui32 i = 0; i < nbPoints; ++i )
            {
               for ( ui32 j = i; j < nbPoints; ++j )
               {
                  err += core::sqr( d( i, j ) - dproj( i, j ) ) / ( d( i, j ) + error );
               }
            }
            err /= c;

            std::cout << "err=" << err << std::endl;

            core::Buffer1D<double> dx( nbDimension );
            for ( ui32 n = 0; n < nbPoints; ++n )
            {
               // compute gradient
               for ( ui32 k = 0; k < nbDimension; ++k )
               {
                  double sum = 0;
                  double sum2 = 0;
                  for ( ui32 i = 0; i < nbPoints; ++i )
                  {
                     if ( n == i )
                        continue;

                     const double di = d( n, i );
                     const double dpi = dproj( n, i );
                     const double diff = projections[ n ][ k ] - projections[ i ][ k ];
                     const double c1 = di * dpi + error;
                     const double c2 = di - dpi;
                     const double c3 = dpi + error;
                     sum  += c2 / c1 * diff;
                     sum2 += 1 / c1 * ( c2 - core::sqr( diff ) / c3 * ( 1 + c2 / c3 ) );
                  }
                  dx[ k ] = - sum / fabs( sum2 + error );
               }

               // update the projection
               for ( ui32 k = 0; k < nbDimension; ++k )
               {
                  projections[ n ][ k ] -= dx[ k ] * learningRate;
               }
            }

            ++nbIter;
         }

         return projections;
      }

      class Distances
      {
      public:
         typedef double type;
         typedef core::Matrix<type> Matrix;
         typedef typename Points::value_type Point;

      public:
         type compute( const Points& ps )
         {
            type sum = 0;
            const ui32 size = static_cast<ui32>( ps.size() );
            _d = Matrix( size, size );
            for ( ui32 i = 0; i < size; ++i )
            {
               for ( ui32 j = i; j < size; ++j )
               {
                  type d = norm2( ps[ i ], ps[ j ] );
                  _d( i, j ) = d;
                  sum += d;
               }
            }

            return sum;
         }

         static type norm2( const Point& p1, const Point& p2 )
         {
            type val = 0;
            ui32 size = static_cast<ui32>( p1.size() );
            for ( ui32 n = 0; n < size; ++n )
               val += core::sqr( p1[ n ] - p2[ n ] );
            return sqrt( val );
         }

         type operator()( ui32 i, ui32 j ) const
         {
            assert( i < d.sizey() && j < d.sizex() );

            if ( i > j )
               std::swap( i, j );
            return _d( i, j );
         }

         type& operator()( ui32 i, ui32 j )
         {
            assert( i < _d.sizey() && j < _d.sizex() );

            if ( i > j )
               std::swap( i, j );
            return _d( i, j );
         }

      private:
         Matrix _d;
      };
   };
}
}

struct TestSammonProjection
{
   void test()
   {
      srand( 0 );
      typedef std::vector<double>   Point;
      typedef std::vector<Point>    Points;

      Points points;
      for ( int n = 0; n < 4000; ++n )
         points.push_back( nll::core::make_vector<double>( nll::core::generateUniformDistribution( -5, 5 ),
                                                           nll::core::generateUniformDistribution( -5, 5 ),
                                                           nll::core::generateUniformDistribution( -5, 5 ) ) );

      nll::algorithm::SammonProjection<Points> projection;
      std::vector<Point> p = projection.project( points, 0.4 );

   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSammonProjection);
 TESTER_TEST(test);
TESTER_TEST_SUITE_END();
#endif
