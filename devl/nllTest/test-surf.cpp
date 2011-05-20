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
            for ( ui32 y = 0; y < resy; ++y )
            {
               for ( ui32 x = 0; x < resx; ++x )
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

      // computes the index in mapDest the closest from (xRef, yRef, mapDest)
      void indexInMap( ui32 xRef, ui32 yRef, ui32 mapRef, ui32 mapDest, int& outx, int& outy ) const
      {
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
         if ( x < 1 || y < 1 || x + 1 >= m.sizex() || y + 1 >= m.sizey() )
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
         Matrix      features;
         value_type  orientation;
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
         Points points;
         ui32 nbPoints = 0;

         FastHessianDetPyramid2D pyramid;
         pyramid.construct( i, _filterSizes, _filterSteps );

         for ( ui32 filter = 1; filter < _filterSizes.size() - 1 ; ++filter )
         {
            const Matrix& f = pyramid.getPyramidDetHessian()[ filter ];
            for ( ui32 y = 0; y < f.sizey(); ++y )
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
                        ++nbPoints;
                        
                        const int half = _filterSizes[ filter ] / 2;
                        const int px = x * _filterSteps[ filter ] + half;
                        const int py = y * _filterSteps[ filter ] + half;
                        core::bresham( output, core::vector2i( px + 5, py ), core::vector2i( px - 5, py ), core::vector3uc(255, 0, 0) );
                        core::bresham( output, core::vector2i( px, py - 5 ), core::vector2i( px, py + 5 ), core::vector3uc(255, 0, 0) );
                        core::bresham( output, core::vector2i( px, py ), core::vector2i( px + half, py), core::vector3uc(255, 0, 0) );
                        //return points;
                     }
                  }
               }
            }
         }

         std::cout << "nbpointsSelected=" << nbPoints << std::endl;
         return points;
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
