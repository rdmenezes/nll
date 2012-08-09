#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"
#include "utils.h"

namespace nll
{
namespace algorithm
{
   /**
    @brief Transformation creator base class

    Create a transformation from a list of parameters
    */
   class TransformationCreator
   {
   public:
      virtual std::shared_ptr<imaging::Transformation> create( const nll::core::Buffer1D<nll::f64>& parameters ) const = 0;
      virtual ~TransformationCreator()
      {}
   };

   /**
    @brief Rigid Transformation
    
    It is modeled by 3 parameters (tx, ty, tz) such that:
        | 1 0 0 tx |
    T = | 0 1 0 ty |
        | 0 0 1 tz |
        | 0 0 0 1  |
    */
   class TransformationCreatorRigid : public TransformationCreator
   {
   public:
      virtual std::shared_ptr<imaging::Transformation> create( const nll::core::Buffer1D<nll::f64>& parameters ) const
      {
         ensure( parameters.size() == 3, "only (tx, ty, tz) parameters expected" );
         core::Matrix<float> tfmMat = core::identityMatrix< core::Matrix<float> >( 4 );
         tfmMat( 0, 3 ) = static_cast<float>( parameters[ 0 ] );
         tfmMat( 1, 3 ) = static_cast<float>( parameters[ 1 ] );
         tfmMat( 2, 3 ) = static_cast<float>( parameters[ 2 ] );

         std::shared_ptr<imaging::Transformation> tfm( new imaging::TransformationAffine( tfmMat ) );
         return tfm;
      }
   };

   /**
    @ingroup algorithm
    @brief Evaluate a registration base class
    */
   template <class T, class Storage>
   class RegistrationEvaluator : public core::NonCopyable, public OptimizerClient
   {
   public:
      typedef imaging::VolumeSpatial<T, Storage>   Volume;

   public:
      RegistrationEvaluator( const Volume& source, const Volume& target ) : _source( source ), _target( target )
      {}

      /**
       @brief Evaluate the transfomration source->target modeled with <parameters>
       */
      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const = 0;

      virtual ~RegistrationEvaluator()
      {}

      const Volume& getSource() const
      {
         return _source;
      }

      const Volume& getTarget() const
      {
         return _target;
      }

   protected:
      const Volume&                 _source;
      const Volume&                 _target;
   };

   /**
    @brief Class encapsulating the histogram creation mecanism
    */
   template <class T, class Storage>
   class HistogramMaker
   {
   public:
      typedef imaging::VolumeSpatial<T, Storage> Volume;

      virtual ~HistogramMaker()
      {}

      virtual void compute( const Volume& source, const imaging::Transformation& tfmSourceTarget, const Volume& target, JointHistogram& histogram ) const = 0;
   };

   template <class T, class Storage>
   class HistogramMakerNearestNeighbor : public HistogramMaker<T, Storage>
   {
   public:
      void compute( const Volume& source, const imaging::Transformation& tfmSourceTarget, const Volume& target, JointHistogram& histogram ) const 
      {
         algorithm::computeHistogram_nearestNeighbor( source, tfmSourceTarget, target, histogram );
      }
   };

   template <class T, class Storage>
   class HistogramMakerTrilinearPartial : public HistogramMaker<T, Storage>
   {
   public:
      void compute( const Volume& source, const imaging::Transformation& tfmSourceTarget, const Volume& target, JointHistogram& histogram ) const 
      {
         algorithm::computeHistogram_partialTrilinearInterpolation( source, tfmSourceTarget, target, histogram );
      }
   };

   /**
    @brief Implementation of the registration evaluator using a similatity function criteria to maximize
    */
   template <class T, class Storage>
   class RegistrationEvaluatorSimilarity : public RegistrationEvaluator<T, Storage>
   {
   public:
      typedef RegistrationEvaluator<T, Storage>          Base;
      typedef typename Base::Volume                      Volume;
      typedef std::vector< std::pair<double, double> >   SimilarityPlot;
      typedef HistogramMaker<T, Storage>                 HistogramMakerAlgorithm;

   public:
      RegistrationEvaluatorSimilarity  ( const Volume& source, const Volume& target, const SimilarityFunction& similarity, const TransformationCreator& transformationCreator, const HistogramMakerAlgorithm& histogramMaker, size_t jointHistogramNbBins = 256 ) : Base( source, target ), _similarity( similarity ), _transformationCreator( transformationCreator ), _histogramMaker( histogramMaker ), _jointHistogramNbBins( jointHistogramNbBins )
      {}

