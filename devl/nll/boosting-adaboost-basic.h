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

#ifndef NLL_ALGORITHM_BOOSTING_ADABOOST_BASIC_H_
# define NLL_ALGORITHM_BOOSTING_ADABOOST_BASIC_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief basic Adaboost implementation for binary classification only

    This version of the algorithm is using resampling of the training data instead of the original version
    (i.e., a specific weak learner using the Dt distribution must be used to weight the learner)

    @note see http://www.site.uottawa.ca/~stan/csi5387/boost-tut-ppr.pdf for implementation details
          see http://www.face-rec.org/algorithms/Boosting-Ensemble/decision-theoretic_generalization.pdf for full proof of the algorithm
    */
   template <class Database>
   class AdaboostBasic
   {
   public:
      typedef WeakClassifier<Database>                WeakClassifierT;
      typedef typename WeakClassifierT::value_type    value_type;
      typedef typename Database::Sample::Output       Class;
      typedef typename Database::Sample::Input        Input;

   public:
      /**
       @brief train the adaboost classifier
       @param dat the database to learn. It is assumed binary classification problems
       @param nbWeakClassifiers the number of weakclassifier composing the strong classifier
       @param factory the factory creating a weak classifier
       @note it must be ensured the classification error of the weak classifier is <50%, else it will be discarded
       */
      template <class WeakClassifierFactory>
      void learn( const Database& dat, ui32 nbWeakClassifiers, const WeakClassifierFactory& factory )
      {
         {
            std::stringstream ss;
            ss << "AdaboostBasic::learn()" << std::endl
               << "nbWeakClassifiers=" << nbWeakClassifiers << std::endl
               << "nbSamples=" << dat.size() << std::endl;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         ui32 nbClass = core::getNumberOfClass( dat );
         ensure(  nbClass == 2, "basic Adaboost is only for binary classification problem" );

         // get the LEARNING sample only
         _classifiers.clear();
         Database learning = core::filterDatabase( dat, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );

         // train the classifiers
         core::Buffer1D<value_type>  distribution( learning.size(), false );
         for ( ui32 n = 0; n < learning.size(); ++n )
         {
            distribution[ n ] = 1.0f / learning.size();
         }

         for ( ui32 n = 0; n < nbWeakClassifiers; ++n )
         {
            // generate a weak classifier and test
            std::shared_ptr<WeakClassifierT> weak = factory.create();
            weak->learn( learning, distribution );

            std::vector<Class> res( learning.size() );
            value_type eps = 0;
            for ( ui32 nn = 0; nn < learning.size(); ++nn )
            {
               res[ nn ] = weak->test( learning[ nn ].input );
               if ( res[ nn ] != learning[ nn ].output )
               {
                  eps += distribution[ nn ];
               }
            }

            {
               std::stringstream ss;
               ss << "weak classifier:" << n << " learning error rate=" << eps;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }

            if ( eps < 0.5 )
            {
               value_type alpha_t = static_cast<value_type>( 0.5 * core::log2( ( 1.0 - eps ) / ( eps + 1e-4 ) ) );

               // update the distribution
               for ( ui32 nn = 0; nn < distribution.size(); ++nn )
               {
                  if ( res[ nn ] != learning[ nn ].output )
                  {
                     distribution[ nn ] *= exp( alpha_t );
                  } else {
                     distribution[ nn ] *= exp( -alpha_t );
                  }
               }

               // renormalize the distribution
               value_type sum = 0;
               for ( ui32 nn = 0; nn < distribution.size(); ++nn )
               {
                  sum += distribution[ nn ];
               }

               ensure( sum > 0, "must be > 0" );
               for ( ui32 nn = 0; nn < distribution.size(); ++nn )
               {
                  distribution[ nn ] /= sum;
               }

               {
                  std::stringstream ss;
                  ss << "weak classifier alpha=" << alpha_t;
                  core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
               }

               _classifiers.push_back( WeakClassifierTest( weak, alpha_t ) );
            } else {
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "weak classifier's error is too large! skipped" + core::val2str( n ) );
               continue;
            }
         }
      }

      virtual Class test( const Input& p ) const
      {
         core::Buffer1D<double> prob( 2 );
         for ( ui32 n = 0; n < _classifiers.size(); ++n )
         {
            Class t = _classifiers[ n ].classifier->test( p );
            ensure( t < 2, "this Adaboost implementation handles only binary decision problems" );
            prob[ t ] += _classifiers[ n ].alpha;
         }

         if ( prob[ 0 ] < prob[ 1 ] )
         {
            return 1;
         } else {
            return 0;
         }
      }

   public:
      struct WeakClassifierTest
      {
         WeakClassifierTest( std::shared_ptr<WeakClassifierT> c, value_type a ) : classifier( c ), alpha( a )
         {}

         value_type                        alpha;
         std::shared_ptr<WeakClassifierT>  classifier;
      };

      const std::vector<WeakClassifierTest>& getClassifiers() const
      {
         return _classifiers;
      }


   private:
      std::vector<WeakClassifierTest> _classifiers;
   };
}
}

#endif
