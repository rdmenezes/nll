#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"
#include "utils.h"

namespace nll
{
namespace algorithm
{
   
}
}

using namespace nll;

class TestIntensityBasedRegistration
{
public:
   typedef imaging::VolumeSpatial<ui8>                                         Volume;
   typedef imaging::VolumeSpatial<f32>                                         Volumef;
   typedef algorithm::RegistrationEvaluatorHelper<Volume>::EvaluatorSimilarity RegistrationEvaluator;
   typedef algorithm::RegistrationAlgorithmIntensity<Volume::value_type, Volume::VoxelBuffer> RegistrationAlgorithmIntensity;

   static Volume preprocess( const Volumef& vf, size_t joinHistogramNbBins )
   {
      imaging::LookUpTransformWindowingRGB lut( 0, 1000, joinHistogramNbBins, 1 );
      lut.createGreyscale( (float)joinHistogramNbBins );
      algorithm::VolumePreprocessorLut8bits<Volumef::value_type, Volumef::VoxelBuffer, Volume::VoxelBuffer> preprocess( lut );
      

      return preprocess.run( vf );
   }

   static Volume createSyntheticPerfect( const core::vector3ui& size, const core::vector3ui sphereCenter, double sphereRadiusVoxel, size_t idSphere = 1 )
   {
      Volume v( size, core::identityMatrix< core::Matrix<float> >( 4 ) );
      
      for ( size_t z = 0; z < v.getSize()[ 2 ]; ++z )
      {
         for ( size_t y = 0; y < v.getSize()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < v.getSize()[ 0 ]; ++x )
            {
               const double dist = std::sqrt( core::sqr<double>( sphereCenter[ 0 ] - x ) +
                                              core::sqr<double>( sphereCenter[ 1 ] - y ) +
                                              core::sqr<double>( sphereCenter[ 2 ] - z ) );
               if ( dist < sphereRadiusVoxel )
               {
                  v( x, y, z ) = (ui8)idSphere;
               }
            }
         }
      }

