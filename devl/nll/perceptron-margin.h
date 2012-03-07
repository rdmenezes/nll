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
       @param margin the margin where a positive/negative sample will update the classification plane. If margin == 0, same output as the original perceptron
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
            datnorm[ n ] = 1.0 / getNorm( learning[ n ].input );
         }

         // now train the algo
         ui32 inputSize = static_cast<ui32>( learning[ 0 ].input.size() );
         _w = std::vector<value_type>( inputSize + 1 ); // here we are simulating the threshold by adding extra unit x[inputSize] always set to 1
         value_type wnorm = 1;  // wnorm is the norm of the current w
         for ( ui32 n = 0; n < nbCycles; ++n )
         {
            // test and update the separating plane
            for ( ui32 s = 0; s < nbSamples; ++s )
            {
               // compute <xi, w> / || xi ||
               value_type dot = 0;
               for ( ui32 i = 0; i < inputSize; ++i )
               {
                  dot += _w[ i ] * learning[ s ].input[ i ];
               }
               dot += 1 * _w[ inputSize ];   // extra unit
               //dot /= wnorm;
               dot *= datnorm[ s ];


               // test if within the margin or wrong classification
               ui32 classId;
               const value_type marginVal = margin / 2;
               if ( dot > marginVal )
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
                  const value_type signUpdate = ( learning[ s ].output == 1 ) ? 1 : -1;
                  const value_type updateFactor = signUpdate * sampleWeights[ s ] * learningRate;
                  for ( ui32 i = 0; i < inputSize; ++i )
                  {
                     _w[ i ] += updateFactor * learning[ s ].input[ i ] * datnorm[ s ];
                  }
                  _w[ inputSize ] += updateFactor * 1;        // extra unit
                  wnorm = getNorm( _w );
                  if ( wnorm < 1e-5 )
                     wnorm = 1;
               }
            }
         }

         {
            std::stringstream ss;
            ss << "perceptron learning result=" << std::endl;
            for ( ui32 i = 0; i < inputSize + 1; ++i )
            {
               ss << _w[ i ] << " ";
            }

            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }
      }

      template <class Vector>
      ui32 test( const Vector& v ) const
      {
         ui32 inputSize = static_cast<ui32>( v.size() );
         ensure( inputSize + 1 == _w.size(), "wrong size!" );

         value_type dot = 0;
         for ( ui32 i = 0; i < inputSize; ++i )
         {
            dot += _w[ i ] * v[ i ];
         }
         dot += 1 * _w[ inputSize ];

         return dot > 0 ? 1 : 0;
      }

   private:
      template <class Vector>
      value_type getNorm( const Vector& v )
      {
         const ui32 size = static_cast<ui32>( v.size() );
         value_type norm = 0;
         for ( ui32 n = 0; n < size; ++n )
         {
            norm += core::sqr( v[ n ] );
         }
         return std::sqrt( norm );
      }

   private:
      std::vector<value_type> _w;
   };
}
}

#endif
