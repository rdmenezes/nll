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
      for ( size_t x = 0; x < i.sizex(); ++x )
      {
         for ( size_t y = 0; y < i.sizey(); ++y )
         {
            for ( size_t c = 0; c < i.getNbComponents(); ++c )
            {
               iout( x, y, c ) = (ui8)NLL_BOUND( i(x, y, c ), 0, 255 );
            }
         }
      }
      return iout;
   }

   static Image createCheckerboard0()
   {
      // encode 2 checkers at index [0] and [1]
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
      for ( size_t x = 0; x < i.sizex(); ++x )
      {
         for ( size_t y = 0; y < i.sizey(); ++y )
         {
            for ( size_t c = 0; c < i.getNbComponents(); ++c )
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
      TESTER_ASSERT( core::equal<double>( out[ 0 ], 0.50, 1e-3 ) );

      out[ 0 ] = interpolator.interpolate( 0.50, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0.5 );

      out[ 0 ] = interpolator.interpolate( 0.50, 0.50, 0 );
      TESTER_ASSERT( out[ 0 ] == 0.5 );

// don't run this UT : floor doesn't behave correctly in [-0.5; -1]
//      out[ 0 ] = interpolator.interpolate( -0.51, -0.51, 0 );
//      TESTER_ASSERT( out[ 0 ] == 0 );

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
      for ( size_t x = 0; x < i.sizex(); ++x )
      {
         for ( size_t y = 0; y < i.sizey(); ++y )
         {
            size_t ratiox = i.sizex() / oi.sizex();
            size_t ratioy = i.sizey() / oi.sizey();
            const size_t xi = x / ratiox;
            const size_t yi = y / ratioy;
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
      core::rescaleNearestNeighbor( i, 255, 255 );

      Image resampled(255, 255, 3 );
      double col[ 3 ] = {0, 0, 0};

      const size_t ratiox = i.sizex() / oi.sizex();
      const size_t ratioy = i.sizey() / oi.sizey();

      core::Matrix<double> tfm = core::identityMatrix< core::Matrix<double> >( 3 );
      tfm( 0, 0 ) = ratiox;
      tfm( 1, 1 ) = ratioy;

      core::resampleNearestNeighbour( oi, resampled, tfm, col );

      core::writeBmp( toImagec(i), "c:/tmp/reampled.bmp" );
      core::writeBmp( toImagec(resampled), "c:/tmp/reampled2.bmp" );

      for ( size_t x = 0; x < i.sizex(); ++x )
      {
         for ( size_t y = 0; y < i.sizey(); ++y )
         {
            const size_t xi = x / ratiox;
            const size_t yi = y / ratioy;
            if ( ( x % ratiox ) != 0 && ( y % ratioy ) != 0 )
            {
               TESTER_ASSERT( i( x, y, 0 ) == oi( xi, yi, 0 ) );
               TESTER_ASSERT( i( x, y, 1 ) == oi( xi, yi, 1 ) );
               TESTER_ASSERT( i( x, y, 2 ) == oi( xi, yi, 2 ) );
            }
         }
      }

      for ( size_t x = 0; x < i.sizex(); ++x )
      {
         for ( size_t y = 0; y < i.sizey(); ++y )
         {
            TESTER_ASSERT( i( x, y, 0 ) == resampled( x, y, 0 ) );
            TESTER_ASSERT( i( x, y, 1 ) == resampled( x, y, 1 ) );
            TESTER_ASSERT( i( x, y, 2 ) == resampled( x, y, 2 ) );
         }
      }
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
      core::Buffer1D<float> expectedDisplacement = rbfTfm.getRawDeformableDisplacementOnly( p2 );
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
      const std::string file = NLL_TEST_PATH "data/image/test-image3.bmp";
      typedef core::Image<ui8>::Allocator Alloc;
      core::readBmp( sourceImage, file, Alloc() );
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

   // regression tests.. doesn't mean the values are true here!
   void testImageMapper()
   {
      typedef core::Matrix<double> Matrix;
      typedef core::Matrix<float> Matrixf;
      typedef core::ImageSpatial<double> Image;
      typedef core::InterpolatorLinear2D<double, Image::IndexMapper, Image::Allocator> Interpolator;

      srand( 0 );
      Matrix pstTarget = core::createTransformationAffine2D(0.1,
                                                            core::vector2d( 0.95, 1.05 ),
                                                            core::vector2d( -3, 1 ) );
      Matrix pstResampled = core::createTransformationAffine2D(-0.05,
                                                            core::vector2d( 1.04, 1.03 ),
                                                            core::vector2d( 1, 2 ) );
      Matrix affineTfm = core::createTransformationAffine2D(-0.15,
                                                            core::vector2d( 0.90, 0.92 ),
                                                            core::vector2d( 1, 1.1 ) );

      Matrixf pstTargetf;
      pstTargetf.import( pstTarget );

      Matrixf pstResampledf;
      pstResampledf.import( pstResampled );

      Matrixf affineTfmf;
      affineTfmf.import( affineTfm );

      Image resampled( 256, 512, 1, pstResampledf );
      Image target( 256, 512, 1, pstTargetf );

      for ( Image::iterator it = target.begin(); it != target.end(); ++it )
      {
         *it = core::generateUniformDistribution( 100, 500 );
      }      

      core::ImageTransformationProcessorResampler<Image, Interpolator> resampler( target, resampled );
      core::ImageTransformationMapper mapper;
      mapper.run( resampler, target, affineTfmf, resampled );

      std::cout << resampled( 0, 0, 0 ) << std::endl;
      std::cout << resampled( 8, 0, 0 ) << std::endl;
      std::cout << resampled( 0, 9, 0 ) << std::endl;
      std::cout << resampled( 0, 3, 0 ) << std::endl;
      std::cout << resampled( 12, 30, 0 ) << std::endl;
      std::cout << resampled( 150, 80, 0 ) << std::endl;

      // these values were computed using this library and are for regression tests only (it is assumed correct...)
      {
         const float v = resampled( 0, 0, 0 );
         TESTER_ASSERT( core::equal<float>( v, 224.195, 1e-2 ) );
      }

      {
         const float v = resampled( 8, 0, 0 );
         TESTER_ASSERT( core::equal<float>( v, 0, 1e-2 ) );
      }
      {
         const float v = resampled( 0, 9, 0 );
         TESTER_ASSERT( core::equal<float>( v, 259.206, 1e-2 ) );
      }
      {
         const float v = resampled( 0, 3, 0 );
         TESTER_ASSERT( core::equal<float>( v, 321.13, 1e-2 ) );
      }
      {
         const float v = resampled( 12, 30, 0 );
         TESTER_ASSERT( core::equal<float>( v, 220.669, 1e-2 ) );
      }
      {
         const float v = resampled( 150, 80, 0 );
         TESTER_ASSERT( core::equal<float>( v, 413.67, 1e-2 ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestDeformable2D);
 TESTER_TEST(testResamplerNearest2Dvals);
 TESTER_TEST(testResamplerNearest2D);
 TESTER_TEST(testResamplerLinear2Dvals);
 TESTER_TEST(testResizeNn);
 TESTER_TEST(testResamplerLinear2D);
 TESTER_TEST(testResampleNn);
 TESTER_TEST(testDdfTansform);
 TESTER_TEST(testDdfResampling);
 TESTER_TEST(testDdfResamplingEx);
 TESTER_TEST(testImageMapper);
TESTER_TEST_SUITE_END();
#endif