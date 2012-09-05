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
}
}

#endif
