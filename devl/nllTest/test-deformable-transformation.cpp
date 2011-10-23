#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
   class RadialBasisFunctions
   {
      typedef double  value_type;

   public:
      /**
       @brief Defines a Gaussian RBF parametrized by a weigh, variance and mean.
              If the mean has size > 1, creates a RBF with the same number of values
       */
      struct GaussianRbf
      {
         value_type                    weight;
         core::Buffer1D<value_type>    mean;
         core::Buffer1D<value_type>    var;

         template <class Vector>
         core::Buffer1D<value_type> eval( const Vector& v )
         {
            ensure( v.size() == mean.size(), "vector size mismatch" );
            core::Buffer1D<value_type> r( v.size() );
            for ( ui32 n = 0; n < mean.size(); ++n )
            {
               const value_type diff = v[ n ] - mean[ n ];
               r[ n ] = std::exp( - diff * diff / ( 2 * var[ n ] ) );
            }

            return r;
         }
      };
      typedef std::vector<GaussianRbf> Rbfs;

   public:
      RadialBasisFunctions()
      {}

      RadialBasisFunctions( const Rbfs& g ) : _rbfs( g )
      {
      }

      void clear()
      {
         _rbfs.clear();
      }

      void add( const GaussianRbf& g )
      {
         _rbfs.push_back( g );
      }

      GaussianRbf& operator[]( size_t i )
      {
         return _rbfs[ i ];
      }

      const GaussianRbf& operator[]( size_t i ) const
      {
         return _rbfs[ i ];
      }

      size_t size() const
      {
         return _rbfs.size();
      }

   private:
      Rbfs   _rbfs;
   };


   /**
    @brief Export a mixture of gaussians as a dense displacement field
    @param mixture the gaussian mixture. The intensity at a given point is computed from the weighted gaussian sum
    */
   template <class T, class Mapper, class Allocator>
   void exportAsDenseDensityField( const RadialBasisFunctions& mixture, core::Image<T, Mapper, Allocator>& outDdf )
   {
   }
}
}

struct TestDeformable2D
{
   typedef core::Image<double>   Image;
   typedef core::Image<ui8>      Imagec;

