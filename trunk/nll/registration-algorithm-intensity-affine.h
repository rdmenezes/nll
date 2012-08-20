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
      virtual std::shared_ptr<imaging::Transformation> evaluate( const Volume& source, const Volume& target, const imaging::Transformation& source2TargetInitTransformation ) const
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
         core::Buffer1D<double> seed = _creator.getParameters( source2TargetInitTransformation );
         core::Buffer1D<double> result = _optimizer.optimize( _evaluator, _creator.getOptimizerParameters(), seed );
         std::shared_ptr<imaging::Transformation> tfm = _creator.create( result );

         {
            std::stringstream ss;
            ss << "transformation found:" << std::endl;
            tfm->print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }
         return tfm;
      }

   protected:
      const TransformationCreator&  _creator;
      Evaluator&                    _evaluator;
      Optimizer&                    _optimizer;
   };

   /**
    @ingroup algorithm
    @brief Affine registration algorithm embeded in a multi resolution scheme
           
           This is the main algorithm for affine registration using intensity based algorithms

           Volumes are preprocessed by a preprocessor and a pyramid is constructed for both volumes
           which are registered by the given algorithm on each level of the pyramid from the highest to
           the lowest. This done to improve the robustness of the algorithm (less local minima) and to speed up
           the algorithm (i.e., the lowest levels are already registered and only a small transformation will be
           required)
    */
   template <class T, class Storage>
   class RegistrationAlgorithmIntensityPyramid : public RegistrationAlgorithm<T, Storage>, public core::NonAssignable
   {
   public:
      typedef RegistrationAlgorithm<T, Storage>       Base;
      typedef typename Base::Volume                   Volume;
      typedef typename Base::Matrix                   Matrix;

      typedef ui8                                                             InternalType;
      typedef typename Storage::template Rebind<InternalType>::value_type     InternalStorage;
      typedef VolumePreprocessor<T, Storage, InternalType, InternalStorage>   VolumePreprocessorFixed;
      typedef imaging::VolumeSpatial<InternalType, InternalStorage>           VolumeInternal;
      typedef VolumePyramid<InternalType, InternalStorage>                    Pyramid;
      
   public:
      typedef RegistrationEvaluator<InternalType, InternalStorage >        Evaluator;
      typedef RegistrationAlgorithm<InternalType, InternalStorage>         RegistrationAlgorithmInternal;

   public:
      /**
       @param preprocessor the preprocessor used to convert the input volumes into discretized volumes
       @param RegistrationAlgorithmInternal the registration algorithm operating on the discretized volumes
       @param pyramidNbLevels the number of levels in the pyramid (i.e., the granularity of the registration )
       */
      RegistrationAlgorithmIntensityPyramid( const VolumePreprocessorFixed& preprocessor, const RegistrationAlgorithmInternal& registrationAlgorithm, size_t pyramidNbLevels = 4 ) : _preprocessor( preprocessor ), _registrationAlgorithm( registrationAlgorithm ), _pyramidNbLevels( pyramidNbLevels )
      {
         ensure( pyramidNbLevels >= 1, "wrong number of levels" );
      }

      /**
       @brief Run the optimization process
       */
      virtual std::shared_ptr<imaging::Transformation> evaluate( const Volume& source, const Volume& target, const imaging::Transformation& source2TargetInitTransformation ) const
      {
         {
            std::stringstream ss;
            ss << "RegistrationAlgorithmIntensityPyramid::evaluate" << std::endl
               << "nbLevels=" << _pyramidNbLevels
               << "source:";
            source.print( ss );
            ss << std::endl << " target:";
            target.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         const VolumeInternal sourceSampled = _preprocessor.run( source );
         const VolumeInternal targetSampled = _preprocessor.run( target );

         Pyramid sourceSampledPyramid;
         sourceSampledPyramid.construct( sourceSampled, _pyramidNbLevels );

         Pyramid targetSampledPyramid;
         targetSampledPyramid.construct( targetSampled, _pyramidNbLevels );

         ensure( targetSampledPyramid.size() == _pyramidNbLevels, "unexpected number og levels" );
         ensure( sourceSampledPyramid.size() == _pyramidNbLevels, "unexpected number og levels" );

         std::shared_ptr<imaging::Transformation> tfm;
         for ( int level = -1 + (int)_pyramidNbLevels; level >= 0; --level )
         {
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "pyramid level=" + core::val2str( level ) );
            const imaging::Transformation* tfmToUse = ( level + 1 == (int)_pyramidNbLevels ) ? &source2TargetInitTransformation : tfm.get();
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
      const VolumePreprocessorFixed&         _preprocessor;
      size_t                                 _pyramidNbLevels;
      const RegistrationAlgorithmInternal&   _registrationAlgorithm;
   };
}
}

#endif
