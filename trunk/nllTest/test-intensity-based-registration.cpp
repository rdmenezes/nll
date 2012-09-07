#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"
#include "utils.h"

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Generic registration algorithm

    There is no "generic" algorithm as such, but it is a good starting template to build your specific registration problem.
    */
   template <class VolumePreprocessor>
   class RegistrationIntensityBasedGenericAffine : public RegistrationAlgorithm<typename VolumePreprocessor::VolumeInput::value_type, typename VolumePreprocessor::VolumeInput::VoxelBuffer>
   {
   public:
      typedef RegistrationAlgorithm<typename VolumePreprocessor::VolumeInput::value_type, typename VolumePreprocessor::VolumeInput::VoxelBuffer>   Base;
      typedef typename VolumePreprocessor::VolumeInput                                                                                             VolumeInput;
      typedef typename VolumePreprocessor::VolumeOutput                                                                                            VolumeOutput;

   protected:
      typedef VolumePyramid<typename VolumeOutput::value_type, typename VolumeOutput::VoxelBuffer>                                                 Pyramid;
      typedef algorithm::HistogramMakerTrilinearPartial<typename VolumeOutput::value_type, typename VolumeOutput::VoxelBuffer>                     HistogramMaker;
      typedef typename algorithm::RegistrationEvaluatorHelper<VolumeOutput>::EvaluatorSimilarity                                                   RegistrationEvaluator;
      typedef algorithm::RegistrationAlgorithmIntensity<typename VolumeOutput::value_type, typename VolumeOutput::VoxelBuffer>                     RegistrationAlgorithmIntensity;
      typedef algorithm::RegistrationGradientEvaluatorFiniteDifference<typename VolumeOutput::value_type, typename VolumeOutput::VoxelBuffer>      GradientEvaluator;
      
   public:
      RegistrationIntensityBasedGenericAffine( const VolumePreprocessor& sourcePreprocessor,
                                               const VolumePreprocessor& targetPreprocessor,
                                               const SimilarityFunction& similarity ) : _sourcePreprocessor( sourcePreprocessor ), _targetPreprocessor( targetPreprocessor ), _similarity( similarity )
      {}

      // note: only the initial translation in the initial transformation
      virtual std::shared_ptr<TransformationParametrized> evaluate( const VolumeInput& source, const VolumeInput& target, const TransformationParametrized& source2TargetInitTransformation ) const
      {
         {
            std::stringstream ss;
            ss << "RegistrationIntensityBasedGenericAffine: " << std::endl
               << " source= size:" << source.getSize() << " spacing:" << source.getSpacing() << std::endl
               << " target= size:" << target.getSize() << " spacing:" << target.getSpacing() << std::endl
               << " initialTransformatiion=" << std::endl;
            source2TargetInitTransformation.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }


         // first, preprocess the volume so we have discretized volumes ready for the similarity measure
         const VolumeOutput sourcePreprocessed = _sourcePreprocessor.run( source );
         const VolumeOutput targetPreprocessed = _targetPreprocessor.run( target );

         // construct the pyramid
         // first determine the maximum level (i.e., we want to have as a last level a volume of size 20x20x20)
         // using very rough volumes help the registration algorithm jump the local minima. The registration will be
         // refined using the higher definition volume and will require typically few computations as it will already be positioned to
         // the global maximum (well, ideally!)
         const size_t maxSizeSource = *std::max_element( source.getSize().getBuf(), source.getSize().getBuf() + 3 );
         const size_t maxSizeTarget = *std::max_element( target.getSize().getBuf(), target.getSize().getBuf() + 3 );
         const size_t nbSourceLevels = getPyramidSize( maxSizeSource );
         const size_t nbTargetLevels = getPyramidSize( maxSizeTarget );

         Pyramid pyramidSource;
         pyramidSource.construct( sourcePreprocessed, nbSourceLevels );

         Pyramid pyramidTarget;
         pyramidTarget.construct( targetPreprocessed, nbTargetLevels );

         {
            std::stringstream ss;
            ss << " pyramidSourceNbLevels=" << pyramidSource.size() << std::endl
               << " pyramidTargetNbLevels=" << pyramidTarget.size();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // now we are progressively computing the registration starting from a very rough level of the pyramid,
         // and we also use transformation having the lowest degree of freedom, and progressively increase it
         std::shared_ptr<TransformationParametrized> result;
         {
            const VolumeOutput& sourcep = pyramidSource[ pyramidSource.size() - 1 ];
            const VolumeOutput& targetp = pyramidTarget[ pyramidTarget.size() - 1 ];
            const core::vector3f initialTranslation = getInitialTranslation( source, source2TargetInitTransformation );

            RegistrationAlgorithmResource resource = instanciateTranslationAlgorithm( _similarity );
            const std::shared_ptr<TransformationParametrized> tfmInit( resource.transformationCreator->create( core::make_buffer1D<double>( initialTranslation[ 0 ], initialTranslation[ 1 ], initialTranslation[ 2 ] ) ) );

            {
               std::stringstream ss;
               ss << " TranslationAlgorithm:" << std::endl
                  << "  level=" << pyramidSource.size() - 1 << std::endl
                  << "  initialTransofrmation=" << std::endl;
               tfmInit->print( ss );
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }

            result = resource.registrationAlgorithm->evaluate( sourcep, targetp, *tfmInit );

            {
               std::stringstream ss;
               ss << " TranslationAlgorithm result:" << pyramidSource.size() << std::endl;
               result->print( ss );
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }
         }

         {
            std::stringstream ss;
            ss << "RegistrationIntensityBasedGenericAffine Final transformation: " << std::endl;
            result->print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         return result;
      }

   protected:
      static core::vector3f getInitialTranslation( const VolumeInput& source, const TransformationParametrized& source2TargetInitTransformation )
      {
         const core::vector3f sourceCenter = source.indexToPosition( core::vector3f( source.getSize()[ 0 ] / 2,
                                                                                     source.getSize()[ 1 ] / 2,
                                                                                     source.getSize()[ 2 ] / 2 ) );

         const core::vector3f sourceCenterTransformed = source2TargetInitTransformation.transform( sourceCenter );
         return core::vector3f( sourceCenterTransformed[ 0 ] - sourceCenter[ 0 ],
                                sourceCenterTransformed[ 1 ] - sourceCenter[ 1 ],
                                sourceCenterTransformed[ 2 ] - sourceCenter[ 2 ] );
      }

   public:
      struct RegistrationAlgorithmResource
      {
         std::shared_ptr<TransformationCreator>          transformationCreator;
         std::shared_ptr<HistogramMaker>                 histogramMaker;
         std::shared_ptr<RegistrationEvaluator>          registrationEvaluator;
         std::shared_ptr<StopCondition>                  optimizationStopCondition;
         std::shared_ptr<Optimizer>                      optimizer;
         std::shared_ptr<RegistrationAlgorithmIntensity> registrationAlgorithm;
      };

      /**
       @brief Construct a translation algorithm
       */
      static RegistrationAlgorithmResource instanciateTranslationAlgorithm( const SimilarityFunction& similarity ) 
      {
         const size_t joinHistogramNbBins = 16;
         std::shared_ptr<GradientEvaluator> gradientEvaluator( new GradientEvaluator( core::make_buffer1D<double>( 0.1, 0.1, 0.1 ), true ) );

         RegistrationAlgorithmResource algoRes;
         algoRes.transformationCreator = std::shared_ptr<TransformationCreator>( new TransformationCreatorTranslation() );
         algoRes.histogramMaker        = std::shared_ptr<HistogramMaker>( new HistogramMaker() );
         algoRes.registrationEvaluator = std::shared_ptr<RegistrationEvaluator>( new RegistrationEvaluator( similarity, *algoRes.histogramMaker, joinHistogramNbBins, gradientEvaluator, false ) );

         algoRes.optimizationStopCondition = std::shared_ptr<StopCondition>( new algorithm::StopConditionStable( 15 ) );
         algoRes.optimizer = std::shared_ptr<Optimizer>( new algorithm::OptimizerGradientDescent( *algoRes.optimizationStopCondition, 0.0,
                                                                                                  true,
                                                                                                  1,
                                                                                                  core::make_buffer1D<double>( 4, 4, 4 ),
                                                                                                  core::make_buffer1D<double>( 1, 1, 1 ) ) );
         algoRes.registrationAlgorithm = std::shared_ptr<RegistrationAlgorithmIntensity>( new RegistrationAlgorithmIntensity( *algoRes.transformationCreator,
                                                                                                                              *algoRes.registrationEvaluator,
                                                                                                                              *algoRes.optimizer ) );
         return algoRes;
      }

   protected:
      // we want size / 2^level <= 20 => level >= ln( sizeInVoxel ) / ( ln( 2 ) * ln( 20 ) )
      static size_t getPyramidSize( const size_t sizeInVoxel )
      {
         const static double maxSize = 20;
         return core::round( core::log2( static_cast<double>( sizeInVoxel ) ) / ( core::log2( 2.0 ) * core::log2( 20.0 ) ) );
      }

   protected:
      const VolumePreprocessor& _sourcePreprocessor;
      const VolumePreprocessor& _targetPreprocessor;
      const SimilarityFunction& _similarity;
   };
}
}

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

   static Volume preprocess( const Volumef& vf, size_t joinHistogramNbBins, double min = 0, double max = 1000 )
   {
      imaging::LookUpTransformWindowingRGB lut( min, max, joinHistogramNbBins, 1 );
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
         algorithm::TransformationCreatorTranslation c;
         algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
         algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
         RegistrationEvaluator evaluator( source, target, similarity, c, histogramMaker, joinHistogramNbBins );
         std::shared_ptr<algorithm::TransformationParametrized> initTfm = c.create( seed );

         // run a registration
         algorithm::OptimizerPowell optimizer( 1, 0.05, 10 );
         RegistrationAlgorithmIntensity registration( c, evaluator, optimizer );
         std::shared_ptr<algorithm::TransformationParametrized> tfm = registration.evaluate( source, target, *initTfm );
         core::Buffer1D<double> resultd = tfm->getParameters();

         std::cout << "registration results (expected=(0,0,0)):" << std::endl;
         resultd.print( std::cout );
         std::cout << "similarity=" << evaluator.evaluate( *c.create( resultd ) ) << std::endl;
         TESTER_ASSERT( fabs( resultd[ 0 ] - 0.0 ) < source.getSpacing()[ 0 ] * 2 &&
                        fabs( resultd[ 1 ] - 0.0 ) < source.getSpacing()[ 1 ] * 2 &&
                        fabs( resultd[ 2 ] - 0.0 ) < source.getSpacing()[ 2 ] * 2 );
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

   void testRegistrationGradient()
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

      const Volume volumeDiscrete = preprocess( volumeOrig, joinHistogramNbBins );
      Volume source = volumeDiscrete;
      Volume target = source;

      std::cout << "pyramid registration:";
      std::cout.flush();
      for ( size_t n = 0; n < 10; ++n )
      {
         srand( n * 10 );
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
         algorithm::TransformationCreatorTranslation c;
         algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
         algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
         typedef algorithm::RegistrationGradientEvaluatorFiniteDifference<Volume::value_type, Volume::VoxelBuffer> GradientEvaluator;
         std::shared_ptr<GradientEvaluator> gradientEvaluator( new GradientEvaluator( core::make_buffer1D<double>( 0.1, 0.1, 0.1 ) ) );
         RegistrationEvaluator evaluator( similarity, histogramMaker, joinHistogramNbBins, gradientEvaluator );
         std::shared_ptr<algorithm::TransformationParametrized> initTfm = c.create( seed );

         // run a registration
         algorithm::StopConditionIteration stopCondition( 50 );
         algorithm::OptimizerGradientDescent optimizer( stopCondition, 4.0 );

         RegistrationAlgorithmIntensity registration( c, evaluator, optimizer );
         std::shared_ptr<algorithm::TransformationParametrized> tfm = registration.evaluate( source, target, *initTfm );
         core::Buffer1D<double> resultd = tfm->getParameters();

         TESTER_ASSERT( fabs( resultd[ 0 ] - 0.0 ) < source.getSpacing()[ 0 ] * 2 &&
                        fabs( resultd[ 1 ] - 0.0 ) < source.getSpacing()[ 1 ] * 2 &&
                        fabs( resultd[ 2 ] - 0.0 ) < source.getSpacing()[ 2 ] * 2 );
      }
   }
   
   void testRegistrationRotationGradient()
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

      const Volume volumeDiscrete = preprocess( volumeOrig, joinHistogramNbBins );
      Volume source = volumeDiscrete;
      Volume target = source;

      std::cout << "pyramid registration:";
      std::cout.flush();
      for ( size_t n = 0; n < 5; ++n )
      {
         srand( n * 10 );
         std::cout << "#";
         core::Buffer1D<double> seed = core::make_buffer1D<double>( core::generateUniformDistributionf( -20, 20 ),
                                                                    core::generateUniformDistributionf( -20, 20 ),
                                                                    core::generateUniformDistributionf( -10, 10 ),
                                                                    core::generateUniformDistributionf( -0.1, 0.1 ),
                                                                    core::generateUniformDistributionf( -0.1, 0.1 ),
                                                                    core::generateUniformDistributionf( -0.1, 0.1 ) );// */ 20, -5, -10, 0.2, 0.15, 0 );
         


         // construct the evaluator
         algorithm::TransformationCreatorRigid c;
         algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
         algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
         typedef algorithm::RegistrationGradientEvaluatorFiniteDifference<Volume::value_type, Volume::VoxelBuffer> GradientEvaluator;
         std::shared_ptr<GradientEvaluator> gradientEvaluator( new GradientEvaluator( core::make_buffer1D<double>( 0.1, 0.1, 0.1, 0.001, 0.001, 0.001 ), false ) );
         RegistrationEvaluator evaluator( similarity, histogramMaker, joinHistogramNbBins, gradientEvaluator );
         std::shared_ptr<algorithm::TransformationParametrized> initTfm = c.create( seed );

         initTfm->print( std::cout );

         // run a registration: note this can be tricky here if the parameters are not set correctly... e.g., learningRate is too high, we will loop on local minima
         algorithm::StopConditionStable stopCondition( 5 );
         algorithm::OptimizerGradientDescent optimizer( stopCondition, 0.0, true, 1, core::make_buffer1D<double>( 1, 1, 1, 0.005, 0.005, 0.005 ),
                                                                                     core::make_buffer1D<double>( 0.1, 0.1, 0.1, 0.00001, 0.00001, 0.00001 ) );

         RegistrationAlgorithmIntensity registration( c, evaluator, optimizer );

         std::shared_ptr<algorithm::TransformationParametrized> tfm = registration.evaluate( source, target, *initTfm );
         core::Buffer1D<double> resultd = tfm->getParameters();

         // the translational part
         TESTER_ASSERT( fabs( resultd[ 0 ] - 0.0 ) < source.getSpacing()[ 0 ] * 2 &&
                        fabs( resultd[ 1 ] - 0.0 ) < source.getSpacing()[ 1 ] * 2 &&
                        fabs( resultd[ 2 ] - 0.0 ) < source.getSpacing()[ 2 ] * 2 );

         // the rotational part
         TESTER_ASSERT( fabs( resultd[ 3 ] - 0.0 ) < 0.05 &&
                        fabs( resultd[ 4 ] - 0.0 ) < 0.05 &&
                        fabs( resultd[ 5 ] - 0.0 ) < 0.05 );
      }
   }

   template <class Volume>
   core::Image<ui8> GetMprForDisplay( const Volume& volume,
                                      const core::vector3ui& size,
                                      const core::vector3f& axisX,
                                      const core::vector3f& axisY,
                                      const core::vector3f& origin,
                                      const imaging::LookUpTransformWindowingRGB& lut )
   {
      typedef typename Volume::value_type             value_type;
      typedef imaging::InterpolatorTriLinear<Volume>  Interpolator;
      typedef imaging::Slice<value_type>              Slice;

      ensure( lut.getNbComponents() == 3, "we need a RGB LUT" );

      core::vector2f spacing( axisX.norm2(), axisY.norm2() );
      imaging::Slice<float> slice( size,
                                   axisX,
                                   axisY,
                                   origin, 
                                   spacing );
      imaging::Mpr<Volume, Interpolator> mpr( volume );
      mpr.getSlice( slice );


      core::Image<ui8> slicei( slice.sizex(), slice.sizey(), 3 );
      for ( size_t y = 0; y < slicei.sizey(); ++y )
      {
         for ( size_t x = 0; x < slicei.sizex(); ++x )
         {
            const float* val = lut.transform( slice( x, y, 0 ) );
            slicei( x, y, 0 ) = (ui8)NLL_BOUND( val[ 0 ], 0, 255 );
            slicei( x, y, 1 ) = (ui8)NLL_BOUND( val[ 1 ], 0, 255 );
            slicei( x, y, 2 ) = (ui8)NLL_BOUND( val[ 2 ], 0, 255 );
         }
      }

      return slicei;
   }

   static Volumef convert( const Volume& v )
   {
      Volumef r( v.getSize(), v.getPst() );
      for ( size_t z = 0; z < v.sizez(); ++z )
      {
         for ( size_t y = 0; y < v.sizey(); ++y )
         {
            for ( size_t x = 0; x < v.sizex(); ++x )
            {
               r( x, y, z ) = v( x, y, z );
            }
         }
      }

      return r;
   }

   



   void testRealMr()
   {
      std::cout << "loading volume..." << std::endl;
      Volumef volumeOrig;
      Volumef volumeOrig2;
      /*
      bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/mr-1h.mf2", volumeOrig );
      loaded &= imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/mr-2h.mf2", volumeOrig2 );
      */
      bool loaded = imaging::loadSimpleFlatFile( "c:/tmp2/source.mf2", volumeOrig );
      loaded &= imaging::loadSimpleFlatFile( "c:/tmp2/target.mf2", volumeOrig2 );
      ensure( loaded, "can't load volume" );

      std::cout << "volume size=" << volumeOrig.getSize() << std::endl;
      std::cout << "volume size2=" << volumeOrig2.getSize() << std::endl;

      std::cout << "smoothing volume..." << std::endl;
      




      std::cout << "registering volume..." << std::endl;
      const size_t joinHistogramNbBins = 16;

      const Volume volumeDiscrete = preprocess( volumeOrig, joinHistogramNbBins - 1, -100, 100 );
      const Volume volumeDiscrete2 = preprocess( volumeOrig2, joinHistogramNbBins - 1, -100, 100 );

      imaging::LookUpTransformWindowingRGB lut( 0, joinHistogramNbBins, 256, 3 );
      float green[] = { 0, 255, 0 };
      lut.createColorScale( green );
      //lut.createGreyscale();

      imaging::LookUpTransformWindowingRGB lutTarget( 0, joinHistogramNbBins, 256, 3 );
      float red[] = { 0, 0, 255 };
      lutTarget.createColorScale( red );

      core::Image<ui8> slicei = GetMprForDisplay( volumeDiscrete,
                                                  core::vector3ui( 512, 512, 1 ),
                                                  core::vector3f( 1, 0, 0 ),
                                                  core::vector3f( 0, 1, 0 ),
                                                  volumeOrig.getOrigin(), lut );
      core::writeBmp( slicei, "c:/tmp2/mpr.bmp" );
      slicei = GetMprForDisplay( volumeDiscrete2,
                                 core::vector3ui( 512, 512, 1 ),
                                 core::vector3f( 1, 0, 0 ),
                                 core::vector3f( 0, 1, 0 ),
                                 volumeOrig2.getOrigin(), lut );
      core::writeBmp( slicei, "c:/tmp2/mpr2.bmp" );
      //return;

      algorithm::VolumePyramid<Volume::value_type, Volume::VoxelBuffer> pyramid1( volumeDiscrete, 10 );
      algorithm::VolumePyramid<Volume::value_type, Volume::VoxelBuffer> pyramid2( volumeDiscrete2, 10 );

      for ( size_t n = 1; n < pyramid1.size(); ++n )
      {
         imaging::saveSimpleFlatFile( "c:/tmp2/pyr-" + core::val2str( n ) + ".mf2", convert( pyramid1[ n ] ) );
      }

      Volume source = pyramid1[ 8 ];
      Volume target = pyramid2[ 8 ];

      //core::Buffer1D<double> seed = core::make_buffer1D<double>( 20, 10, 0, 1, 1, 1 );
      core::Buffer1D<double> seed = core::make_buffer1D<double>( 30, 0, 0 );
         


      // construct the evaluator
      //algorithm::TransformationCreatorRigid c;
      algorithm::TransformationCreatorTranslation c;
      //algorithm::TransformationCreatorTranslationScaling c;
      //algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      algorithm::SimilarityFunctionMutualInformation similarity;
      algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
      typedef algorithm::RegistrationGradientEvaluatorFiniteDifference<Volume::value_type, Volume::VoxelBuffer> GradientEvaluator;
      //std::shared_ptr<GradientEvaluator> gradientEvaluator( new GradientEvaluator( core::make_buffer1D<double>( 0.1, 0.1, 0.1, 0.01, 0.01, 0.01 ), false ) );
      std::shared_ptr<GradientEvaluator> gradientEvaluator( new GradientEvaluator( core::make_buffer1D<double>( 0.1, 0.1, 0.1 ), true ) );
      RegistrationEvaluator evaluator( similarity, histogramMaker, joinHistogramNbBins, gradientEvaluator, false );
      
      std::shared_ptr<algorithm::TransformationParametrized> initTfm = c.create( seed );

      initTfm->print( std::cout );
      
      // run a registration: note this can be tricky here if the parameters are not set correctly... e.g., learningRate is too high, we will loop on local minima
      algorithm::StopConditionStable stopCondition( 10 );
      //algorithm::StopConditionIteration stopCondition( 500 );
      algorithm::OptimizerGradientDescent optimizer( stopCondition, 0.0, true, 1, core::make_buffer1D<double>( 4, 4, 4 ),
                                                                                    core::make_buffer1D<double>( 1, 1, 1 ) );
      //algorithm::OptimizerGradientDescent optimizer( stopCondition, 0.0, true, 1, core::make_buffer1D<double>( 2, 2, 2, 0.031, 0.031, 0.031 ),
      //                                                                            core::make_buffer1D<double>( 100, 100, 100, 0.1, 0.1, 0.1 ) );

      //algorithm::OptimizerPowell optimizer;

      RegistrationAlgorithmIntensity registration( c, evaluator, optimizer );

      std::shared_ptr<algorithm::TransformationParametrized> tfm = registration.evaluate( source, target, *initTfm );
      core::Buffer1D<double> resultd = tfm->getParameters();

      const core::vector3f center = source.indexToPosition( core::vector3f( source.getSize()[ 0 ] / 2,
                                                                            source.getSize()[ 1 ] / 2,
                                                                            source.getSize()[ 2 ] / 2 ) );

      {
         const imaging::TransformationAffine tfmAffine = dynamic_cast<imaging::TransformationAffine&>( *tfm );
         tfmAffine.print( std::cout );
         std::vector< core::Image<ui8> > mprs = test::visualizeRegistration( source, lut, target, lutTarget, tfmAffine, center );
         core::writeBmp( mprs[ 0 ], "c:/tmp2/regx_s.bmp" );
         core::writeBmp( mprs[ 1 ], "c:/tmp2/regy_s.bmp" );
         core::writeBmp( mprs[ 2 ], "c:/tmp2/regz_s.bmp" );
      }


      {
         const imaging::TransformationAffine tfmAffine = dynamic_cast<imaging::TransformationAffine&>( *initTfm );
         tfmAffine.print( std::cout );
         std::vector< core::Image<ui8> > mprs = test::visualizeRegistration( source, lut, target, lutTarget, tfmAffine, center );
         core::writeBmp( mprs[ 0 ], "c:/tmp2/regxI_s.bmp" );
         core::writeBmp( mprs[ 1 ], "c:/tmp2/regyI_s.bmp" );
         core::writeBmp( mprs[ 2 ], "c:/tmp2/regzI_s.bmp" );
      }
      


      /*
      evaluator.setTransformationCreator( c );
      evaluator.setSource( source );
      evaluator.setTarget( target );
      RegistrationEvaluator::SimilarityPlot plot = evaluator.returnSimilarityAlongParameter( core::make_buffer1D<double>( 30, 0, 0 ), 0, -0.5, 60 );
      std::ofstream f( "c:/tmp2/vat.txt" );
      for ( size_t n = 0; n < plot.size(); ++n )
      {
         f << plot[ n ].first << " " << plot[ n ].second << std::endl;
      }*/
   }

   void testBasicIntensityBasedRegistration()
   {
      //
      // Load data
      //
      std::cout << "loading volume..." << std::endl;
      Volumef source;
      Volumef target;

      bool loaded = imaging::loadSimpleFlatFile( "c:/tmp2/source.mf2", source );
      loaded &= imaging::loadSimpleFlatFile( "c:/tmp2/target.mf2", target );
      ensure( loaded, "can't load volume" );

      //
      // prepare algorithm
      //
      typedef algorithm::VolumePreprocessorLut8bits<Volumef::value_type, Volumef::VoxelBuffer, Volume::VoxelBuffer> VolumePreprocessor;
      typedef algorithm::RegistrationIntensityBasedGenericAffine<VolumePreprocessor> RegistrationAlgorithm;

      const size_t joinHistogramNbBins = 16;
      imaging::LookUpTransformWindowingRGB lut( -100, 100, joinHistogramNbBins, 1 );
      lut.createGreyscale( (float)joinHistogramNbBins );
      VolumePreprocessor preprocessor( lut );

      algorithm::SimilarityFunctionMutualInformation similarity;
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

      {
         const imaging::TransformationAffine tfmAffine = dynamic_cast<imaging::TransformationAffine&>( *tfm );
         tfmAffine.print( std::cout );
         std::vector< core::Image<ui8> > mprs = test::visualizeRegistration( source, lutDisplaySource, target, lutDisplayTarget, tfmAffine, center );
         core::writeBmp( mprs[ 0 ], "c:/tmp2/regx_s.bmp" );
         core::writeBmp( mprs[ 1 ], "c:/tmp2/regy_s.bmp" );
         core::writeBmp( mprs[ 2 ], "c:/tmp2/regz_s.bmp" );
      }


      {
         const imaging::TransformationAffine tfmAffine = dynamic_cast<imaging::TransformationAffine&>( *initTfm );
         tfmAffine.print( std::cout );
         std::vector< core::Image<ui8> > mprs = test::visualizeRegistration( source, lutDisplaySource, target, lutDisplayTarget, tfmAffine, center );
         core::writeBmp( mprs[ 0 ], "c:/tmp2/regxI_s.bmp" );
         core::writeBmp( mprs[ 1 ], "c:/tmp2/regyI_s.bmp" );
         core::writeBmp( mprs[ 2 ], "c:/tmp2/regzI_s.bmp" );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIntensityBasedRegistration);
/*
 TESTER_TEST(testBasic);
 TESTER_TEST(testRange);
 TESTER_TEST(testEvaluatorSpecificData);
 TESTER_TEST(testPyramidalRegistration);
 TESTER_TEST(testRegistrationGradient);
 //TESTER_TEST(testRegistrationRotationGradient);
 */
 //TESTER_TEST(testRealMr);
TESTER_TEST(testBasicIntensityBasedRegistration);
TESTER_TEST_SUITE_END();
#endif