   Imagec toImagec( const Image& i )
   {
      Imagec iout( i.sizex(), i.sizey(), i.getNbComponents() );
      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            for ( ui32 c = 0; c < i.getNbComponents(); ++c )
            {
               iout( x, y, c ) = (ui8)NLL_BOUND( i(x, y, c ), 0, 255 );
            }
         }
      }
      return iout;
   }

   static Image createCheckerboard0()
   {
      Image i( 3, 3, 2 );
      i( 0, 0, 0 ) = 1;
      i( 1, 0, 0 ) = 0;
      i( 2, 0, 0 ) = 1;

      i( 0, 1, 0 ) = 0;
      i( 1, 1, 0 ) = 1;
      i( 2, 1, 0 ) = 0;

      i( 0, 2, 0 ) = 1;
      i( 1, 2, 0 ) = 0;
      i( 2, 2, 0 ) = 1;

      i( 0, 0, 1 ) = 0;
      i( 1, 0, 1 ) = 1;
      i( 2, 0, 1 ) = 0;

      i( 0, 1, 1 ) = 1;
      i( 1, 1, 1 ) = 0;
      i( 2, 1, 1 ) = 1;

      i( 0, 2, 1 ) = 0;
      i( 1, 2, 1 ) = 1;
      i( 2, 2, 1 ) = 0;

      return i;
   }

   static Image createCheckerboard()
   {
      Image i = createCheckerboard0();
      Image iout( i.sizex(), i.sizey(), 3 );
      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            for ( ui32 c = 0; c < i.getNbComponents(); ++c )
            {
               iout( x, y, c ) = i(x, y, 0 ) * 255;
            }
         }
      }
      return iout;
   }

   void testResamplerNearest2Dvals()
   {
      typedef core::InterpolatorNearestNeighbor2D<Image::value_type, Image::IndexMapper, Image::Allocator> Interpolator;

      Image i = createCheckerboard0();
      Interpolator interpolator( i );
      
      Image::value_type out[ 2 ];
      interpolator.interpolateValues( 0, 0, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0.49, 0, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( -0.49, -0.49, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0.50, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 1 );

      interpolator.interpolateValues( 0.50, 0.50, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( -0.51, -0.51, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 1, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 1 );

      interpolator.interpolateValues( 2, 0, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 2.49, 0, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0, 2.49, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0, 2.51, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 2.51, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );
   }

   void testResamplerNearest2D()
   {
      typedef core::InterpolatorNearestNeighbor2D<Image::value_type, Image::IndexMapper, Image::Allocator> Interpolator;

      Image i = createCheckerboard0();
      Interpolator interpolator( i );
      
      Image::value_type out[ 1 ];
      out[ 0 ] = interpolator.interpolate( 0, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 0.49, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( -0.49, -0.49, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 0.50, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 0.50, 0.50, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( -0.51, -0.51, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 1, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 2, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 2.49, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 0, 2.49, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 0, 2.51, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 2.51, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );
   }

   void testResamplerLinear2Dvals()
   {
      typedef core::InterpolatorLinear2D<Image::value_type, Image::IndexMapper, Image::Allocator> Interpolator;

      Image i = createCheckerboard0();
      Interpolator interpolator( i );

      Image::value_type out[ 2 ];
      interpolator.interpolateValues( 0, 0, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0.49, 0, out );
      TESTER_ASSERT( core::equal<double>( out[ 0 ], 0.51, 1e-3 ) && core::equal<double>( out[ 1 ], 0.49, 1e-3 ) );

      interpolator.interpolateValues( -0.49, -0.49, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0.50, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0.5 && out[ 1 ] == 0.5 );

      interpolator.interpolateValues( 0.50, 0.50, out );
      TESTER_ASSERT( out[ 0 ] == 0.5 && out[ 1 ] == 0.5 );

      interpolator.interpolateValues( -0.51, -0.51, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 1, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 1 );

      interpolator.interpolateValues( 2, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 ); // we are just one the border...

      interpolator.interpolateValues( 1.999999, 0, out );
      TESTER_ASSERT( core::equal<double>( out[ 0 ], 1, 1e-3 ) && core::equal<double>( out[ 1 ], 0, 1e-3 ) ); // we are just one the border...

      interpolator.interpolateValues( 2.49, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0, 2.49, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0, 2.51, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 2.51, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );
   }

   void testResamplerLinear2D()
   {
      typedef core::InterpolatorLinear2D<Image::value_type, Image::IndexMapper, Image::Allocator> Interpolator;

      Image i = createCheckerboard0();
      Interpolator interpolator( i );

      Image::value_type out[ 1 ];
      out[ 0 ] = interpolator.interpolate( 0, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 0.49, 0, 0 );
      TESTER_ASSERT( core::equal<double>( out[ 0 ], 0.51, 1e-3 ) );

      out[ 0 ] = interpolator.interpolate( -0.49, -0.49, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 0.50, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0.5 );

      out[ 0 ] = interpolator.interpolate( 0.50, 0.50, 0 );
      TESTER_ASSERT( out[ 0 ] == 0.5 );

      out[ 0 ] = interpolator.interpolate( -0.51, -0.51, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 1, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 2, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 ); // we are just one the border...

      out[ 0 ] = interpolator.interpolate( 1.999999, 0, 0 );
      TESTER_ASSERT( core::equal<double>( out[ 0 ], 1, 1e-3 ) ); // we are just one the border...

      out[ 0 ] = interpolator.interpolate( 2.49, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 0, 2.49, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 0, 2.51, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 2.51, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );
   }

   void testResizeNn()
   {
      Image oi = createCheckerboard();
      Image i = createCheckerboard();
      core::rescaleNearestNeighbor( i, 256, 256 );
      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            ui32 ratiox = i.sizex() / oi.sizex();
            ui32 ratioy = i.sizey() / oi.sizey();
            const ui32 xi = x / ratiox;
            const ui32 yi = y / ratioy;
            if ( ( x % ratiox ) != 0 && ( y % ratioy ) != 0 )
            {
               TESTER_ASSERT( i( x, y, 0 ) == oi( xi, yi, 0 ) );
            }
         }
      }
      //core::writeBmp( toImagec(i), "c:/tmp/reampled.bmp" );
   }

   // check the resampling with a zoom factor, check we find the same result than resizing
   void testResampleNn()
   {
      Image oi = createCheckerboard();
      Image i = createCheckerboard();
      core::rescaleNearestNeighbor( i, 256, 256 );

      Image resampled(256, 256, 3 );
      double col[ 3 ] = {0, 0, 0};

      const ui32 ratiox = i.sizex() / oi.sizex();
      const ui32 ratioy = i.sizey() / oi.sizey();

      core::Matrix<double> tfm = core::identityMatrix< core::Matrix<double> >( 3 );
      tfm( 0, 0 ) = ratiox;
      tfm( 1, 1 ) = ratioy;

      core::resampleNearestNeighbour( oi, resampled, tfm, col );
      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            const ui32 xi = x / ratiox;
            const ui32 yi = y / ratioy;
            if ( ( x % ratiox ) != 0 && ( y % ratioy ) != 0 )
            {
               TESTER_ASSERT( i( x, y, 0 ) == oi( xi, yi, 0 ) );
               TESTER_ASSERT( i( x, y, 1 ) == oi( xi, yi, 1 ) );
               TESTER_ASSERT( i( x, y, 2 ) == oi( xi, yi, 2 ) );
            }
         }
      }

      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            TESTER_ASSERT( i( x, y, 0 ) == resampled( x, y, 0 ) );
            TESTER_ASSERT( i( x, y, 1 ) == resampled( x, y, 1 ) );
            TESTER_ASSERT( i( x, y, 2 ) == resampled( x, y, 2 ) );
         }
      }

      //core::writeBmp( toImagec(i), "c:/tmp/reampled.bmp" );
      //core::writeBmp( toImagec(resampled), "c:/tmp/reampled2.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestDeformable2D);
 TESTER_TEST(testResamplerNearest2Dvals);
 TESTER_TEST(testResamplerNearest2D);
 TESTER_TEST(testResamplerLinear2Dvals);
 TESTER_TEST(testResamplerLinear2D);
 TESTER_TEST(testResizeNn);
 TESTER_TEST(testResampleNn);
TESTER_TEST_SUITE_END();
#endif