#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;
using namespace nll::algorithm;

class TestPlanarRegistration
{
public:
   /*
   // check we have the same resampling between image resampling and spatial resampling
   void testResampling()
   {
      core::Image<ui8> src( NLL_TEST_PATH "data/image/test-image2.bmp" );
      core::Image<ui8> resampled( src.sizex(), src.sizey(), 3 );

      core::Matrix<float> id = core::identityMatrix< core::Matrix<float> >( 3 );
      core::ImageSpatial<ui8> srcsp( src, id );
      core::ImageSpatial<ui8> resampledsp( src.sizex(), src.sizey(), 3, id );

      core::vector3ui black( 0, 0, 0 );
      core::Matrix<double> tfm = core::createTransformationAffine2D(  1 * 0.1, core::vector2d( 1, 1 ), core::vector2d(  0 * (int)src.sizey() + 0, 1 * -20 ) );
      core::Matrix<float> tfmf;
      tfmf.import(tfm);

      core::inverse(tfm);


      core::resampleNearestNeighbour( src, resampled, tfm, black );
      core::resampleNearest( srcsp, tfmf, resampledsp );
      resampledsp( 0, 0, 0 ) = 0;
      resampledsp( 0, 0, 1 ) = 0;
      resampledsp( 0, 0, 2 ) = 255;

      core::writeBmp( resampled, NLL_TEST_PATH "data/test-resampling-image1.bmp" );
      core::writeBmp( resampledsp, NLL_TEST_PATH "data/test-resampling-image-sp1.bmp" );
   }*/

   static core::Image<ui8> applyRegistration( core::Image<ui8>& source, 
                                              core::Image<ui8>& target,
                                              const core::Matrix<double>& srcToTarget )
   {
      ensure( source.getNbComponents() == 1, "BAD" );
      ensure( target.getNbComponents() == 1, "BAD" );
      core::Matrix<float> id = core::identityMatrix< core::Matrix<float> >( 3 );
      core::ImageSpatial<ui8> srcsp( source, id );
      core::ImageSpatial<ui8> tgtsp( target, id );
      core::ImageSpatial<ui8> resampledsp( source.sizex(), source.sizey(), 1, id );

      core::Matrix<float> tfmf;
      tfmf.import(srcToTarget);
      core::resampleLinear( tgtsp, tfmf, resampledsp );
      core::extend( resampledsp, 3 );
      for ( ui32 ny = 0; ny < source.sizey(); ++ny )
      {
         for ( ui32 nx = 0; nx < source.sizex(); ++nx )
         {
            for ( ui32 c = 0; c < 2; ++c )
            {
               resampledsp( nx, ny, c ) = (ui8)NLL_BOUND( source( nx, ny, 0 ) + ( c == 2 ) * resampledsp( nx, ny, c ), 0, 255);
            }
         }
      }

      return resampledsp;
   }

   static void displayTransformation( const core::Image<ui8>& source, const core::Image<ui8>& target, core::Image<ui8>& output, const core::Matrix<double>& srcToTgtTfm )
   {
      core::Image<ui8> targetc = target;

      core::Matrix<float> id = core::identityMatrix< core::Matrix<float> >( 3 );
      core::ImageSpatial<ui8> resampled( source.sizex(), source.sizey(), 3, id );
      core::ImageSpatial<ui8> targetSp( targetc, id );

      core::Matrix<float> tfmd;
      tfmd.import( srcToTgtTfm );
      core::resampleNearest( targetSp, tfmd, resampled );

      output = resampled;
      for ( ui32 y = 0; y < source.sizey(); ++y )
      {
         for ( ui32 x = 0; x < source.sizex(); ++x )
         {
            for ( ui32 c = 0; c < 3; ++c )
            {
               const double val = ( 2 * (c==2) * source( x, y, c )  + output( x, y, c ) );
               output( x, y, c ) = NLL_BOUND( val, 0, 255 );
            }
         }
      }
   }

