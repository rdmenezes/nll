#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace algorithm
{
   /**
    @brief Hold a stack of the hessian determinant using a crude approximation of a gaussian in 2D
           for each point, H(x, o) =| Lxx Lxy |
                                    | Lxy Lyy |
                           det(H)= Lxx*Lyy-(0.9 * Lxy)^2
           note the normalization factor 0.9 comes from the gaussian approximation
    */
   class FastHessianDetPyramid2D
   {
   public:
      typedef double                   value_type;
      typedef core::Matrix<value_type> Matrix;

   public:
      /**
       @brief Construct and computes the hessian determinant pyramid
       @param i the image,
       @param scales the size of each level of the pyramid (in pixel), must be in increasing order
       @param displacements the step between two filter evaluation for this particular level
       */
      template <class T, class Mapper, class Alloc>
      void construct( const core::Image<T, Mapper, Alloc>& i, const std::vector<ui32>& scales, const std::vector<ui32>& displacements )
      {
         ensure( displacements.size() == scales.size(), "must be the same size" );

         _pyramidDetHessian.clear();
         _pyramidLaplacian.clear();
         _scales = scales;
         _displacements = displacements;

         const T max = (T)std::max( abs( *std::max_element( i.begin(), i.end() ) ),
                                    abs( *std::min_element( i.begin(), i.end() ) ) );

         // construct an integral image
         IntegralImage image;
         image.process( i );

          
         // build each level by redimensioning the approximated gaussian derivatives
         ui32 lastScale = 0;

         for ( size_t n = 0; n < scales.size(); ++n )
         {
            ensure( scales[ n ] % 2 == 1, "scales must be odd numbers" );
            ensure( scales[ n ] >= 9, "minimal size" );
            //ensure( lastScale < scales[ n ], "scales must be in increasing order" );

            lastScale = scales[ n ];
            const ui32 step = displacements[ n ];

            const ui32 sizeFilterx = scales[ n ];
            const ui32 sizeFiltery = scales[ n ];
            const double sizeFilter = sizeFilterx * sizeFiltery * max; // we normalize by the filter size and maximum value

            const ui32 halfx = sizeFilterx / 2;
            const ui32 halfy = sizeFilterx / 2;

            // the total size must take into account the step size and filter size (it must be fully inside the image to be computed)
            const ui32 resx = ( i.sizex() - 2 * halfx + 1 ) / step;
            const ui32 resy = ( i.sizey() - 2 * halfy + 1 ) / step;

            Matrix detHessian( resy, resx );
            Matrix laplacian( resy, resx );

            if ( !resx || !resy )
               break;   // the scale is too big!

            // compute the hessian
            #pragma omp parallel for
            for ( int y = 0; y < resy; ++y )
            {
               for ( int x = 0; x < resx; ++x )
               {
                  core::vector2ui bl( x * step, y * step );
                  core::vector2ui tr( bl[ 0 ] + sizeFilterx - 1, bl[ 1 ] + sizeFiltery - 1 );
                  const double dxx = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::VERTICAL_TRIPLE,
                                                                        image,
                                                                        bl,
                                                                        tr ) / sizeFilter;
                  const double dyy = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::HORIZONTAL_TRIPLE,
                                                                        image,
                                                                        bl,
                                                                        tr ) / sizeFilter;
                  const double dxy = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::CHECKER,
                                                                        image,
                                                                        bl,
                                                                        tr ) / sizeFilter;
                  detHessian( y, x ) = dxx * dyy - core::sqr( 0.9 * dxy );
                  laplacian( y, x ) = dxx + dyy;
               }
            }

            _pyramidDetHessian.push_back( detHessian );
            _pyramidLaplacian.push_back( laplacian );
         }
      }

      /**
       @brief Computes the gradient of the hessian at position (x, y, map)
              using finite difference
       */
      core::Buffer1D<value_type> getHessianGradient( ui32 x, ui32 y, ui32 map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].sizex() - 1 &&
                 y < _pyramidDetHessian[ map ].sizey() - 1 );

         int xminus, yminus;
         indexInMap( x, y, map, map - 1, xminus, yminus );  // we need to look up the closed index in a map that has different dimensions

         int xplus, yplus;
         indexInMap( x, y, map, map + 1, xplus, yplus ); // we need to look up the closed index in a map that has different dimensions

         const Matrix& current = _pyramidDetHessian[ map ];
         core::Buffer1D<value_type> grad( 3 );
         grad[ 0 ] = ( current( y, x + 1 ) - current( y, x - 1 ) ) / 2;
         grad[ 1 ] = ( current( y + 1, x ) - current( y - 1, x ) ) / 2;
         grad[ 2 ] = ( _pyramidDetHessian[ map + 1 ]( yplus, xplus ) -
                       _pyramidDetHessian[ map - 1 ]( yminus, xminus ) ) / 2;
         return grad;
      }

      /**
       @brief Computes the hessian of the hessian at position (x, y, map)
              using finite difference
       */
      Matrix getHessianHessian( ui32 x, ui32 y, ui32 map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].sizex() &&
                 y < _pyramidDetHessian[ map ].sizey() );

         const Matrix& mc = _pyramidDetHessian[ map ];
         const Matrix& mm = _pyramidDetHessian[ map - 1 ];
         const Matrix& mp = _pyramidDetHessian[ map + 1 ];
         const double val = mc( y, x );

         int xm, ym;
         indexInMap( x, y, map, map - 1, xm, ym );  // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xm > 0 && ym > 0 &&
                 xm < (int)_pyramidDetHessian[ map - 1 ].sizex() - 1 &&
                 ym < (int)_pyramidDetHessian[ map - 1 ].sizey() - 1 );

         int xp, yp;
         indexInMap( x, y, map, map + 1, xp, yp ); // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xp > 0 && ym > 0 &&
                 xp < (int)_pyramidDetHessian[ map + 1 ].sizex() - 1 &&
                 yp < (int)_pyramidDetHessian[ map + 1 ].sizey() - 1 );

         const double dxx = mc( y, x + 1 ) + mc( y, x - 1 ) - 2 * val;
         const double dyy = mc( y + 1, x ) + mc( y - 1, x ) - 2 * val;
         const double dss = mp( yp, xp )   + mm( ym, xm )   - 2 * val;
         const double dxy = ( mc( y + 1, x + 1 ) + mc( y - 1, x - 1 ) -
                              mc( y - 1, x + 1 ) - mc( y + 1, x - 1 ) ) / 4;
         const double dxs = ( mp( yp, xp + 1 ) + mm( ym, xm - 1 ) -
                              mm( ym, xm + 1 ) - mp( yp, xp - 1 ) ) / 4;
         const double dys = ( mp( yp + 1, xp ) + mm( ym - 1, xm ) -
                              mm( ym + 1, xm ) - mp( yp - 1, xp ) ) / 4;

         Matrix hs( 3, 3 );
         hs( 0, 0 ) = dxx;
         hs( 0, 1 ) = dxy;
         hs( 0, 2 ) = dxs;

         hs( 1, 0 ) = dxy;
         hs( 1, 1 ) = dyy;
         hs( 1, 2 ) = dys;

         hs( 2, 0 ) = dxs;
         hs( 2, 1 ) = dys;
         hs( 2, 2 ) = dss;

         return hs;
      }

      // computes the index in mapDest the closest from (xRef, yRef, mapDest)
      void indexInMap( ui32 xRef, ui32 yRef, ui32 mapRef, ui32 mapDest, int& outx, int& outy ) const
      {
         /*
         // CHECK robustness
         outx = xRef;
         outy = yRef;
         return;
         */

         if ( mapRef == mapDest )
         {
            outx = xRef;
            outy = yRef;
         } else {
            // map a point at a given scale to the image space
            const int half = _scales[ mapRef ] / 2;
            const int x = xRef * _displacements[ mapRef ] + half;
            const int y = yRef * _displacements[ mapRef ] + half;

            // convert the image space coordinate to the other scale space
            const int halfd = _scales[ mapDest ] / 2;
            outx = ( x - halfd ) / (int)_displacements[ mapDest ];
            outy = ( y - halfd ) / (int)_displacements[ mapDest ];
         }
      }

      /**
       @brief returns true if all value around the projection (xRef, yRef, mapRef) on mapDest are smaller
       */
      bool isDetHessianMax( double val, ui32 xRef, ui32 yRef, ui32 mapRef, ui32 mapDest ) const
      {
         int x, y;

         // if it is outside, then skip it
         indexInMap( xRef, yRef, mapRef, mapDest, x, y );
         const Matrix& m = _pyramidDetHessian[ mapDest ];
         if ( x < 1 || y < 1 || x + 1 >= (int)m.sizex() || y + 1 >= (int)m.sizey() )
            return false;

         return val >= m( y + 0, x + 0 ) &&
                val >= m( y + 1, x + 0 ) &&
                val >= m( y - 1, x + 0 ) &&
                val >= m( y + 0, x + 1 ) &&
                val >= m( y + 1, x + 1 ) &&
                val >= m( y - 1, x + 1 ) &&
                val >= m( y + 0, x - 1 ) &&
                val >= m( y + 1, x - 1 ) &&
                val >= m( y - 1, x - 1 );
      }

      const std::vector<Matrix>& getPyramidDetHessian() const
      {
         return _pyramidDetHessian;
      }

      const std::vector<Matrix>& getPyramidLaplacianSign() const
      {
         return _pyramidLaplacian;
      }

   private:
      std::vector<Matrix>  _pyramidDetHessian;
      std::vector<Matrix>  _pyramidLaplacian;
      std::vector<ui32>    _scales;
      std::vector<ui32>    _displacements;

   };

   /**
    @brief Implementation of the Speeded Up Robust Features or SURF algorithm

    @see http://sites.google.com/site/chrisevansdev/files/opensurf.pdf for good notes on the algorithm
         http://www.vision.ee.ethz.ch/~surf/eccv06.pdf reference paper
    */
   class SpeededUpRobustFeatures
   {
      typedef double                   value_type;
      typedef core::Matrix<value_type> Matrix;

   public:
      struct Point
      {
         Point( core::vector2ui p, ui32 s ) : position( p ), scale( s )
         {}

         Matrix            features;
         value_type        orientation;
         core::vector2ui   position;
         ui32              scale;
      };

      typedef core::Buffer1D<Point> Points;

      /**
       @brief Construct SURF
       @param octaves the number of octaves to analyse. This increases the range of filters exponentially
       @param intervals the number of intervals per octave This increase the filter linearly
       @param threshold the minimal threshold of the hessian. The lower, the more features (but less robust) will be detected
       */
      SpeededUpRobustFeatures( ui32 octaves = 5, ui32 intervals = 4, ui32 init_step = 2, value_type threshold = 0.00012 ) : _threshold( threshold )
      {
         ui32 step = init_step;
         for ( ui32 o = 1; o <= octaves; ++o )
         {
            for ( ui32 i = 1; i <= intervals; ++i )
            {
               const ui32 filterSize = core::round( 3 * ( std::pow( 2.0, (int)o ) * i + 1 ) );
               
               if ( _filterSizes.size() == 0 || *_filterSizes.rbegin() < filterSize )
               {
                  _filterSizes.push_back( filterSize );
                  _filterSteps.push_back( step );
               }
            }
            step *= 2;
         }
      }

      template <class T, class Mapper, class Alloc>
      Points computesFeatures( const core::Image<T, Mapper, Alloc>& i, core::Image<T, Mapper, Alloc>& output )
      {
         Points p;
         ui32 nbPoints = 0;

         // each thread can work independently, so allocate an array og points that are not shared
         // between threads
         const ui32 maxNumberOfThread = omp_get_max_threads();
         std::vector< std::vector<Point> > points( maxNumberOfThread );

         std::cout << "max thread=" << omp_get_max_threads() << std::endl;

         FastHessianDetPyramid2D pyramid;
         pyramid.construct( i, _filterSizes, _filterSteps );

         for ( ui32 filter = 1; filter < _filterSizes.size() - 1 ; ++filter )
         {
            const Matrix& f = pyramid.getPyramidDetHessian()[ filter ];
            #pragma omp parallel for reduction(+ : nbPoints)
            for ( int y = 0; y < f.sizey(); ++y )
            {
               for ( ui32 x = 0; x < f.sizex(); ++x )
               {
                  const double val = f( y, x );
                  if ( val > _threshold )
                  {
                     bool isMax = pyramid.isDetHessianMax( val, x, y, filter, filter )     &&
                                  pyramid.isDetHessianMax( val, x, y, filter, filter + 1 ) &&
                                  pyramid.isDetHessianMax( val, x, y, filter, filter - 1 );
                     if ( isMax )
                     {
                        core::Buffer1D<value_type> hessianGradient = pyramid.getHessianGradient( x, y, filter );
                        Matrix hessianHessian = pyramid.getHessianHessian( x, y, filter );
                        const bool inverted = core::inverse3x3( hessianHessian );
                        Matrix interpolatedPoint = hessianHessian * Matrix( hessianGradient, 3, 1 );
                        //std::cout << interpolatedPoint[ 0 ] << " " << interpolatedPoint[ 1 ] << std::endl;

                        if ( inverted && interpolatedPoint[ 0 ] < 0.5 &&
                                         interpolatedPoint[ 1 ] < 0.5 &&
                                         interpolatedPoint[ 2 ] < 0.5 )
                        {
                           const int size = _filterSizes[ filter ];
                           const int half = size / 2;

                           int px    = ( x    - interpolatedPoint[ 0 ] ) * _filterSteps[ filter ] + half;
                           int py    = ( y    - interpolatedPoint[ 1 ] ) * _filterSteps[ filter ] + half;                           
                           int scale = size   - interpolatedPoint[ 2 ]   * _filterSteps[ filter ];

                           ui32 threadId = omp_get_thread_num();
                           points[ threadId ].push_back( Point( core::vector2ui( px, py ), scale ) );
                           ++nbPoints;

                           if ( px > 5 && py > 5 && px + half < output.sizex() - 1 && py + 5 < output.sizey() - 1 )
                           {
                              core::bresham( output, core::vector2i( px + 5, py ), core::vector2i( px - 5, py ), core::vector3uc(255, 0, 0) );
                              core::bresham( output, core::vector2i( px, py - 5 ), core::vector2i( px, py + 5 ), core::vector3uc(255, 0, 0) );
                              core::bresham( output, core::vector2i( px, py ), core::vector2i( px + scale / 2, py), core::vector3uc(255, 0, 0) );
                           }
                           
                           //return points;
                        }
                     }
                  }
               }
            }
         }

         std::cout << "nbpointsSelected=" << nbPoints << std::endl;
         return p;
      }

   private:
      std::vector<ui32> _filterSizes;
      std::vector<ui32> _filterSteps;
      value_type _threshold;
   };
}
}

class TestSurf
{
public:
   void testBasic()
   {
      core::Image<ui8> image( NLL_TEST_PATH "data/feature/sf.bmp" );
      core::Image<ui8> cpy;
      cpy.clone( image );

      TESTER_ASSERT( image.sizex() );
      core::decolor( image );

//      core::extend( image, 3 );
//      core::writeBmp( image, NLL_TEST_PATH "data/feature/sf2.bmp" );

      std::cout << "start computatio=" << std::endl;
      //algorithm::SpeededUpRobustFeatures surf( 5, 4, 2, 0.005 );
      algorithm::SpeededUpRobustFeatures surf( 5, 4, 2, 0.0005 );

      nll::core::Timer timer;
      algorithm::SpeededUpRobustFeatures::Points points = surf.computesFeatures( image, cpy );
      std::cout << "done=" << timer.getCurrentTime() << std::endl;

      core::writeBmp( cpy, "c:/tmp/o.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSurf);
TESTER_TEST(testBasic);
TESTER_TEST_SUITE_END();
#endif