      return v;
   }

   static Volume createSyntheticCube( const core::vector3ui& size, const core::vector3ui center, double radiusVoxel, size_t id = 1 )
   {
      Volume v( size, core::identityMatrix< core::Matrix<float> >( 4 ) );
      size_t nbInside = 0;
      
      for ( size_t z = 0; z < v.getSize()[ 2 ]; ++z )
      {
         for ( size_t y = 0; y < v.getSize()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < v.getSize()[ 0 ]; ++x )
            {
               const bool inside = abs( (int)center[ 0 ] - (int)x ) <= radiusVoxel &&
                                   abs( (int)center[ 1 ] - (int)y ) <= radiusVoxel &&
                                   abs( (int)center[ 2 ] - (int)z ) <= radiusVoxel;

               if ( inside )
               {
                  v( x, y, z ) = (ui8)id;
                  ++nbInside;
               }
            }
         }
      }

      std::cout << "nbVoxelInside=" << nbInside << std::endl;
      return v;
   }

   void testEvaluatorSpecificData()
   {
      const size_t joinHistogramNbBins = 3;
      const core::vector3ui size( 4, 4, 4 );
      Volume v( size, core::identityMatrix< core::Matrix<float> >( 4 ) );
      v( 1, 1, 1 ) = 1;

      // construct the evaluator
      algorithm::TransformationCreatorRigid transformationCreator;
      algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
      RegistrationEvaluator evaluator( v, v, similarity, transformationCreator, histogramMaker, joinHistogramNbBins );

      evaluator.setTransformationCreator( transformationCreator );

      const double v0 = evaluator.evaluate( *transformationCreator.create( core::make_buffer1D<double>( 0, 0, 0 ) ) );
      const double v1 = evaluator.evaluate( *transformationCreator.create( core::make_buffer1D<double>( 0.1, 0, 0 ) ) );
      const double v2 = evaluator.evaluate( *transformationCreator.create( core::make_buffer1D<double>( -0.1, 0, 0 ) ) );

      TESTER_ASSERT( v0 < v1 );
      TESTER_ASSERT( v0 < v2 );      
   }

   void testSimilarity()
   {
      const Volume v = createSyntheticCube( core::vector3ui( 8, 8, 8 ), core::vector3ui( 4, 4, 4 ), 0 );
      const size_t joinHistogramNbBins = 2;

      // construct the evaluator
      algorithm::TransformationCreatorRigid transformationCreator;
      algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
      RegistrationEvaluator evaluator( v, v, similarity, transformationCreator, histogramMaker, joinHistogramNbBins );

      // get the similarity. We expect it to be decreasing until we reach 0, then increasing
      RegistrationEvaluator::SimilarityPlot plot = evaluator.returnSimilarityAlongParameter( core::make_buffer1D<double>( 0.1,  0,  0 ), 0, -0.01, 22 );
      for ( size_t n = 0; n < plot.size() - 1; ++n )
      {
         if ( plot[ n ].first > 0 )
         {
            TESTER_ASSERT( plot[ n + 1 ].second < plot[ n ].second );
         } else {
            TESTER_ASSERT( plot[ n + 1 ].second > plot[ n ].second );
         }
      }
   }

   // test a real registration case (MR-MR, using powll, SSD)
   // then plot the similarity on the x-axis
   void testBasic()
   {
      std::cout << "loading volume..." << std::endl;
      Volumef volumeOrig;
      bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/MR-1.mf2", volumeOrig );
      ensure( loaded, "can't load volume" );
      volumeOrig.setSpacing( core::vector3f( 1, 1, 1 ) );

      std::cout << "volume size=" << volumeOrig.getSize() << std::endl;

      std::cout << "smoothing volume..." << std::endl;
      test::VolumeUtils::AverageFull( volumeOrig, 8 );

      std::cout << "registering volume..." << std::endl;
      const size_t joinHistogramNbBins = 32;
      const Volume volumeDiscrete = preprocess( volumeOrig, joinHistogramNbBins );

      Volume source = volumeDiscrete;
      Volume target = source;


      for ( size_t n = 1; n < 10; ++n )
      {
         srand( n * 10 );
         core::vector3f start( core::generateUniformDistributionf( -60, 60 ),
                               core::generateUniformDistributionf( -30, 30 ),
                               core::generateUniformDistributionf( -10, 10 ) );

         core::Buffer1D<double> seed( start.size() );
         for ( size_t n = 0; n < start.size(); ++n )
         {
            seed[ n ] = start[ n ];
         }


         // construct the evaluator
         algorithm::TransformationCreatorRigid c;
         algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
         algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
         RegistrationEvaluator evaluator( source, target, similarity, c, histogramMaker, joinHistogramNbBins );
         std::shared_ptr<imaging::Transformation> initTfm = c.create( seed );

         // run a registration
         algorithm::OptimizerPowell optimizer( 1, 0.05, 10 );
         RegistrationAlgorithmIntensity registration( c, evaluator, optimizer );
         std::shared_ptr<imaging::Transformation> tfm = registration.evaluate( source, target, *initTfm );
         core::Buffer1D<double> resultd = c.getParameters( *tfm );

         std::cout << "registration results (expected=(0,0,0)):" << std::endl;
         resultd.print( std::cout );
         std::cout << "similarity=" << evaluator.evaluate( *c.create( resultd ) ) << std::endl;
         TESTER_ASSERT( fabs( resultd[ 0 ] - 0.0 ) < source.getSpacing()[ 0 ] * 2 &&
                        fabs( resultd[ 1 ] - 0.0 ) < source.getSpacing()[ 1 ] * 2 &&
                        fabs( resultd[ 2 ] - 0.0 ) < source.getSpacing()[ 2 ] * 2 );
      }
   }

   void testRigidTransformationCreator()
   {
      for ( size_t n = 0; n < 100; ++n )
      {
         algorithm::TransformationCreatorRigid tfmCreator;

         const core::Buffer1D<double> params = core::make_buffer1D<double>( core::generateUniformDistributionf( -100, 100 ),
                                                                            core::generateUniformDistributionf( -100, 100 ),
                                                                            core::generateUniformDistributionf( -100, 100 ) );
         const std::shared_ptr<imaging::Transformation> tfm = tfmCreator.create( params );
         const core::Buffer1D<double> paramsBack = tfmCreator.getParameters( *tfm );
         TESTER_ASSERT( params.equal( paramsBack, 1e-3 ) );
      }
   }

   void testRange()
   {
      // data loading
      std::cout << "loading volume..." << std::endl;
      Volumef volumeOrig;
      bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/MR-1.mf2", volumeOrig );
      ensure( loaded, "can't load volume" );
      volumeOrig.setSpacing( core::vector3f( 1, 1, 1 ) );

      std::cout << "volume size=" << volumeOrig.getSize() << std::endl;

      std::cout << "smoothing volume..." << std::endl;
      test::VolumeUtils::AverageFull( volumeOrig, 8 );

      std::cout << "plotting similarity function..." << std::endl;
      const size_t joinHistogramNbBins = 32;
      const Volume volumeDiscrete = preprocess( volumeOrig, joinHistogramNbBins );

      Volume source = volumeDiscrete;
      Volume target = source;

      algorithm::TransformationCreatorRigid c;
      algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
      RegistrationEvaluator evaluator( source, target, similarity, c, histogramMaker, joinHistogramNbBins );
      RegistrationEvaluator::SimilarityPlot plot = evaluator.returnSimilarityAlongParameter( core::make_buffer1D<double>( 60,  0,  0 ), 0, -0.5, 150 );

      std::ofstream f( "c:/tmp2/similarity.txt" );
      ensure( f.good(), "bad!" );
      for ( ui32 n = 0; n < plot.size() - 1; ++n )
      {
         f << plot[ n ].first << " " << plot[ n ].second << std::endl;
         std::cout << "similarity=" << plot[ n ].second << " next=" << plot[ n + 1 ].second << std::endl;
         if ( plot[ n ].first > 0 )
         {
            TESTER_ASSERT( plot[ n ].second > plot[ n + 1 ].second );
         } else {
            TESTER_ASSERT( plot[ n ].second < plot[ n + 1 ].second );
         }
      }
   }

   void testPyramidalRegistration()
   {
      std::cout << "loading volume..." << std::endl;
      Volumef volumeOrig;
      bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/MR-1.mf2", volumeOrig );
      ensure( loaded, "can't load volume" );
      volumeOrig.setSpacing( core::vector3f( 1, 1, 1 ) );

      std::cout << "volume size=" << volumeOrig.getSize() << std::endl;

      std::cout << "smoothing volume..." << std::endl;
      test::VolumeUtils::AverageFull( volumeOrig, 8 );

      std::cout << "registering volume..." << std::endl;
      const size_t joinHistogramNbBins = 64;
      Volumef source = volumeOrig;
      Volumef target = volumeOrig;


      srand( 70 );

      std::cout << "pyramid registration:";
      std::cout.flush();
      for ( size_t n = 0; n < 10; ++n )
      {
         std::cout << "#";
         core::vector3f start( core::generateUniformDistributionf( -60, 60 ),
                                 core::generateUniformDistributionf( -30, 30 ),
                                 core::generateUniformDistributionf( -10, 10 ) );

         core::Buffer1D<double> seed( start.size() );
         for ( size_t n = 0; n < start.size(); ++n )
         {
            seed[ n ] = start[ n ];
         }


         // construct the evaluator
         algorithm::TransformationCreatorRigid c;
         algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
         algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
         RegistrationEvaluator evaluator( similarity, histogramMaker, joinHistogramNbBins );
         std::shared_ptr<imaging::Transformation> initTfm = c.create( seed );

         // run a registration
         algorithm::OptimizerPowell optimizer( 1, 0.001, 10 );
         RegistrationAlgorithmIntensity registration( c, evaluator, optimizer );

         // setup the pyramid
         typedef algorithm::VolumePreprocessorLut8bits<Volumef::value_type, Volumef::VoxelBuffer, Volume::VoxelBuffer> Preprocessor;
         typedef algorithm::RegistrationAlgorithmIntensityPyramid<Volumef::value_type, Volumef::VoxelBuffer> PyramidRegistration;

         imaging::LookUpTransformWindowingRGB lut( 0, 10000, joinHistogramNbBins, 1 );
         lut.createGreyscale( (float)joinHistogramNbBins );
         Preprocessor preprocessor( lut );
         PyramidRegistration pyramidRegistration( preprocessor, registration, 3 );

         std::shared_ptr<imaging::Transformation> tfm = pyramidRegistration.evaluate( source, target, *initTfm );
         core::Buffer1D<double> resultd = c.getParameters( *tfm );

         TESTER_ASSERT( fabs( resultd[ 0 ] - 0.0 ) < source.getSpacing()[ 0 ] * 2 &&
                        fabs( resultd[ 1 ] - 0.0 ) < source.getSpacing()[ 1 ] * 2 &&
                        fabs( resultd[ 2 ] - 0.0 ) < source.getSpacing()[ 2 ] * 2 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIntensityBasedRegistration);
 //TESTER_TEST(testSimilarity);
 //TESTER_TEST(testBasic);
 //TESTER_TEST(testRange);
 //TESTER_TEST(testEvaluatorSpecificData);
 //TESTER_TEST(testRigidTransformationCreator);
 TESTER_TEST(testPyramidalRegistration);
TESTER_TEST_SUITE_END();
#endif