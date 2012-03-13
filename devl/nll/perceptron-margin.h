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

#ifndef NLL_ALGORITHM_PERCEPTRON_MARGIN_H_
# define NLL_ALGORITHM_PERCEPTRON_MARGIN_H_

namespace nll
{
namespace algorithm
{
   /**
   @brief Implementation of the margine perceptron whci is better handling the noise than the original one
   @note implemented as described in http://jmlr.csail.mit.edu/papers/volume8/khardon07a/khardon07a.pdf
         "Noise Tolerant Variants of the Perceptron Algorithm", Roni Khardon, Gabriel Wachman

         Additionally, the algorithm is also weigthing the samples
   */
   class MarginPerceptron
   {
   public:
      typedef float      value_type;

   public:

      /**
       @brief Train the classifier
       @param learningRate the speed of each update
       @param margin the margin where a positive/negative sample will update the classification plane. it must be in domain [0..1]. If margin == 0, same output as the original perceptron
       @param _sampleWeights the weight of each sample. Must be in [0..1]
       */
      template <class Database>
      void learn( const Database& _dat, ui32 nbCycles, value_type learningRate, value_type margin, const core::Buffer1D<value_type> _sampleWeights = core::Buffer1D<value_type>() )
      {
         Database learning = core::filterDatabase( _dat, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );
         if ( learning.size() == 0 )
            return;

         const ui32 nbSamples = static_cast<ui32>( learning.size() );
         const ui32 nbClasses = getNumberOfClass( learning );
         ensure( nbClasses == 2, "only binary decision problems" );
         ensure( _sampleWeights.size() == _dat.size() || _sampleWeights.size() == 0, "weights must be undefined or have exactly the same size!" );
         ensure( margin >= 0 && learningRate > 0, "margin and learning rate must be positive" );

         // extract the weights
         core::Buffer1D<value_type> sampleWeights( learning.size() );
         value_type maxW = -1;
         for ( ui32 n = 0, id = 0; n < _dat.size(); ++n )
         {
            if ( _dat[ n ].type == Database::Sample::LEARNING )
            {
               sampleWeights[ id ] = _sampleWeights.size() ? _sampleWeights[ n ] : 1;
               maxW = std::max( maxW, sampleWeights[ id ] );
               ++id;
            }
         }

         // resample the weights so that maxweight = 1, we do that so that the learningRate doesn't depend on the data
         const value_type factor = 1 / maxW;
         for ( ui32 n = 0; n < sampleWeights.size(); ++n )
         {
            sampleWeights[ n ] *= factor;
         }


         // get the norm of the examples
         std::vector<value_type> datnorm( learning.size() );
         for ( ui32 n = 0; n < nbSamples; ++n )
         {
            const value_type norm = getNorm( learning[ n ].input );
            datnorm[ n ] = (value_type)1.0 / ( norm > 0 ? norm : 1 );
         }

         value_type bestNumberOfUpdates = std::numeric_limits<ui32>::max();
         std::vector<value_type> bestW;
         value_type bestBias = 0;

         const ui32 nbCyclesUpdate = nbCycles / 10;

         // now train the algo
         ui32 inputSize = static_cast<ui32>( learning[ 0 ].input.size() );
         _w = std::vector<value_type>( inputSize );
         _bias = 0; // the bias can just be seen as another entry in <_w> with the corresponding new x_j alway equal to 1
         value_type wnorm = 1;
         for ( ui32 n = 0; n < nbCycles; ++n )
         {
            value_type nbUpdates = 0;

            // test and update the separating plane
            for ( ui32 s = 0; s < nbSamples; ++s )
            {
               // compute <xi, w> / (|| xi || * ||w||)   // we normalize w and x so that the margin is constant i.e. in domain [0..1]
               value_type dot = 0;
               for ( ui32 i = 0; i < inputSize; ++i )
               {
                  dot += _w[ i ] * learning[ s ].input[ i ];
               }
               dot *= datnorm[ s ] / wnorm;
               dot += _bias * datnorm[ s ]  / wnorm;


               // test if within the margin or wrong classification
               ui32 classId;
               const value_type marginVal = margin / 2;
               if ( dot > + marginVal )
               {
                  classId = 1;   // positive
               } else if ( dot < - marginVal )
               {
                  classId = 0;   // negative
               } else {
                  classId = 2;   // margin, force update
               }

               // update class mistake or too close to the margin
               const bool needToUpdate = classId != learning[ s ].output;
               if ( needToUpdate )
               {
                  const value_type signUpdate = ( learning[ s ].output == 1 ) ? (value_type)1 : (value_type)-1;
                  const value_type updateFactor = signUpdate * sampleWeights[ s ] * learningRate * datnorm[ s  ];
                  wnorm = 0;
                  for ( ui32 i = 0; i < inputSize; ++i )
                  {
                     _w[ i ] += updateFactor * learning[ s ].input[ i ];
                  }
                  _bias += updateFactor * 1;
                  wnorm = getWeightNorm();
                  nbUpdates += sampleWeights[ s ];
               }
            }

            if ( n % nbCyclesUpdate == 0 )
            {
               std::stringstream ss;
               ss << " perceptron cycle=" << n << " nbUpdatesThisCycle=" << nbUpdates;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }

            if ( nbUpdates < bestNumberOfUpdates )
            {
               bestNumberOfUpdates = nbUpdates;
               bestW = _w;
               bestBias = _bias;
            }

            if ( nbUpdates == 0 )
               break;
         }

         _w = bestW;
         _bias = bestBias;

         // renormalize decision plane
         wnorm = getWeightNorm();
         for ( ui32 i = 0; i < inputSize; ++i )
         {
            _w[ i ] /= wnorm;
         }
         _bias /= wnorm;

         {
            std::stringstream ss;
            ss << "perceptron learning result=" << std::endl;
            for ( ui32 i = 0; i < inputSize; ++i )
            {
               ss << _w[ i ] << " ";
            }

            ss << std::endl << "bias=" << _bias;

            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }
      }

