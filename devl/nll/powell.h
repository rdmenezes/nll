#ifndef NLL_ALGORITHM_POWELL_H_
# define NLL_ALGORITHM_POWELL_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Powell optimizer. Particularly suited for optimizing problems with low dimensionality (<=10) and find
           solutions in narrow valley problems

    Point must define operator[], size(), Point(n)
    @param p the starting point
    @param xi the set of directions that can be used
    @param tol the tolerance
    */
   template <class Point, class Functor>
   double powell( Point& p, std::vector< core::Buffer1D<double> >& xi, double tol, const Functor& f, ui32 itMax = 200, bool* error = 0 )
   {
      if ( error )
         *error = false;
      const double tiny = 1e-20;
      double fret;
      const int n = static_cast<int>( p.size() );
      int i, ibig, j;
      double del, fp, fptt, t;
      ui32 iter;

      Point pt( n );
      Point ptt( n );
      core::Buffer1D<double> xit( n );

      fret = f( p );
      for ( j = 0; j< n; j++ )
         pt[ j ] = p[ j ];
      for ( iter = 0; ; ++iter )
      {
         fp = fret;
         ibig = 0;
         del = 0;
         for ( i = 0; i < n; i++ )
         {
            //In each iteration, loop over all directions in the set.
            for ( j = 0; j < n; j++ )
               xit[ j ] = xi[ j ][ i ]; // Copy the direction,
            fptt = fret;
            lineMinimization( p, xit, fret, f, tol );
            if ( fptt - fret > del )
            {
               //and record it if it is the largest decrease so far.
               del = fptt - fret;
               ibig = i;
            }
         }
         if ( 2.0 * ( fp - fret ) <= tol * ( fabs( fp ) + fabs( fret ) + tiny ) )
         {
            return fret;
         }
         
         if ( iter == itMax )
         {
            if ( error )
               *error = true;
            return 1e20;
         }

         for ( j = 0; j < n; j++ )
         {
            // Construct the extrapolated point and the average direction moved. Save the old starting point.
            ptt[ j ] = 2.0 * p[ j ] - pt[ j ];
            xit[ j ] = p[ j ] - pt[ j ];
            pt[ j ] = p[ j ];
         }
         fptt = f( ptt ); // Function value at extrapolated point.
         if ( fptt < fp )
         {
            t = 2.0 * ( fp - 2.0 * fret + fptt ) * core::sqr( fp - fret - del ) - del * core::sqr( fp - fptt );
            if ( t < 0.0 )
            {
               lineMinimization( p, xit, fret, f, tol );
               for ( j = 0; j < n; j++ )
               {
                  xi[ j ][ ibig ] = xi[ j ][ n - 1 ];
                  xi[ j ][ n - 1 ] = xit[ j ];
               }
            }
         }
      }
   }
}
}

#endif
