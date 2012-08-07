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

#ifndef NLL_ALGORITHM_REGISTRATION_JOINT_HISTOGRAM_SIMILARITY_H_
# define NLL_ALGORITHM_REGISTRATION_JOINT_HISTOGRAM_SIMILARITY_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Function measuring the similarity represented as a joint histogram
    */
   class SimilarityFunction
   {
   public:
      virtual double evaluate( const JointHistogram& jh ) const = 0;

      ~SimilarityFunction()
      {}
   };

   /**
    @brief Sum of square differences similarity function
    */
   class SimilarityFunctionSumOfSquareDifferences : public SimilarityFunction
   {
   public:
      virtual double evaluate( const JointHistogram& jh ) const
      {
         double sum = 0;

         for ( size_t y = 0; y < jh.getNbBins(); ++y )
         {
            for ( size_t x = 0; x < jh.getNbBins(); ++x )
            {
               sum += jh( x, y ) * core::sqr( double( x ) - double( y ) );
            }
         }

         if ( sum <= 0 )
            return std::numeric_limits<double>::min();
         return - sum / jh.getNbSamples();
      }
   };
}
}

#endif