      template <class Vector>
      ui32 test( const Vector& v ) const
      {
         ui32 inputSize = static_cast<ui32>( v.size() );
         ensure( inputSize == _w.size(), "wrong size!" );

         value_type dot = 0;
         for ( ui32 i = 0; i < inputSize; ++i )
         {
            dot += _w[ i ] * v[ i ];
         }
         dot += _bias;

         return dot > 0 ? 1 : 0;
      }

   private:
      template <class Vector>
      static value_type getNorm( const Vector& v )
      {
         const ui32 size = static_cast<ui32>( v.size() );
         value_type norm = 0;
         for ( ui32 n = 0; n < size; ++n )
         {
            norm += core::sqr( v[ n ] );
         }
         return std::sqrt( norm );
      }

      value_type getWeightNorm() const
      {
         value_type wnorm = 0;
         for ( size_t i = 0; i < _w.size(); ++i )
         {
            wnorm += core::sqr( _w[ i ] );
         }
         wnorm += core::sqr( _bias );
         wnorm = sqrt( wnorm );
         return wnorm;
      }

   private:
      std::vector<value_type> _w;
      value_type              _bias;
   };

   /**
   @brief Implementation of the margine perceptron whci is better handling the noise than the original one
   @note The pocket version keeps the best solution found so far
   */
   class MarginPocketPerceptron
   {
   public:
      typedef float      value_type;

   public:

