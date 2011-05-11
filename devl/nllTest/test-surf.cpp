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
       */
      template <class T>
      FastHessianDetPyramid2D( const core::Image<T>& i, const std::vector<ui32>& scales )
      {
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

            const ui32 sizeFilterx = scales[ n ];
            const ui32 sizeFiltery = scales[ n ];
            const double sizeFilter = sizeFilterx * sizeFiltery;
            const ui32 sizex = i.sizex() / scales[ n ];
            const ui32 sizey = i.sizey() / scales[ n ];

            if ( !sizex || !sizey )
               break;   // the scale is too big!

            // compute the hessian
            const ui32 halfx = scales[ n ] / 2;
            const ui32 halfy = scales[ n ] / 2;

            for ( ui32 y = 0; y < sizey; ++y )
            {
               for ( ui32 x = 0; x < sizex; ++x )
               {
                  core::vector2ui bl( x * sizeFilterx, y * sizeFiltery );
                  core::vector2ui tr( ( x + 1 ) * sizeFilterx - 1, ( y + 1 ) * sizeFiltery - 1 );
                  ui32 midx = ( bl[ 0 ] + tr[ 0 ] ) / 2;
                  ui32 midy = ( bl[ 1 ] + tr[ 1 ] ) / 2;
                  
                  //double xx = ( image.getSum( bl, tr ) / sizeFilter;
               }
            }
         }
      }

   private:
      std::vector<Matrix>  _pyramid;
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

      /**
       @brief Construct SURF
       @param octaves the number of octaves to analyse. This increases the range of filters exponentially
       @param intervals the number of intervals per octave This increase the filter linearly
       @param threshold the minimal threshold of the hessian. The lower, the more features (but less robust) will be detected
       */
      SpeededUpRobustFeatures( ui32 octaves = 5, ui32 interfals = 4, value_type threshold = 0.1 )
      {
      }

   private:
      std::vector<ui32> _filterSizes;
   };
}
}

class TestSurf
{
public:
   void testBasic()
   {
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSurf);
TESTER_TEST(testBasic);
TESTER_TEST_SUITE_END();
#endif
