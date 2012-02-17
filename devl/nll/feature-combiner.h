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

#ifndef NLL_FEATURE_COMBINER_H_
# define NLL_FEATURE_COMBINER_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Helper function Combine 2 feature transformation into one by concatenating the different feature vectors
    */
   class FeatureCombiner
   {
   public:

      /**
       @brief It is assumed both databases have exactly the same number of samples and they match (i.e., input1[ n ]
              correspond to the same sample than input2[ n ] )
       */
      template <class Database>
      static Database transform( const Database& input1, const Database& input2 )
      {
         Database out;

         ensure( input1.size() == input2.size(), "samples from input1 and input2 must match!!" );
         for ( ui32 n = 0; n < input1.size(); ++n )
         {
            assert( input1[ n ].output == input2[ n ].output ); // "samples must match!!"
            typename Database::Sample s = input1[ n ];
            s.input = process( input1[ n ].input, input2[ n ].input );

            out.add( s );
         }

         return out;
      }

      /**
       @brief Concats all the features of both points
       */
      template <class Point>
      static Point process( const Point& input1, const Point& input2 )
      {
         Point p( input1.size() + input2.size() );
         for ( ui32 n = 0; n < input1.size(); ++n )
            p[ n ] = input1[ n ];
         for ( ui32 n = 0; n < input2.size(); ++n )
            p[ n + input1.size() ] = input2[ n ];
         return p;
      }
   };
}
}

#endif