      /**
       @brief Train the classifier
       @param learningRate the speed of each update
       @param margin the margin where a positive/negative sample will update the classification plane. it must be in domain [0..1]. If margin == 0, same output as the original perceptron
       @param _sampleWeights the weight of each sample. Must be in [0..1]
       */
      template <class Database>
      void learn( const Database& _dat, ui32 nbCycles, value_type learningRate, value_type margin, const core::Buffer1D<value_type> _sampleWeights = core::Buffer1D<value_type>() )
      {
         Database learning = core::filterDatabase( _dat, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );
         if ( learning.size() == 0 )
            return;

         const ui32 nbSamples = static_cast<ui32>( learning.size() );
         const ui32 nbClasses = getNumberOfClass( learning );
         ensure( nbClasses == 2, "only binary decision problems" );
         ensure( _sampleWeights.size() == _dat.size() || _sampleWeights.size() == 0, "weights must be undefined or have exactly the same size!" );
         ensure( margin >= 0 && learningRate > 0, "margin and learning rate must be positive" );

         // extract the weights
         core::Buffer1D<value_type> sampleWeights( learning.size() );
         value_type maxW = -1;
         for ( ui32 n = 0, id = 0; n < _dat.size(); ++n )
         {
            if ( _dat[ n ].type == Database::Sample::LEARNING )
            {
               sampleWeights[ id ] = _sampleWeights.size() ? _sampleWeights[ n ] : 1;
               maxW = std::max( maxW, sampleWeights[ id ] );
               ++id;
            }
         }

         // resample the weights so that maxweight = 1, we do that so that the learningRate doesn't depend on the data
         const value_type factor = 1 / maxW;
         for ( ui32 n = 0; n < sampleWeights.size(); ++n )
         {
            sampleWeights[ n ] *= factor;
         }


         // get the norm of the examples
         std::vector<value_type> datnorm( learning.size() );
         for ( ui32 n = 0; n < nbSamples; ++n )
         {
            const value_type norm = getNorm( learning[ n ].input );
            datnorm[ n ] = (value_type)1.0 / ( norm > 0 ? norm : 1 );
         }

         value_type bestError = std::numeric_limits<ui32>::max();
         std::vector<value_type> bestW;
         value_type bestBias = 0;

         const ui32 nbCyclesUpdate = nbCycles / 10;

         // now train the algo
         ui32 inputSize = static_cast<ui32>( learning[ 0 ].input.size() );
         _w = std::vector<value_type>( inputSize );
         _bias = 0; // the bias can just be seen as another entry in <_w> with the corresponding new x_j alway equal to 1
         value_type wnorm = 1;
         for ( ui32 n = 0; n < nbCycles; ++n )
         {
            ui32 nbUpdates = 0;

            // test and update the separating plane
            for ( ui32 s = 0; s < nbSamples; ++s )
            {
               // compute <xi, w> / (|| xi || * ||w||)   // we normalize w and x so that the margin is constant i.e. in domain [0..1]
               value_type dot = 0;
               for ( ui32 i = 0; i < inputSize; ++i )
               {
                  dot += _w[ i ] * learning[ s ].input[ i ];
               }
               dot *= datnorm[ s ] / wnorm;
               dot += _bias * datnorm[ s ]  / wnorm;


               // test if within the margin or wrong classification
               ui32 classId;
               const value_type marginVal = margin / 2;
               if ( dot > + marginVal )
               {
                  classId = 1;   // positive
               } else if ( dot < - marginVal )
               {
                  classId = 0;   // negative
               } else {
                  classId = 2;   // margin, force update
               }

               // update class mistake or too close to the margin
               const bool needToUpdate = classId != learning[ s ].output;
               if ( needToUpdate )
               {
                  const value_type signUpdate = ( learning[ s ].output == 1 ) ? (value_type)1 : (value_type)-1;
                  const value_type updateFactor = signUpdate * sampleWeights[ s ] * learningRate * datnorm[ s  ];
                  wnorm = 0;
                  for ( ui32 i = 0; i < inputSize; ++i )
                  {
                     _w[ i ] += updateFactor * learning[ s ].input[ i ];
                  }
                  _bias += updateFactor * 1;
                  wnorm = getWeightNorm();
                  ++nbUpdates;

                  // now test if we are doing better
                  const value_type error = evaluate( learning, sampleWeights );
                  if ( error < bestError )
                  {
                     bestError = error;
                     bestW = _w;
                     bestBias = _bias;
                  }
               }
            }

            if ( n % nbCyclesUpdate == 0 )
            {
               std::stringstream ss;
               ss << " perceptron cycle=" << n << " nbUpdatesThisCycle=" << nbUpdates;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }

            if ( nbUpdates == 0 )
               break;
         }

         _w = bestW;
         _bias = bestBias;

         // renormalize decision plane
         wnorm = getWeightNorm();
         for ( ui32 i = 0; i < inputSize; ++i )
         {
            _w[ i ] /= wnorm;
         }
         _bias /= wnorm;

         {
            std::stringstream ss;
            ss << "perceptron learning result=" << std::endl;
            for ( ui32 i = 0; i < inputSize; ++i )
            {
               ss << _w[ i ] << " ";
            }

            ss << std::endl << "bias=" << _bias;

            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }
      }

      template <class Vector>
      ui32 test( const Vector& v ) const
      {
         ui32 inputSize = static_cast<ui32>( v.size() );
         ensure( inputSize == _w.size(), "wrong size!" );

         value_type dot = 0;
         for ( ui32 i = 0; i < inputSize; ++i )
         {
            dot += _w[ i ] * v[ i ];
         }
         dot += _bias;

         return dot > 0 ? 1 : 0;
      }

   private:
      template <class Database>
      value_type evaluate( const Database& dat, const core::Buffer1D<float>& weights ) const
      {
         value_type error = 0;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            ui32 cc = test( dat[ n ].input );
            if ( cc != dat[ n ].output )
               error += weights[ n ];
         }
         return error;
      }

      template <class Vector>
      static value_type getNorm( const Vector& v )
      {
         const ui32 size = static_cast<ui32>( v.size() );
         value_type norm = 0;
         for ( ui32 n = 0; n < size; ++n )
         {
            norm += core::sqr( v[ n ] );
         }
         return std::sqrt( norm );
      }

      value_type getWeightNorm() const
      {
         value_type wnorm = 0;
         for ( size_t i = 0; i < _w.size(); ++i )
         {
            wnorm += core::sqr( _w[ i ] );
         }
         wnorm += core::sqr( _bias );
         wnorm = sqrt( wnorm );
         return wnorm;
      }

   private:
      std::vector<value_type> _w;
      value_type              _bias;
   };
}
}

#endif
