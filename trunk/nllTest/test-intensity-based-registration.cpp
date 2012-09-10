#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"
#include "utils.h"

using namespace nll;

class TestIntensityBasedRegistration
{
public:
   typedef imaging::VolumeSpatial<ui8>                                         Volume;
   typedef imaging::VolumeSpatial<f32>                                         Volumef;
   typedef algorithm::RegistrationEvaluatorHelper<Volume>::EvaluatorSimilarity RegistrationEvaluator;
   typedef algorithm::RegistrationAlgorithmIntensity<Volume::value_type, Volume::VoxelBuffer> RegistrationAlgorithmIntensity;

   static Volume preprocess( const Volumef& vf, size_t joinHistogramNbBins, double min = 0, double max = 1000 )
   {
      imaging::LookUpTransformWindowingRGB lut( min, max, joinHistogramNbBins, 1 );
      lut.createGreyscale( (float)joinHistogramNbBins );
      algorithm::VolumePreprocessorLut8bits<Volumef::value_type, Volumef::VoxelBuffer, Volume::VoxelBuffer> preprocess( lut );
      

      return preprocess.run( vf );
   }

   void testEvaluatorSpecificData()
   {
      const size_t joinHistogramNbBins = 3;
      const core::vector3ui size( 4, 4, 4 );
      Volume v( size, core::identityMatrix< core::Matrix<float> >( 4 ) );
      v( 1, 1, 1 ) = 1;

      // construct the evaluator
      algorithm::TransformationCreatorTranslation transformationCreator;
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

      algorithm::TransformationCreatorTranslation c;
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

   void testBasicIntensityBasedRegistration()
   {
      srand( 0 );
      //
      // Load data
      //
      std::cout << "loading volume..." << std::endl;
      Volumef source;
      Volumef target;

      //bool loaded = imaging::loadSimpleFlatFile( "c:/tmp2/source.mf2", source );
      //loaded &= imaging::loadSimpleFlatFile( "c:/tmp2/target.mf2", target );
      
      //bool loaded = imaging::loadSimpleFlatFile( "c:/tmp2/source2.mf2", source );
      //loaded &= imaging::loadSimpleFlatFile( "c:/tmp2/target2.mf2", target );

      
      //bool loaded = imaging::loadSimpleFlatFile( "c:/tmp2/source3.mf2", source );
      //loaded &= imaging::loadSimpleFlatFile( "c:/tmp2/target3.mf2", target );

      //bool loaded = imaging::loadSimpleFlatFile( "c:/tmp2/source4.mf2", source );
      //loaded &= imaging::loadSimpleFlatFile( "c:/tmp2/target4.mf2", target );

      bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/mr-1h.mf2", source );
      loaded &= imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/mr-2h.mf2", target );
      ensure( loaded, "can't load volume" );

      //
      // prepare algorithm
      //
      typedef algorithm::VolumePreprocessorLut8bits<Volumef::value_type, Volumef::VoxelBuffer, Volume::VoxelBuffer> VolumePreprocessor;
      typedef algorithm::RegistrationAlgorithmIntensityAffineGeneric<VolumePreprocessor> RegistrationAlgorithm;

      const size_t joinHistogramNbBins = 16;
      imaging::LookUpTransformWindowingRGB lut( -300, 100, joinHistogramNbBins, 1 );
      lut.createGreyscale( (float)joinHistogramNbBins );
      VolumePreprocessor preprocessor( lut );

      //algorithm::SimilarityFunctionMutualInformation similarity;
      algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      RegistrationAlgorithm algorithm( preprocessor, preprocessor, similarity );

      std::shared_ptr<algorithm::TransformationParametrized> initTfm = algorithm::TransformationCreatorTranslation().create( core::make_buffer1D<double>( 0, 0, 0 ) );
      std::shared_ptr<algorithm::TransformationParametrized> tfm = algorithm.evaluate( source, target, *initTfm );

      //
      // Visualization of the result
      //
      imaging::LookUpTransformWindowingRGB lutDisplaySource( 0, joinHistogramNbBins, 256, 3 );
      float green[] = { 0, 255, 0 };
      lutDisplaySource.createColorScale( green );

      imaging::LookUpTransformWindowingRGB lutDisplayTarget( 0, joinHistogramNbBins, 256, 3 );
      float red[] = { 0, 0, 255 }; 
      lutDisplayTarget.createColorScale( red );

      const core::vector3f center = source.indexToPosition( core::vector3f( source.getSize()[ 0 ] / 2,
                                                                            source.getSize()[ 1 ] / 2,
                                       
                                                                            source.getSize()[ 2 ] / 2 ) );

      Volume sourcePreprocessed = preprocessor.run( source );
      Volume targetPreprocessed = preprocessor.run( target );

      // print the registration steps with the different transformation models. The last one counts.
      for ( size_t n = 0; n < algorithm.getDebugInfo().steps.size(); ++n )
      {
         {
            const imaging::TransformationAffine tfmAffine = dynamic_cast<imaging::TransformationAffine&>( *algorithm.getDebugInfo().steps[ n ].second );
            tfmAffine.print( std::cout );
            std::vector< core::Image<ui8> > mprs = test::visualizeRegistration( sourcePreprocessed, lutDisplaySource, targetPreprocessed, lutDisplayTarget, tfmAffine, center );
            core::writeBmp( mprs[ 0 ], "c:/tmp2/regx_step_" + core::val2str( n ) + ".bmp" );
            core::writeBmp( mprs[ 1 ], "c:/tmp2/regy_step_" + core::val2str( n ) + ".bmp" );
            core::writeBmp( mprs[ 2 ], "c:/tmp2/regz_step_" + core::val2str( n ) + ".bmp" );
         }
      }


      {
         const imaging::TransformationAffine tfmAffine = dynamic_cast<imaging::TransformationAffine&>( *initTfm );
         tfmAffine.print( std::cout );
         std::vector< core::Image<ui8> > mprs = test::visualizeRegistration( sourcePreprocessed, lutDisplaySource, targetPreprocessed, lutDisplayTarget, tfmAffine, center );
         core::writeBmp( mprs[ 0 ], "c:/tmp2/regxI_s.bmp" );
         core::writeBmp( mprs[ 1 ], "c:/tmp2/regyI_s.bmp" );
         core::writeBmp( mprs[ 2 ], "c:/tmp2/regzI_s.bmp" );
      }

      // if this test is failing, it doesn't mean the algo is wrong, just something changed. Check the registration result is correct!
      const core::Buffer1D<double> expectedParams = core::make_buffer1D<double>( -3.3387, -6.8691, 5.8666, 1.034, 1.0177, 0.9539 );
      std::cout << "RESULT=" << tfm->getParameters() << std::endl;
      TESTER_ASSERT( tfm->getParameters().size() == 6 ); // we have only translation + scaling here
      for ( int n = 0; n < 6; ++n )
         TESTER_ASSERT( core::equal<double>( tfm->getParameters()[ n ], expectedParams[ n ], 1e-2 ) ); // we have only translation + scaling here
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIntensityBasedRegistration);
 TESTER_TEST(testEvaluatorSpecificData);
 TESTER_TEST(testRange);
 TESTER_TEST(testBasicIntensityBasedRegistration);
TESTER_TEST_SUITE_END();
#endif