   static void composeMatch( const core::Image<ui8>& i1,
                             const core::Image<ui8>& i2,
                             core::Image<ui8>& output,
                             const algorithm::AffineRegistrationPointBased2d<>::PointPairs& points )
   {
      output = core::Image<ui8>( i1.sizex() + i2.sizex(),
                                 std::max( i1.sizey(), i2.sizey() ),
                                 3 );
      for ( ui32 y = 0; y < i1.sizey(); ++y )
      {
         for ( ui32 x = 0; x < i1.sizex(); ++x )
         {
            output( x, y, 0 ) = i1( x, y, 0 );
            output( x, y, 1 ) = i1( x, y, 1 );
            output( x, y, 2 ) = i1( x, y, 2 );
         }
      }

      for ( ui32 y = 0; y < i2.sizey(); ++y )
      {
         for ( ui32 x = 0; x < i2.sizex(); ++x )
         {
            output( x + i1.sizex(), y, 0 ) = i2( x, y, 0 );
            output( x + i1.sizex(), y, 1 ) = i2( x, y, 1 );
            output( x + i1.sizex(), y, 2 ) = i2( x, y, 2 );
         }
      }

      for ( ui32 n = 0; n < (ui32)points.size(); ++n )
      {
         const algorithm::SpeededUpRobustFeatures::Point& f1 = points[ n ].first;
         const algorithm::SpeededUpRobustFeatures::Point& f2 = points[ n ].second;
         core::bresham( output, f1.position, core::vector2i( f2.position[ 0 ] + i1.sizex(), f2.position[ 1 ] ), core::vector3uc( (ui8)( rand() % 128 ), (ui8)( rand() % 128 ), (ui8)( rand() % 128 ) ) );
      }
   }

   static void printPointsSimple( core::Image<ui8>& output, const algorithm::SpeededUpRobustFeatures::Points& points )
   {
      for ( ui32 n = 0; n < points.size(); ++n )
      {
         ui32 px = points[ n ].position[ 0 ];
         ui32 py = points[ n ].position[ 1 ];
         if ( px < output.sizex() && py < output.sizey() )
         {
            output( px, py, 0 ) = 0;
            output( px, py, 1 ) = 0;
            output( px, py, 2 ) = 255;
         }
      }
   }

   static void printDebug( const core::Image<ui8>& projSource,
                           const algorithm::SpeededUpRobustFeatures::Points& pointsSource,
                           const core::Image<ui8>& projTarget,
                           const algorithm::SpeededUpRobustFeatures::Points& pointsTarget,
                           core::Image<ui8>& out,
                           const core::Matrix<double>& regTfm,
                           const algorithm::AffineRegistrationPointBased2d<>::PointPairs& inliers )
   {
      core::Image<ui8> src;
      src.clone( projSource );
      core::extend( src, 3 );

      core::Image<ui8> tgt;
      tgt.clone( projTarget );
      core::extend( tgt, 3 );

      // create the blended image
      core::Image<ui8> outputReg;
      displayTransformation( src, tgt, outputReg, regTfm );

      printPointsSimple( src, pointsSource );
      printPointsSimple( tgt, pointsTarget );

      // now print the inliers
      core::Image<ui8> composed;
      composeMatch( src, tgt, composed, inliers );

      // now compose all the images
      core::Image<ui8> final( src.sizex() + tgt.sizex() + outputReg.sizex(),
                              std::max( src.sizey(), tgt.sizey() ),
                              3 );
      for ( ui32 x = 0; x < composed.sizex(); ++x )
      {
         for ( ui32 y = 0; y < composed.sizey(); ++y )
         {
            final( x + outputReg.sizex(), y, 0 ) = composed( x, y, 0 );
            final( x + outputReg.sizex(), y, 1 ) = composed( x, y, 1 );
            final( x + outputReg.sizex(), y, 2 ) = composed( x, y, 2 );
         }
      }

      for ( ui32 x = 0; x < outputReg.sizex(); ++x )
      {
         for ( ui32 y = 0; y < outputReg.sizey(); ++y )
         {
            final( x, y, 0 ) = outputReg( x, y, 0 );
            final( x, y, 1 ) = outputReg( x, y, 1 );
            final( x, y, 2 ) = outputReg( x, y, 2 );
         }
      }

      // print the matched points
      for ( ui32 n = 0; n < inliers.size(); ++n )
      {
         const algorithm::SpeededUpRobustFeatures::Point& source = inliers[ n ].first;
         final( source.position[ 0 ], source.position[ 1 ], 0 ) = 255;
         final( source.position[ 0 ], source.position[ 1 ], 1 ) = 0;
         final( source.position[ 0 ], source.position[ 1 ], 2 ) = 0;
      }

      out = final;
   }