      /**
       @brief Evaluate the transformation parameters
       @note: the (0, 0) bin of the histogram is removed as we don't want to count the background (a lot of background matching doesn't mean the registration is good!)
       */
      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const
      {
         // create the actual transformation given the parameters
         std::shared_ptr<imaging::Transformation> transformation = _transformationCreator.create( parameters );
         ensure( transformation.get(), "can't be null!" );

         // then construct a joint histogram
         JointHistogram jointHistogram( _jointHistogramNbBins );
         _histogramMaker.compute( getSource(), *transformation, getTarget(), jointHistogram );

         // Remove the background intensity from the histogram. Reason: a lot of background match does not mean the registration is good.
         // It will biase the joint histogram measure
         const JointHistogram::value_type nbBackground = jointHistogram( 0, 0 );
         jointHistogram( 0, 0 ) = 0;
         jointHistogram.setNbSamples( static_cast<JointHistogram::value_type>( jointHistogram.getNbSamples() - nbBackground ) );

         // then run the similarity measure
         const double val = _similarity.evaluate( jointHistogram );

         _lastRunHistogram = jointHistogram;
         return val;
      }

      /**
       #brief Returns the last histogram computed
       */
      const JointHistogram& getLastRunHistogram() const
      {
         return _lastRunHistogram;
      }

      /**
       @brief Helper to print the similarity while varying only one parameter
       @return a list of pair containaing <parameter to vary, similarity>
       @note this is helpful to understand why a minimization algorithm might fail
       */
      SimilarityPlot returnSimilarityAlongParameter( const core::Buffer1D<f64>& startingParameters, size_t varyingParameter, double increment, size_t nbIterations ) const
      {
         SimilarityPlot result;
         result.reserve( nbIterations );

         const double start = startingParameters[ varyingParameter ];
         for ( size_t iter = 0; iter < nbIterations; ++iter )
         {
            core::Buffer1D<f64> p;
            p.clone( startingParameters );

            const double value = iter * increment + start;
            std::cout << "value=" << value << std::endl;
            p[ varyingParameter ] = value;
            const double similarity = evaluate( p );
            result.push_back( std::make_pair( value, similarity ) );
         }

         return result;
      }

   protected:
      const SimilarityFunction&        _similarity;
      const TransformationCreator&     _transformationCreator;
      const HistogramMakerAlgorithm&   _histogramMaker;
      size_t                           _jointHistogramNbBins;

      mutable JointHistogram           _lastRunHistogram;
   };


   /**
    @brief Helper class contructing a RegistrationEvaluator type given a spatial volume
    */
   template <class Volume>
   class RegistrationEvaluatorHelper
   {
   private:
      // not constructible!
      RegistrationEvaluatorHelper()
      {};

   public:
      typedef typename Volume::value_type    value_type;
      typedef typename Volume::VoxelBuffer   VoxelBuffer;

      typedef RegistrationEvaluatorSimilarity<value_type, VoxelBuffer>   EvaluatorSimilarity;
   };
}
}

using namespace nll;

class TestIntensityBasedRegistration
{
public:
   typedef imaging::VolumeSpatial<ui8>                                         Volume;
   typedef imaging::VolumeSpatial<f32>                                         Volumef;
   typedef algorithm::RegistrationEvaluatorHelper<Volume>::EvaluatorSimilarity RegistrationEvaluator;

   static Volume preprocess( const Volumef& vf, size_t joinHistogramNbBins )
   {
      imaging::LookUpTransformWindowingRGB lut( 0, 1000, joinHistogramNbBins, 1 );
      lut.createGreyscale( (float)joinHistogramNbBins );
      Volume v( vf.size(), vf.getPst() );
      for ( size_t z = 0; z < v.getSize()[ 2 ]; ++z )
      {
         for ( size_t y = 0; y < v.getSize()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < v.getSize()[ 0 ]; ++x )
            {
               const float voxel = vf( x, y, z );
               const float val = lut.transform( voxel )[ 0 ];
               v( x, y, z ) = (ui8)val;
            }
         }
      }

