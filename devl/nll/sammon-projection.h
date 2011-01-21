#ifndef NLL_ALGORITHM_SAMMON_PROJECTION_H_
# define NLL_ALGORITHM_SAMMON_PROJECTION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Sammon projection or Sammon mapping is an algorithm that maps a high-dimensional space to a space of lower dimensionality
           by trying to preserve the structure of inter-point distances in high-dimensional space in the lower-dimension projection.
           It is particularly suited for use in exploratory data analysis. The method was proposed by John W. Sammon in 1969.
           It is considered a non-linear approach as the projection cannot be represented as a linear combination of the original variables

    @see Sammon JW (1969). "A nonlinear mapping for data structure analysis
         http://theoval.cmp.uea.ac.uk/~gcc/matlab/sammon/sammon.pdf
    @note multithreaded implementation
    @todo use a symmetric matrix implementation to save half memory
    */
   template <class Points>
   class SammonProjection
   {
   public:
      typedef typename Points::value_type Point;

   public:
      /**
       @brief Project a high dimentional point on a typical 2D/3D space
       @note the outStress must be < 0.1 to be able to represent the data correctly (note that the algorithm is only approximating as it is not
       possible to find a transformation on a smaller space that keeps the same distance).
       */
      std::vector<Point> project( const Points& points, double learningRate = 0.3, ui32 maxIter = 1000, double epsilon = 1e-4, ui32 nbDimension = 2, double* outStress = 0 ) const
      {
         {
            std::stringstream ss;
            ss << "started Sammon's projection:" << std::endl
               << "number of points=" << points.size() << std::endl
               << "initial dimension=" << points[ 0 ].size() << " final=" << nbDimension << std::endl
               << "learning rate=" << learningRate << std::endl
               << "epsilon=" << epsilon;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // initialize the mapping with the PCA projections as initial guess
         PrincipalComponentAnalysis<Points> pca;
         bool res = pca.compute( points, nbDimension );
         if ( !res )
            throw std::runtime_error( "PCA failed to compute the initial points for SammonProjection" );
         std::vector<Point> projections;
         projections.reserve( points.size() );

         const int nbPoints = static_cast<ui32>( points.size() );
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

#ifndef NLL_NOT_MULTITHREADED
            #pragma omp parallel for reduction(+ : err)
#endif
            for ( int i = 0; i < nbPoints; ++i )
            {
               for ( int j = i; j < nbPoints; ++j )
               {
                  err += core::sqr( d( i, j ) - dproj( i, j ) ) / ( d( i, j ) + error );
               }
            }
            err /= c;

            {
               std::stringstream ss;
               ss << " stress=" << err;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }

            core::Buffer1D<double> dx( nbDimension );

            for ( int n = 0; n < nbPoints; ++n )
            {
               // compute gradient
               for ( ui32 k = 0; k < nbDimension; ++k )
               {
                  double sum = 0;
                  double sum2 = 0;

#ifndef NLL_NOT_MULTITHREADED
                  #pragma omp parallel for reduction(+ : sum) reduction(+ : sum2)
#endif
                  for ( int i = 0; i < nbPoints; ++i )
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
               for ( int k = 0; k < nbDimension; ++k )
               {
                  projections[ n ][ k ] -= dx[ k ] * learningRate;
               }
            }

            ++nbIter;
            if ( nbIter > maxIter )
            {
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, " maximum number of iterations reached" );
               break;
            }
         }

         if ( outStress )
         {
            // final stress
            *outStress = err;
         }

         {
            std::stringstream ss;
            ss << " final stress=" << err;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
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
            std::cout << "compute d" << std::endl;
            type sum = 0;
            const ui32 size = static_cast<ui32>( ps.size() );
            if ( _d.sizex() != size || _d.sizey() != size )
               _d = Matrix( size, size );
#ifndef NLL_NOT_MULTITHREADED
            #pragma omp parallel for reduction(+ : sum)
#endif
            for ( int i = 0; i < (int)size; ++i )
            {
               for ( ui32 j = i; j < size; ++j )
               {
                  type d = norm2( ps[ i ], ps[ j ] );
                  _d( i, j ) = d;
                  sum += d;
               }
            }

            std::cout << "end compute d" << std::endl;
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

#endif