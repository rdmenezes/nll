/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_ALGORITHM_REGISTRATION_ALGORITHM_INTENSITY_AFFINE_H_
# define NLL_ALGORITHM_REGISTRATION_ALGORITHM_INTENSITY_AFFINE_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief 3D/3D Registration algorithm based on the intensity overlap of the two volumes,
           this is measured by a cost function which is minimized

    This is usually producing a good registration while used in a multi-resolution approach.
    It can however be traped in a local minima and so a good initialization with sufficient volume
    overlap is necessary.

    In this registration scheme, it is assumed the volumes are already preprocessed (i.e., using a VolumePreprocessor)
    and discretized (i.e., the cost function will be directly run on the volumes pdf)

    The algorithm is as follow:
    - start from the seed
    - compute the joint histogram
    - compute the similarity from the JH (using the provided similarity function)
    - find the next step (using the optimizer)
    - repeat

    To diagnose registration problems, it is useful to check the joint histogram. On some synthetic cases & powell
    wrong solutions where found due to large transformation making the volumes not in the joint histogram anymore (and so achieving perfect match!)

    Other problems can be due to the gradient optimizer, where it is too low/high. It depends on the similarity measure used and the number of bins of the
    joint histogram.
    */
   template <class DiscreteType, class DiscreteStorage>
   class RegistrationAlgorithmIntensity : public RegistrationAlgorithm<DiscreteType, DiscreteStorage>, public core::NonAssignable
   {
   public:
      typedef RegistrationAlgorithm<DiscreteType, DiscreteStorage>      Base;
      typedef typename Base::Volume                                     Volume;
      typedef typename Base::Matrix                                     Matrix;
      typedef RegistrationEvaluator<DiscreteType, DiscreteStorage>      Evaluator;

   public:
      RegistrationAlgorithmIntensity( const TransformationCreator& creator, Evaluator& evaluator, Optimizer& optimizer ) : _creator( creator ), _evaluator( evaluator ), _optimizer( optimizer )
      {}

      /**
       @brief Run the optimization process
       @note the parameter source2TargetInitTransformation must be of the same class as what of the evaluator (i.e., rigid registration
             if the evaluator is using rigid, else the initial transformation will not be fully used... TODO prepending transformation as a work around?
       */
      virtual std::shared_ptr<TransformationParametrized> evaluate( const Volume& source, const Volume& target, const TransformationParametrized& source2TargetInitTransformation ) const
      {
         {
            std::stringstream ss;
            ss << "RegistrationAlgorithmIntensity::evaluate" << std::endl
               << "source:";
            source.print( ss );
            ss << std::endl << " target:";
            target.print( ss );
            ss << "initialTransformation=" << std::endl;
            source2TargetInitTransformation.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         _evaluator.setSource( source );
         _evaluator.setTarget( target );
         _evaluator.setTransformationCreator( _creator );
         core::Buffer1D<double> seed = source2TargetInitTransformation.getParameters();
         core::Buffer1D<double> result = _optimizer.optimize( _evaluator, _creator.getOptimizerParameters(), seed );
         std::shared_ptr<TransformationParametrized> tfm = _creator.create( result );

         {
            std::stringstream ss;
            ss << "transformation found:" << std::endl;
            tfm->print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }
         return tfm;
      }

      const TransformationCreator& getTransformationCreator() const
      {
         return _creator;
      }

   protected:
      const TransformationCreator&  _creator;
      Evaluator&                    _evaluator;
      Optimizer&                    _optimizer;
   };

   /**
    @ingroup algorithm
    @brief Affine registration algorithm embeded in a multi resolution scheme
           
           A pyramid is constructed for both volumes which are registered by the given algorithm on each level of
           the pyramid from the highest to the lowest. This done to improve the robustness of the algorithm (less local minima)
           and to speed up the algorithm (i.e., the lowest levels are already registered and only a small transformation will be
           required)
    */
   template <class T, class Storage>
   class RegistrationAlgorithmIntensityPyramid : public RegistrationAlgorithm<T, Storage>, public core::NonAssignable
   {
   public:
      typedef RegistrationAlgorithm<T, Storage>       Base;
      typedef typename Base::Volume                   Volume;
      typedef typename Base::Matrix                   Matrix;
      typedef VolumePyramid<T, Storage>               Pyramid;
      
   public:
      typedef RegistrationEvaluator<T, Storage >        Evaluator;
      typedef RegistrationAlgorithm<T, Storage>         RegistrationAlgorithmInternal;

   public:
      /**
       @param RegistrationAlgorithmInternal the registration algorithm operating on the discretized volumes
       @param pyramidNbLevels the number of levels in the pyramid (i.e., the granularity of the registration )
       */
      RegistrationAlgorithmIntensityPyramid( const RegistrationAlgorithmInternal& registrationAlgorithm, size_t pyramidNbLevels = 4 ) : _registrationAlgorithm( registrationAlgorithm ), _pyramidNbLevels( pyramidNbLevels )
      {
         ensure( pyramidNbLevels >= 1, "wrong number of levels" );
      }

      /**
       @brief Run the optimization process
       */
      virtual std::shared_ptr<TransformationParametrized> evaluate( const Volume& sourceSampled, const Volume& targetSampled, const TransformationParametrized& source2TargetInitTransformation ) const
      {
         {
            std::stringstream ss;
            ss << "RegistrationAlgorithmIntensityPyramid::evaluate" << std::endl
               << "nbLevels=" << _pyramidNbLevels
               << "source:";
            sourceSampled.print( ss );
            ss << std::endl << " target:";
            targetSampled.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         Pyramid sourceSampledPyramid;
         sourceSampledPyramid.construct( sourceSampled, _pyramidNbLevels );

         Pyramid targetSampledPyramid;
         targetSampledPyramid.construct( targetSampled, _pyramidNbLevels );

         ensure( targetSampledPyramid.size() == _pyramidNbLevels, "unexpected number og levels" );
         ensure( sourceSampledPyramid.size() == _pyramidNbLevels, "unexpected number og levels" );

         std::shared_ptr<TransformationParametrized> tfm;
         for ( int level = -1 + (int)_pyramidNbLevels; level >= 0; --level )
         {
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "pyramid level=" + core::val2str( level ) );
            const TransformationParametrized* tfmToUse = ( level + 1 == (int)_pyramidNbLevels ) ? &source2TargetInitTransformation : tfm.get();
            ensure( tfmToUse, "registration problem! Wrong initial transformation" );
            tfm   = _registrationAlgorithm.evaluate( sourceSampledPyramid[ level ],
                                                     targetSampledPyramid[ level ],
                                                     *tfmToUse );
         }

         {
            std::stringstream ss;
            ss << "transformation found:" << std::endl;
            tfm->print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         return tfm;
      }

   protected:
      size_t                                 _pyramidNbLevels;
      const RegistrationAlgorithmInternal&   _registrationAlgorithm;
   };

   /**
    @ingroup algorithm
    @brief Generic registration algorithm

    There is no "generic" algorithm as such, but it is a good starting template to build your specific registration problem solver. Mostly tested with CT-CT registration
    */
   template <class VolumePreprocessor>
   class RegistrationAlgorithmIntensityAffineGeneric : public core::NonAssignable, public RegistrationAlgorithm<typename VolumePreprocessor::VolumeInput::value_type, typename VolumePreprocessor::VolumeInput::VoxelBuffer>
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
      struct RegistrationAlgorithmResource
      {
         std::shared_ptr<TransformationCreator>          transformationCreator;
         std::shared_ptr<HistogramMaker>                 histogramMaker;
         std::shared_ptr<RegistrationEvaluator>          registrationEvaluator;
         std::shared_ptr<StopCondition>                  optimizationStopCondition;
         std::shared_ptr<Optimizer>                      optimizer;
         std::shared_ptr<RegistrationAlgorithmIntensity> registrationAlgorithm;
         std::string                                     algorithmClassIdentifier;     // used for logging
      };

      struct DebugInfo
      {
         typedef std::pair< RegistrationAlgorithmResource, std::shared_ptr<TransformationParametrized> > Step;
         typedef std::vector<Step> Steps;

         Steps steps;
      };

   public:
      /**
       @brief Construct a generic affine registration
       @param sourcePreprocessor the preprocessor. It is assumed the preprocessor will discretize the volume, and the highest discretized value will be used
                                 in the joint histogram
       @param targetPreprocessor the preprocessor. It is assumed the preprocessor will discretize the volume, and the highest discretized value will be used
                                 in the joint histogram
       */
      RegistrationAlgorithmIntensityAffineGeneric( const VolumePreprocessor& sourcePreprocessor,
                                                   const VolumePreprocessor& targetPreprocessor,
                                                   const SimilarityFunction& similarity ) : _sourcePreprocessor( sourcePreprocessor ), _targetPreprocessor( targetPreprocessor ), _similarity( similarity )
      {}

      // note: only the initial translation in the initial transformation
      virtual std::shared_ptr<TransformationParametrized> evaluate( const VolumeInput& source, const VolumeInput& target, const TransformationParametrized& source2TargetInitTransformation ) const
      {
         _debugInfo = DebugInfo();

         core::Timer registrationTimer;
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
         // find the maximum value
         core::Timer preprocessingTimer;
         const VolumeOutput sourcePreprocessed = _sourcePreprocessor.run( source );
         const VolumeOutput targetPreprocessed = _targetPreprocessor.run( target );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "preprocessing computation time=" + core::val2str( preprocessingTimer.getCurrentTime() ) );

         // find the maximum value, this will be used to in the joint histogram
         core::Timer maxBinHistogramTimer;
         size_t maxValue = std::numeric_limits<size_t>::min();
         for ( typename VolumeOutput::const_iterator it = sourcePreprocessed.begin(); it != sourcePreprocessed.end(); ++it )
         {
            maxValue = std::max<size_t>( *it, maxValue );
         }
         for ( typename VolumeOutput::const_iterator it = targetPreprocessed.begin(); it != targetPreprocessed.end(); ++it )
         {
            maxValue = std::max<size_t>( *it, maxValue );
         }
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "FindMaxValue time=" + core::val2str( maxBinHistogramTimer.getCurrentTime() ) );

         // construct the pyramid
         // first determine the maximum level (i.e., we want to have as a last level a volume of size 30x30x30)
         // using very rough volumes help the registration algorithm jump the local minima. The registration will be
         // refined using the higher definition volume and will require typically few computations as it will already be positioned to
         // the global maximum (well, ideally!)
         static const size_t maximumSize = 30;
         core::Timer pyramidConstructionTimer;
         Pyramid pyramidSource;
         pyramidSource.constructMaxSize( sourcePreprocessed, maximumSize );

         Pyramid pyramidTarget;
         pyramidTarget.constructMaxSize( targetPreprocessed, maximumSize );

         {
            std::stringstream ss;
            ss << " pyramidSourceNbLevels=" << pyramidSource.size() << std::endl
               << " pyramidTargetNbLevels=" << pyramidTarget.size()
               << " maxDiscretePreprocessedValue=" << maxValue << std::endl
               << " pyramid construction time=" << pyramidConstructionTimer.getCurrentTime();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }
         ensure( maxValue < 256, "the maximum value for the joint histogram seems too big!" );

         // now we are progressively computing the registration starting from a very rough level of the pyramid,
         // and we also use transformation having the lowest degree of freedom, and progressively increase it
         std::shared_ptr<TransformationParametrized> result;
         RegistrationAlgorithmResource resourceTranslationAlgorithm = instanciateTranslationAlgorithm( _similarity, maxValue + 1 ); // joint histogram size = maxvalue + 1
         RegistrationAlgorithmResource resourceTranslationScalingAlgorithm = instanciateTranslationScalingAlgorithm( _similarity, maxValue + 1 ); // joint histogram size = maxvalue + 1

         const core::vector3f initialTranslation = getInitialTranslation( source, source2TargetInitTransformation );
         const std::shared_ptr<TransformationParametrized> tfmInit( resourceTranslationAlgorithm.transformationCreator->create( core::make_buffer1D<double>( initialTranslation[ 0 ], initialTranslation[ 1 ], initialTranslation[ 2 ] ) ) );

         // run successive registrations
         // 1 - translation only
         {
            const VolumeOutput& sourcep = pyramidSource[ pyramidSource.size() - 1 ];
            const VolumeOutput& targetp = pyramidTarget[ pyramidTarget.size() - 1 ];
            result = _runAlgorithm( sourcep, targetp, *tfmInit, resourceTranslationAlgorithm );
         }

         {
            const VolumeOutput& sourcep = pyramidSource[ ( pyramidSource.size() - 1 ) / 2 ];
            const VolumeOutput& targetp = pyramidTarget[ ( pyramidTarget.size() - 1 ) / 2 ];
            result = _runAlgorithm( sourcep, targetp, *result, resourceTranslationAlgorithm );
         }

         // 2 - translation and scaling
         {
            const VolumeOutput& sourcep = pyramidSource[ ( pyramidSource.size() - 1 ) ];
            const VolumeOutput& targetp = pyramidTarget[ ( pyramidTarget.size() - 1 ) ];
            const std::shared_ptr<TransformationParametrized> tfmInitScaling = resourceTranslationScalingAlgorithm.transformationCreator->create( 
               core::make_buffer1D<double>( result->getAffineMatrix()( 0, 3 ),
                                            result->getAffineMatrix()( 1, 3 ),
                                            result->getAffineMatrix()( 2, 3 ),
                                            1, 1, 1 )
            );

            result = _runAlgorithm( sourcep, targetp, *tfmInitScaling, resourceTranslationScalingAlgorithm );
         }

         {
            const VolumeOutput& sourcep = pyramidSource[ ( pyramidSource.size() - 1 ) / 2 ];
            const VolumeOutput& targetp = pyramidTarget[ ( pyramidTarget.size() - 1 ) / 2 ];
            result = _runAlgorithm( sourcep, targetp, *result, resourceTranslationScalingAlgorithm );
         }

         // 3 - translation, scaling and rotation
         // TODO

         {
            std::stringstream ss;
            ss << "RegistrationIntensityBasedGenericAffine:" << std::endl
               << " total computation time=" << registrationTimer.getCurrentTime() << std::endl
               << " Final transformation:" << std::endl;
            result->print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         return result;
      }

      const DebugInfo& getDebugInfo() const
      {
         return _debugInfo;
      }

   protected:
      /**
       @brief Normalize independently the translation and rotation parameters
       */
      class GradientPostprocessorTranslationScaling : public GradientPostprocessor
      {
      public:
         virtual void postprocess( core::Buffer1D<double>& gradient ) const
         {
            ensure( gradient.size() == 6, "error! We are expectingTranslationScaling parameterization" );

            double sumTranslation = 0;
            double sumRotation = 0;
            for ( size_t n = 0; n < 3; ++n )
            {
               sumTranslation += fabs( gradient[ n ] );
               sumRotation += fabs( gradient[ n + 3 ] );
            }

            for ( size_t n = 0; n < 3; ++n )
            {
               gradient[ n ] /= sumTranslation;
               gradient[ n + 3 ] /= sumRotation;
            }
         }
      };

   protected:
      std::shared_ptr<TransformationParametrized> _runAlgorithm( const VolumeOutput& source,
                                                                 const VolumeOutput& target,
                                                                 const TransformationParametrized& initTranslation,
                                                                 const RegistrationAlgorithmResource& resource ) const
      {
         core::Timer algorithmTime;
         std::shared_ptr<TransformationParametrized> result;
         {
            std::stringstream ss;
            ss << " regType=" << resource.algorithmClassIdentifier << ":" << std::endl
               << "  source size=" << source.getSize() << std::endl
               << "  target size=" << target.getSize() << std::endl
               << "  initialTransofrmation=" << std::endl;

            initTranslation.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         result = resource.registrationAlgorithm->evaluate( source, target, initTranslation );

         {
            std::stringstream ss;
            ss << " TranslationAlgorithm result:" << std::endl;
            result->print( ss );
            ss << std:: endl << " computationTime=" << algorithmTime.getCurrentTime();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         _debugInfo.steps.push_back( std::make_pair( resource, result ) );

         return result;
      }


      /**
       @brief From the source volume and initial transformation compute a translation only as initial transformation
       */
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

      /**
       @brief Construct a translation algorithm
       */
      static RegistrationAlgorithmResource instanciateTranslationAlgorithm( const SimilarityFunction& similarity, size_t joinHistogramNbBins ) 
      {
         std::shared_ptr<GradientEvaluator> gradientEvaluator( new GradientEvaluator( core::make_buffer1D<double>( 0.1, 0.1, 0.1 ), true ) );

         RegistrationAlgorithmResource algoRes;
         algoRes.algorithmClassIdentifier = "RegistrationTranslationAlgorithm";
         algoRes.transformationCreator = std::shared_ptr<TransformationCreator>( new TransformationCreatorTranslation() );
         algoRes.histogramMaker        = std::shared_ptr<HistogramMaker>( new HistogramMaker() );
         algoRes.registrationEvaluator = std::shared_ptr<RegistrationEvaluator>( new RegistrationEvaluator( similarity, *algoRes.histogramMaker, joinHistogramNbBins, gradientEvaluator, true ) );

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

      /**
       @brief Construct a translation-scaling algorithm
       */
      static RegistrationAlgorithmResource instanciateTranslationScalingAlgorithm( const SimilarityFunction& similarity, size_t joinHistogramNbBins ) 
      {
         std::shared_ptr<GradientPostprocessor> gradientPostprocessor( new GradientPostprocessorTranslationScaling() );
         std::shared_ptr<GradientEvaluator> gradientEvaluator( new GradientEvaluator( core::make_buffer1D<double>( 0.1, 0.1, 0.1, 0.01, 0.01, 0.01 ), false, gradientPostprocessor ) );

         RegistrationAlgorithmResource algoRes;
         algoRes.algorithmClassIdentifier = "RegistrationTranslationScalingAlgorithm";
         algoRes.transformationCreator = std::shared_ptr<TransformationCreator>( new TransformationCreatorTranslationScaling() );
         algoRes.histogramMaker        = std::shared_ptr<HistogramMaker>( new HistogramMaker() );
         algoRes.registrationEvaluator = std::shared_ptr<RegistrationEvaluator>( new RegistrationEvaluator( similarity, *algoRes.histogramMaker, joinHistogramNbBins, gradientEvaluator, true ) );

         algoRes.optimizationStopCondition = std::shared_ptr<StopCondition>( new algorithm::StopConditionStable( 15 ) );
         algoRes.optimizer = std::shared_ptr<Optimizer>( new algorithm::OptimizerGradientDescent( *algoRes.optimizationStopCondition, 0.0,
                                                                                                  true,
                                                                                                  1,
                                                                                                  core::make_buffer1D<double>( 4, 4, 4, 0.1, 0.1, 0.1 ),
                                                                                                  core::make_buffer1D<double>( 1, 1, 1, 0.1, 0.1, 0.1 ) ) );
         algoRes.registrationAlgorithm = std::shared_ptr<RegistrationAlgorithmIntensity>( new RegistrationAlgorithmIntensity( *algoRes.transformationCreator,
                                                                                                                              *algoRes.registrationEvaluator,
                                                                                                                              *algoRes.optimizer ) );
         return algoRes;
      }

   protected:
      const VolumePreprocessor& _sourcePreprocessor;
      const VolumePreprocessor& _targetPreprocessor;
      const SimilarityFunction& _similarity;

      mutable DebugInfo _debugInfo;
   };
}
}

#endif