   // Simple pipeline to check that everything is implemented correctly: registration, resampling
   // and the transformation roles for all the imaging algorithms involved
   void testRegistration()
   {
      const double angle = 0.1;
      const core::vector2d t( 15, -10 );

      core::Image<ui8> src( NLL_TEST_PATH "data/image/test-image2.bmp" );
      core::Image<ui8> resampled( src.sizex(), src.sizey(), 1 );
      core::decolor( src );

      core::Matrix<float> id = core::identityMatrix< core::Matrix<float> >( 3 );
      core::ImageSpatial<ui8> srcsp( src, id );
      core::ImageSpatial<ui8> resampledsp( src.sizex(), src.sizey(), 1, id );

      core::vector3ui black( 0, 0, 0 );
      core::Matrix<double> tfm = core::createTransformationAffine2D(  angle, core::vector2d( 1, 1 ), t );
      core::Matrix<float> tfmf;
      tfmf.import(tfm);

      core::Matrix<double> sourceToTargetTfm;
      sourceToTargetTfm.clone( tfm );
      core::inverse( sourceToTargetTfm );
      std::cout << "source->target tfm=" << std::endl;
      sourceToTargetTfm.print(std::cout);

      // compute the registration (srcsp->tfmf)^-1
      core::resampleLinear( srcsp, tfmf, resampledsp );
      algorithm::AffineRegistrationPointBased2d<> reg;
      core::Matrix<double> tfmFound = reg.compute( srcsp, resampledsp );

      tfmFound.print( std::cout );

      core::extend( resampledsp, 3 );
      core::writeBmp( resampledsp, NLL_TEST_PATH "data/test-resampling-image1-target.bmp" );

      core::ImageSpatial<ui8> resampled2( src.sizex(), src.sizey(), 1, id );
      core::decolor( resampledsp );
      tfmf.import(tfmFound);
      core::resampleLinear( resampledsp, tfmf, resampled2 );

      core::Image<ui8> i = applyRegistration( src, resampledsp, tfmFound );
      core::writeBmp( i, "c:/tmp/resampled.bmp" );

      double accumError = 0;
      ui32 nb = 0;
      for ( ui32 n = 0; n < resampled2.size(); ++n )
      {
         if ( resampled2[ n ] > 0 )
         {
            accumError += core::sqr( resampled2[ n ] - src[ n ] );
            ++nb;
         }
      }

      const double error = sqrt(accumError / nb);

      std::cout << "meanError=" << error << std::endl;

      core::extend( resampled2, 3 );
      core::writeBmp( resampled2, NLL_TEST_PATH "data/test-resampling-image1-target-tfm.bmp" );
      core::extend( src, 3 );
      core::writeBmp( src, NLL_TEST_PATH "data/test-resampling-image1-source-tfm.bmp" );

      TESTER_ASSERT( error < 70, "Error too big, registration is wrong!" );
   }

   void debugRegistration()
   {
      srand(1);

      core::Image<ui8> src( "c:/tmp/xySource.bmp" );
      core::Image<ui8> tgt( "c:/tmp/xyTarget.bmp" );

      core::decolor( src );
      core::decolor( tgt );

      algorithm::AffineRegistrationPointBased2d<> reg( 5, 6, 0.00001);
      core::Matrix<double> tfmFound = reg.compute( src, tgt );

      core::Image<ui8> i;// = applyRegistration( src, tgt, tfmFound );

      printDebug( src, reg.getPoints1(), tgt, reg.getPoints2(), i, tfmFound, reg.getInliers() );
      core::writeBmp( i, "c:/tmp/resampled.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestPlanarRegistration);
//TESTER_TEST(testResampling);
//TESTER_TEST(testRegistration);
TESTER_TEST(debugRegistration);
TESTER_TEST_SUITE_END();
#endif