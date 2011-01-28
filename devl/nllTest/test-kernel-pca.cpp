#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;

namespace
{
   double test_data[][ 2 ] =
   {
      { 0.0699,    0.0389 },
      {-0.0007,   -0.0108 },
      {-0.2154,    0.0546 },
      { 0.2194,    0.0261 },
      {-0.1648,    0.0233 },
      { 0.1852,   -0.0516 }
   };
}

namespace nll
{
namespace algortihm
{
   /**
    @brief Generic pre-image implementing the Multi-Dimensional Scaling (MDS) by James T. Kwok and Ivor W. Tsang
           in "The Pre-Image Problem in Kernel Methods", 2003
    @note this is typically used for denoising problems: we project the noisy example on the feature space and try 
          to reconstruct a denoised example using a trained KernelPca

    @brief for computation see http://opus.kobv.de/tuberlin/volltexte/2006/1256/pdf/bakir_goekhan.pdf
                           and http://www.hpl.hp.com/conferences/icml2003/papers/345.pdf
    */
   template <class Point, class Kernel>
   class KernelPreImageMDS
   {
   public:
      KernelPreImageMDS()
      {
         STATIC_ASSERT( 0 ); // this operation is not yet implemented for this kernel. Note that
         // only for a restricted subset of kernels, a pre-image can be computed
      }
   };

   /**
    @brief Given a point in feature space z, we are looking for the closest preimage x so that x = arg min_x ||theta(x) - z ||
           
    @note that this is only valid for a gaussian kernel!
    */
   template <class Point>
   class KernelPreImageMDS< Point, KernelRbf<Point> >
   {
   public:
      typedef KernelRbf<Point>         Kernel;
      typedef core::Buffer1D<double>   Vector;
      typedef core::Matrix<double>     Matrix;

   public:
      KernelPreImageMDS( )
      {
      }

      /**
       @brief compute the preimage
       @param kernelPca the kernel PCA model
       @param feature the point in feature space we want to find the pre-image
       @param nbNeighbours the number of neighbours to be used to compute the preimage
       */
      Point preimage( const KernelPca<Point, Kernel>& kernelPca, const Point& feature, ui32 nbNeighbours = 7 ) const
      {
         ensure( kernelPca.getSupports().size() > 0, "kernel PCA not trained!" );

         // compute the distance in input space by method 1
         const Kernel& kernel = kernelPca.getKernel();
         const ui32 nbSupports = static_cast<ui32>( kernelPca.getSupports().size() );
         Vector df( nbSupports );   // ||feature, support||_2^2 distance in feature space
         std::vector< std::pair<double, ui32> > d;
         for ( ui32 n = 0; n < nbSupports; ++n )
         {
            df[ n ] = 2 - 2 * kernel( feature, kernelPca.getSupports()[ n ] );
            const double di = - kernel.getVar() * log( 1 - 0.5 * df[ n ] );
            d.push_back( std::make_pair( di, n ) );
         }

         // compute the neighbours
         std::sort( d.begin(), d.end() );
         nbNeighbours = std::min<ui32>( nbNeighbours, d.size() ); // if there is less than <nbNeighbours> then use only these ones

         // center the neighbours
         const std::vector<Point>& supports = kernelPca.getSupports();
         const ui32 pointDim = static_cast<ui32>( supports[ 0 ].size() );
         Matrix centered( pointDim, nbNeighbours );
         Vector mean( pointDim );
         for ( ui32 dim = 0; dim < pointDim; ++dim )
         {
            double sum = 0;
            for ( ui32 n = 0; n < nbNeighbours; ++n )
            {
               const ui32 id = d[ n ].second;
               sum += supports[ id ][ dim ];
            }
            mean[ dim ] = sum / nbNeighbours;
            for ( ui32 n = 0; n < nbNeighbours; ++n )
            {
               const ui32 id = d[ n ].second;
               for ( ui32 dim = 0; dim < pointDim; ++dim )
               {
                  centered( dim, n ) = supports[ id ][ dim ] - mean[ dim ];
               }
            }
         }

         // compute the projection
         Vector eiv;
         Matrix r;
         bool result = core::svdcmp( centered, eiv, r );
         ensure( result, "cannot compute SVD" );

         // get the rank

         r.print( std::cout );
         eiv.print( std::cout );
         


         df.print( std::cout );

         centered.print( std::cout );

         /*
         [U,L,V] = svd(X*H);
         r = rank(L);

         Z = L*V';

         d02 = sum(Z.^2)';

         z = -0.5*pinv(Z')*(d2-d02);
         x = U*z + sum(X,2)/nn;
         */

         return Point();
      }
   };
}
}

