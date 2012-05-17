#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace core
{
   

   

  
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

   void testDdfTansform()
   {
      typedef core::DeformableTransformationRadialBasis<>            RbfTransform;
      typedef core::DeformableTransformationDenseDisplacementField2d DdfTransform;

      DdfTransform::Matrix pstRbf = core::identityMatrix<DdfTransform::Matrix>( 3 );
      core::RbfGaussian rbf1( core::make_buffer1D<float>( 1, 2 ),
                              core::make_buffer1D<float>( 20, 40 ),
                              core::make_buffer1D<float>( 50, 100 ) );
      core::RbfGaussian rbf2( core::make_buffer1D<float>( 2, 4 ),
                              core::make_buffer1D<float>( 30, 70 ),
                              core::make_buffer1D<float>( 20, 20 ) );
      RbfTransform::Rbfs rbfs;
      rbfs.push_back( rbf1 );
      rbfs.push_back( rbf2 );
      RbfTransform rbfTfm( pstRbf, rbfs );

      DdfTransform::Matrix pst = core::identityMatrix<DdfTransform::Matrix>( 3 );
      pst( 0, 2 ) = 10;
      pst( 1, 2 ) = 20;
      DdfTransform ddf( core::vector2ui( 55, 100 ), pst, core::vector2ui( 30, 40 ), rbfTfm );

      // query in source space in MM the RBF origin, we must find the exact value, the other RBF is too far to influence the result // minus interpolation error...
      {
         core::vector2f d = ddf.getDisplacement( core::vector2f( 20, 40 ) );
         TESTER_ASSERT( core::equal<float>( d[ 0 ], 20 + 1, 1e-1 ) );
         TESTER_ASSERT( core::equal<float>( d[ 1 ], 40 + 2, 1e-1 ) );
      }

      {
         core::vector2f d = ddf.getDisplacement( core::vector2f( 30, 70 ) );
         TESTER_ASSERT( core::equal<float>( d[ 0 ], 30 + 2, 1e-1 ) );
         TESTER_ASSERT( core::equal<float>( d[ 1 ], 70 + 4, 1e-1 ) );
      }

      // now check a point in between
      ddf.getStorage().getPst().print( std::cout );
      core::vector2f p2( 25, 60 );
      core::Buffer1D<float> expectedDisplacement = rbfTfm.getDeformableDisplacementOnly( p2 );
      core::vector2f d = ddf.getDisplacement( p2 ) - p2;

      // the error is due to the interpolation...
      TESTER_ASSERT( core::equal<float>( d[ 0 ], expectedDisplacement[ 0 ], (float)1e-2 ) );
      TESTER_ASSERT( core::equal<float>( d[ 1 ], expectedDisplacement[ 1 ], (float)1e-2 ) );
   }

   void testDdfResampling()
   {
      typedef core::DeformableTransformationRadialBasis<>            RbfTransform;
      typedef core::DeformableTransformationDenseDisplacementField2d DdfTransform;
      typedef core::ImageSpatial<ui8>                                ImageSpatial1;

      // compute a simple RBF transform
      DdfTransform::Matrix tfm = core::identityMatrix<DdfTransform::Matrix>( 3 );
      core::RbfGaussian rbf1( core::make_buffer1D<float>( 0, 0 ),
                              core::make_buffer1D<float>( 20, 40 ),
                              core::make_buffer1D<float>( 50, 100 ) );
      RbfTransform::Rbfs rbfs;
      rbfs.push_back( rbf1 );
      RbfTransform rbfTfm( tfm, rbfs );

      // export the RBF transform as a DDF
      DdfTransform::Matrix pst = core::identityMatrix<DdfTransform::Matrix>( 3 );
      pst( 0, 2 ) = 10;
      pst( 1, 2 ) = 20;
      DdfTransform ddf( core::vector2ui( 30, 60 ), pst, core::vector2ui( 30, 60 ), rbfTfm );

      // setup the images
      ImageSpatial1 target( 30, 30, 3, pst );
      ImageSpatial1 resampled( 30, 60, 3, pst );

      // finally do some resampling
      core::ImageTransformationMapperDdf ddfTransformationMapper;
      typedef core::InterpolatorNearestNeighbor2D<ImageSpatial1::value_type, ImageSpatial1::IndexMapper, ImageSpatial1::Allocator> Interpolator;
      typedef core::ImageTransformationProcessorResampler<ImageSpatial1, Interpolator> Processor;
      Processor proc( target, resampled );
      ddfTransformationMapper.run( proc, target, ddf, resampled );
   }

   void testDdfResamplingEx()
   {
      typedef core::DeformableTransformationRadialBasis<>            RbfTransform;
      typedef core::DeformableTransformationDenseDisplacementField2d DdfTransform;
      typedef core::ImageSpatial<ui8>                                ImageSpatial1;

      // compute a simple RBF transform
      DdfTransform::Matrix tfm = core::identityMatrix<DdfTransform::Matrix>( 3 );
      core::RbfGaussian rbf1( core::make_buffer1D<float>( -30, 0 ),
                              core::make_buffer1D<float>( 41, 41 ),
                              core::make_buffer1D<float>( 300, 300 ) );
      RbfTransform::Rbfs rbfs;
      rbfs.push_back( rbf1 );
      RbfTransform rbfTfm( tfm, rbfs );

      // setup the images
      core::Image<ui8> sourceImage;
      DdfTransform::Matrix pst = core::identityMatrix<DdfTransform::Matrix>( 3 );
      core::readBmp( sourceImage, NLL_TEST_PATH "data/image/test-image3.bmp" );
      ImageSpatial1 target( sourceImage, pst );
      ImageSpatial1 resampled( sourceImage.sizex(), sourceImage.sizey(), 3, pst );

      // export the RBF transform as a DDF
      DdfTransform ddf( core::vector2ui( target.sizex(), target.sizey() ), pst, core::vector2ui( 80, 80 ), rbfTfm );

      // finally do some resampling
      core::Timer t1;
      core::resampleLinear( target, ddf, resampled );
      std::cout << "Time resampling=" << t1.getCurrentTime() << std::endl;
      core::writeBmp( resampled, NLL_TEST_PATH "data/resampled.bmp" );
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
 TESTER_TEST(testDdfTansform);
 TESTER_TEST(testDdfResampling);
 TESTER_TEST(testDdfResamplingEx);
TESTER_TEST_SUITE_END();
#endif