      return v;
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
               }
            }
         }
      }

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

      const double v0 = evaluator.evaluate( core::make_buffer1D<double>( 0, 0, 0 ) );
      const double v1 = evaluator.evaluate( core::make_buffer1D<double>( 0.1, 0, 0 ) );
      const double v2 = evaluator.evaluate( core::make_buffer1D<double>( -0.1, 0, 0 ) );

      TESTER_ASSERT( v0 < v1 );
      TESTER_ASSERT( v0 < v2 );

      for ( double n = -2; n < 2; n += 0.1 )
      {
         std::cout << "n=" << n << " v=" << evaluator.evaluate( core::make_buffer1D<double>( n, 0, 0 ) ) << std::endl;
         evaluator.getLastRunHistogram().print( std::cout );
      }

      
   }

   void testSimilarity()
   {
      const Volume v = createSyntheticCube( core::vector3ui( 64, 64, 64 ), core::vector3ui( 32, 32, 32 ), 16 );
      const size_t joinHistogramNbBins = 3;

      // construct the evaluator
      algorithm::TransformationCreatorRigid transformationCreator;
      algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
      RegistrationEvaluator evaluator( v, v, similarity, transformationCreator, histogramMaker, joinHistogramNbBins );

      // get the similarity. We expect it to be decreasing until we reach 0, then increasing

      RegistrationEvaluator::SimilarityPlot plot = evaluator.returnSimilarityAlongParameter( core::make_buffer1D<double>( 10,  0,  0 ), 0, -0.05, 200 );
      for ( size_t n = 0; n < plot.size() - 1; ++n )
      {
         TESTER_ASSERT( plot[ n + 1 ].second < plot[ n ].second );

         /*
         // NOTE: if we continue after 0 with trilinear interpolation partial, the error will increase but in a stepwise fashion.
         // this is caused by the interoplator!

         if ( plot[ n ].first > 0 )
         {
            TESTER_ASSERT( plot[ n + 1 ].second < plot[ n ].second );
         } else {
            TESTER_ASSERT( plot[ n + 1 ].second > plot[ n ].second );
         }*/
      }
   }

   void testBasic()
   {
      srand( 7 );
      Volumef volumeOrig;
      bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/MR-1.mf2", volumeOrig );
      ensure( loaded, "can't load volume" );
      volumeOrig.setSpacing( core::vector3f( 1, 1, 1 ) );

      test::VolumeUtils::AverageFull( volumeOrig, 12 );

      imaging::saveSimpleFlatFile( "c:/tmp2/v1.mf2", volumeOrig  );

      const size_t joinHistogramNbBins = 64;
      const Volume volumeDiscrete = preprocess( volumeOrig, joinHistogramNbBins );

      Volume source = volumeDiscrete;
      Volume target = source;

      // construct the evaluator
      algorithm::TransformationCreatorRigid transformationCreator;
      algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
      RegistrationEvaluator evaluator( source, target, similarity, transformationCreator, histogramMaker, joinHistogramNbBins );

      // construct the optimizer
      algorithm::OptimizerPowell optimizer( 1, 0.01, 10 );      
      algorithm::ParameterOptimizers parameters;
      parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );
      parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );
      parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -500, 500, 0, 300, 0 ) );

      // run a registration
      std::vector<double> result = optimizer.optimize( evaluator, parameters, core::make_buffer1D<double>( 60,  0,  0 ) );
      core::Buffer1D<double> resultd = core::make_buffer1D<double>( result );

      std::cout << "registration results (expected=(0,0,0)):" << std::endl;
      resultd.print( std::cout );
      std::cout << "similarity=" << evaluator.evaluate( resultd ) << std::endl;
      TESTER_ASSERT( fabs( result[ 0 ] - 0.0 ) < source.getSpacing()[ 0 ] * 1.1 &&
                     fabs( result[ 1 ] - 0.0 ) < source.getSpacing()[ 1 ] * 1.1 &&
                     fabs( result[ 2 ] - 0.0 ) < source.getSpacing()[ 2 ] * 1.1 );

      
      
      /*
      RegistrationEvaluator::SimilarityPlot plot = evaluator.returnSimilarityAlongParameter( core::make_buffer1D<double>( 1.05,  0,  0 ), 0, -0.001, 100 );

      std::ofstream f( "c:/tmp2/similarity.txt" );
      ensure( f.good(), "bad!" );
      for ( ui32 n = 0; n < plot.size(); ++n )
      {
         f << plot[ n ].first << " " << plot[ n ].second << std::endl;
         std::cout << "similarity=" << plot[ n ].second << std::endl;
      }*/
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIntensityBasedRegistration);
 TESTER_TEST(testBasic);
 TESTER_TEST(testSimilarity);
 TESTER_TEST(testEvaluatorSpecificData);
TESTER_TEST_SUITE_END();
#endif