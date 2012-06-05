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

#ifndef NLL_ALGORITHM_BOOSTING_WEAK_CLASSIFIER_MLP_H_
# define NLL_ALGORITHM_BOOSTING_WEAK_CLASSIFIER_MLP_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Train a Multi-layered perceptron suitable for boosting
    @note the network must not be too strong! Else boosting will not improve and classifier may overfit
    */
   template <class DatabaseT>
   class WeakClassifierMlp : public WeakClassifier<DatabaseT>
   {
   public:
      typedef DatabaseT                                  Database;
      typedef float                                      value_type;
      typedef typename Database::Sample::Input           Point;
      typedef Mlp<FunctionSimpleDifferenciableSigmoid>   Classifier;

   public:
      WeakClassifierMlp( const std::vector<ui32>& layerDescriptor, std::shared_ptr<StopConditionMlp> stopCondition, double learningRate = 0.05, double momentum = 0.1, double weightDecayRate = 0, double reportTimeIntervalInSec = 0.2 ) :
         _layerDescriptor( layerDescriptor ), _stopCondition( stopCondition ), _learningRate( learningRate ), _momentum( momentum ), _weightDecayRate( weightDecayRate ), _reportTimeIntervalInSec( reportTimeIntervalInSec )
      {}

      virtual value_type learn( const Database& dat, const core::Buffer1D<value_type> weights )
      {
         ui32 nbClasses = getNumberOfClass( dat );
         ensure( dat.size() && dat[ 0 ].input.size() == _layerDescriptor[ 0 ], "the first layer must match the number of input features" );
         ensure( *_layerDescriptor.rbegin() == nbClasses, "the last layer must have the same number of outputs than classes" );

         typedef core::Database< core::ClassificationSample<Point, core::Buffer1D<value_type> > > DatLearn;
         DatLearn datLearn;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            core::Buffer1D<value_type> out( nbClasses );
            out[ dat[ n ].output ] = 1;
            datLearn.add( DatLearn::Sample( dat[ n ].input, out, (DatLearn::Sample::Type)dat[ n ].type ) );
         }


         value_type max = std::numeric_limits<value_type>::min();
         for ( ui32 n = 0; n < weights.size(); ++n )        // we reweight the weights so that the learningRate is not dependent on the actual weights!!
         {
            max = std::max( weights[ n ], max );
         }
         value_type coef = (value_type)1.0 / max;
         core::Buffer1D<value_type> reweighted( weights.size() );
         for ( ui32 n = 0; n < weights.size(); ++n )
         {
            reweighted[ n ] = weights[ n ] * coef;
         }


         _classifier.createNetwork( _layerDescriptor );
         Classifier::Result result = _classifier.learn( datLearn, *_stopCondition, _learningRate, _momentum, _weightDecayRate, _reportTimeIntervalInSec, reweighted );
         return (value_type)result.testerror;
      }

      virtual ui32 test( const Point& input ) const
      {
         const core::Buffer1D<double>& v = _classifier.propagate( input );
         core::Buffer1D<double>::const_iterator it = std::max_element( v.begin(), v.end() );

         return static_cast<ui32>( it - v.begin() );
      }

   private:
      std::vector<ui32>                   _layerDescriptor;
      std::shared_ptr<StopConditionMlp>   _stopCondition;
      double                              _learningRate;
      double                              _momentum;
      double                              _weightDecayRate;
      double                              _reportTimeIntervalInSec;
      Classifier                          _classifier;
   };

   /**
    @ingroup algorithm
    @param Weak classifier MLP factory
    */
   template <class DatabaseT>
   class WeakClassifierMlpFactory
   {
   public:
      typedef WeakClassifierMlp<DatabaseT>      Classifier;

      WeakClassifierMlpFactory( const std::vector<ui32>& layerDescriptor, std::shared_ptr<StopConditionMlp> stopCondition, double learningRate = 0.05, double momentum = 0.1, double weightDecayRate = 0, double reportTimeIntervalInSec = 0.2 ) :
         _layerDescriptor( layerDescriptor ), _stopCondition( stopCondition ), _learningRate( learningRate ), _momentum( momentum ), _weightDecayRate( weightDecayRate ), _reportTimeIntervalInSec( reportTimeIntervalInSec )
      {}

      std::shared_ptr<Classifier> create() const
      {
         return std::shared_ptr<Classifier>( new Classifier( _layerDescriptor, _stopCondition, _learningRate, _momentum, _weightDecayRate, _reportTimeIntervalInSec ) );
      }

   private:
      std::vector<ui32>                   _layerDescriptor;
      std::shared_ptr<StopConditionMlp>   _stopCondition;
      double                              _learningRate;
      double                              _momentum;
      double                              _weightDecayRate;
      double                              _reportTimeIntervalInSec;
   };
}
}

#endif