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

#ifndef NLL_ALGORITM_OPTIMIZER_POWELL_H_
# define NLL_ALGORITM_OPTIMIZER_POWELL_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Powell optimizer. Efficient for problems with low dimentionality (<= 10)
    @param randomSeed the powell optimization is independantly generated a fixed number of times. The best solution is exported
    @param tolerance the tolerance of the algorithm
    */
   class NLL_API OptimizerPowell : public Optimizer
   {
   public:
      using Optimizer::optimize;

      OptimizerPowell( size_t randomSeed = 40, f64 tolerance = 1e-4, size_t nbMaxIter = 200 ) : _nbSeeds( randomSeed ), _tolerance( tolerance ), _nbMaxIter( nbMaxIter )
      {}

      /**
       @brief returns an empty set if optimization failed.
       */
      virtual core::Buffer1D<double> optimize( const OptimizerClient& client, const ParameterOptimizers& parameters, const core::Buffer1D<double>& seed )
      {
         core::Buffer1D<f64> best;
         best.clone( seed );
         double min = client.evaluate( best );

         std::vector< core::Buffer1D<double> > dir;
         for ( size_t n = 0; n < parameters.size(); ++n )
         {
            dir.push_back( core::Buffer1D<f64>( parameters.size() ) );
            dir[ n ][ n ] = 1;
         }

         for ( size_t n = 0; n < _nbSeeds; ++n )
         {
            core::Buffer1D<f64> ini( parameters.size() );
            if ( n == 0 )
            {
               // just use the seed for the first run, else random generation controlled by the parameters
               for ( size_t n = 0; n < parameters.size(); ++n )
                  ini[ n ] = seed[ n ];
            } else {
               for ( size_t n = 0; n < parameters.size(); ++n )
                  ini[ n ] = parameters[ n ].generate();
            }
            bool error = false;
            double val = powell( ini, dir, _tolerance, client, _nbMaxIter, &error );

            if ( val < min )
            {
               min = val;
               best.clone( ini );
            }
         }

         return best;
      }

   protected:
      size_t   _nbSeeds;
      f64      _tolerance;
      size_t   _nbMaxIter;
   };
}
}

#endif
