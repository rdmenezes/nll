#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

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
    @brief Implementation of the registration evaluator using a similatity function criteria to maximize
    */
   template <class T, class Storage>
   class RegistrationEvaluatorSimilarity : public RegistrationEvaluator<T, Storage>
   {
   public:
      typedef RegistrationEvaluator<T, Storage>    Base;
      typedef typename Base::Volume                Volume;

   public:
      RegistrationEvaluatorSimilarity  ( const Volume& source, const Volume& target, const SimilarityFunction& similarity, const TransformationCreator& transformationCreator, size_t jointHistogramNbBins = 256 ) : Base( source, target ), _similarity( similarity ), _transformationCreator( transformationCreator ), _jointHistogramNbBins( jointHistogramNbBins )
      {}

      virtual double evaluate( const core::Buffer1D<f64>& parameters ) const
      {
         // create the actual transformation given the parameters
         std::shared_ptr<imaging::Transformation> transformation = _transformationCreator.create( parameters );
         ensure( transformation.get(), "can't be null!" );

         // then construct a joint histogram
         JointHistogram jointHistogram( _jointHistogramNbBins );
         computeHistogram_partialInterpolation( getSource(), *transformation, getTarget(), jointHistogram );

         // then run the similarity measure
         const double val = _similarity.evaluate( jointHistogram );
         std::cout << "f=" << val << std::endl;
         parameters.print( std::cout );
         return val;
      }

   protected:
      const SimilarityFunction&        _similarity;
      const TransformationCreator&     _transformationCreator;
      size_t                           _jointHistogramNbBins;
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

   static Volume preprocess( const Volumef& vf )
   {
      imaging::LookUpTransformWindowingRGB lut( -150, 150, 1 );
      lut.createGreyscale();
      Volume v( vf.size(), vf.getPst() );
      for ( size_t z = 0; z < v.getSize()[ 2 ]; ++z )
      {
         for ( size_t y = 0; y < v.getSize()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < v.getSize()[ 0 ]; ++x )
            {
               v( x, y, z ) = lut.transform( vf( x, y, z ) )[ 0 ];
            }
         }
      }

      return v;
   }

   void testBasic()
   {
      Volumef volumeOrig;
      bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/MR-1.mf2", volumeOrig );
      ensure( loaded, "can't load volume" );

      const Volume volumeDiscrete = preprocess( volumeOrig );

      const size_t joinHistogramNbBins = 256;
      const core::vector3ui size( 8, 8, 8);

      // construct the volumes
      /*
      Volume source( size, core::identityMatrix<core::Matrix<float>>( 4 ) );
      for ( Volume::iterator it = source.begin(); it != source.end(); ++it )
      {
         *it = rand() % joinHistogramNbBins;
      }*/

      Volume source = volumeDiscrete;
      Volume target = source;

      // construct the evaluator
      algorithm::TransformationCreatorRigid transformationCreator;
      algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      RegistrationEvaluator evaluator( source, target, similarity, transformationCreator, joinHistogramNbBins );

      // construct the optimizer
      algorithm::OptimizerPowell optimizer( 1, 0.1 );
      algorithm::ParameterOptimizers parameters;
      parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -200, 200, 0, 0.1, 0 ) );
      parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -200, 200, 0, 0.1, 0 ) );
      parameters.push_back( new algorithm::ParameterOptimizerGaussianLinear( -200, 200, 0, 0.1, 0 ) );

      std::vector<double> result = optimizer.optimize( evaluator, parameters );
      core::Buffer1D<double> resultd = core::make_buffer1D<double>( result );

      resultd.print( std::cout );
      std::cout << "similarity=" << evaluator.evaluate( resultd ) << std::endl;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIntensityBasedRegistration);
 TESTER_TEST(testBasic);
TESTER_TEST_SUITE_END();
#endif