// example validated against matlab prototype
// http://www.heikohoffmann.de/documents/hoffmann_kpca_preprint.pdf
// http://www.heikohoffmann.de/publications.html#hoffmann06b
// http://www.heikohoffmann.de/kpca/kpca.zip
class TestKernelPca
{
public:
   /**
    */
   void simpleTest()
   {
      typedef std::vector<double>               Point;
      typedef std::vector<Point>                Points;
      typedef nll::algorithm::KernelRbf<Point>   Kernel;
      typedef nll::algorithm::KernelPca<Point, Kernel>  KernelPca;

      const unsigned size = nll::core::getStaticBufferSize( test_data );
      Points points;
      for ( unsigned n = 0; n < size; ++n )
      {
         points.push_back( nll::core::make_vector<double>( test_data[ n ][ 0 ], test_data[ n ][ 1 ] ) );
      }

      KernelPca kpca;
      Kernel rbfKernel( 0.02 );
      kpca.compute( points, 5, rbfKernel );

      KernelPca::Vector v = kpca.transform( points[ 0 ] );

      TESTER_ASSERT( v.size() == 5 );
      TESTER_ASSERT( fabs( v[ 0 ] - -0.36494782009061227 ) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 1 ] - 0.51765993789384512) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 2 ] - -0.26564637758736048) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 3 ] - 0.27011779549760634) < 1e-4 );
      TESTER_ASSERT( fabs( v[ 4 ] - 0.085854182000892176) < 1e-4 );
      v.print( std::cout );

      std::stringstream ss;
      kpca.write( ss );

      KernelPca kpca2;
      kpca2.read( ss );

      KernelPca::Vector v2 = kpca2.transform( points[ 0 ] );
      TESTER_ASSERT( v2.size() == 5 );
      TESTER_ASSERT( fabs( v2[ 0 ] - -0.36494782009061227 ) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 1 ] - 0.51765993789384512) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 2 ] - -0.26564637758736048) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 3 ] - 0.27011779549760634) < 1e-4 );
      TESTER_ASSERT( fabs( v2[ 4 ] - 0.085854182000892176) < 1e-4 );
   }

   void simplePreimageTest()
   {
      typedef std::vector<double>               Point;
      typedef std::vector<Point>                Points;
      typedef nll::algorithm::KernelRbf<Point>   Kernel;
      typedef nll::algorithm::KernelPca<Point, Kernel>  KernelPca;

      const unsigned size = nll::core::getStaticBufferSize( test_data );
      Points points;
      for ( unsigned n = 0; n < size; ++n )
      {
         points.push_back( nll::core::make_vector<double>( test_data[ n ][ 0 ], test_data[ n ][ 1 ] ) );
      }

      KernelPca kpca;
      Kernel rbfKernel( 0.02 );
      kpca.compute( points, 5, rbfKernel );


      nll::algortihm::KernelPreImageMDS<Point, Kernel> preimageGenerator;
      preimageGenerator.preimage( kpca, points[ 0 ], 3 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestKernelPca);
//TESTER_TEST(simpleTest);
TESTER_TEST(simplePreimageTest);
TESTER_TEST_SUITE_END();
#endif
