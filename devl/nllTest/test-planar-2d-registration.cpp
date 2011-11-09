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
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestPlanarRegistration);
//TESTER_TEST(testResampling);
TESTER_TEST(testRegistration);
TESTER_TEST_SUITE_END();
#endif