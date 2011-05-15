#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace algorithm
{
   /**
    @TODO - check the haar approximation of the gaussian

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
            ensure( lastScale < scales[ n ], "scales must be in increasing order" );

            lastScale = scales[ n ];
            const ui32 step = displacements[ n ];

            const ui32 sizeFilterx = scales[ n ];
            const ui32 sizeFiltery = scales[ n ];
            const double sizeFilter = sizeFilterx * sizeFiltery * max; // we normalize by the filter size and maximum value

            const ui32 resx = i.sizex() / step;
            const ui32 resy = i.sizey() / step;

            Matrix detHessian( resy, resx, false );
            Matrix laplacian( resy, resx, false );

            if ( !resx || !resy )
               break;   // the scale is too big!

            // compute the hessian
            for ( ui32 y = 0; y < resy; ++y )
            {
               for ( ui32 x = 0; x < resx; ++x )
               {
                  core::vector2ui bl( x, y );
                  core::vector2ui tr( x + sizeFilterx - 1, y + sizeFiltery - 1 );

                  HaarFeatures2d features;
                  features.add( HaarFeatures2d::Feature( HaarFeatures2d::Feature::VERTICAL_TRIPLE, image, bl, tr ) );
                  features.add( HaarFeatures2d::Feature( HaarFeatures2d::Feature::HORIZONTAL_TRIPLE, image, bl, tr ) );
                  features.add( HaarFeatures2d::Feature( HaarFeatures2d::Feature::CHECKER, image, bl, tr ) );
                  HaarFeatures2d::Buffer f = features.process( image );

                  // normalize the features
                  f[ 0 ] /= sizeFilter;
                  f[ 1 ] /= sizeFilter;
                  f[ 2 ] /= sizeFilter;

                  // compute the hessian's determinantt & laplacian
                  detHessian( y, x ) = f[ 1 ] * f[ 0 ] - core::sqr( 0.9 * f[ 2 ] );
                  laplacian( y, x ) = f[ 0 ] + f[ 1 ];
               }
            }

            _pyramidDetHessian.push_back( detHessian );
            _pyramidLaplacian.push_back( laplacian );
         }
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
      SpeededUpRobustFeatures( ui32 octaves = 5, ui32 intervals = 4, ui32 init_step = 2, value_type threshold = 0.1 ) : _threshold( threshold )
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
      Points computesFeatures( const core::Image<T, Mapper, Alloc>& i )
      {
         Points points;

         FastHessianDetPyramid2D pyramid;
         pyramid.construct( i, _filterSizes, _filterSteps );

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
      TESTER_ASSERT( image.sizex() );
      core::decolor( image );

//      core::extend( image, 3 );
//      core::writeBmp( image, NLL_TEST_PATH "data/feature/sf2.bmp" );

      algorithm::SpeededUpRobustFeatures surf; //( 3, 3, 4 );

      nll::core::Timer timer;
      algorithm::SpeededUpRobustFeatures::Points points = surf.computesFeatures( image );
      std::cout << "done=" << timer.getCurrentTime() << std::endl;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSurf);
TESTER_TEST(testBasic);
TESTER_TEST_SUITE_END();
#endif
