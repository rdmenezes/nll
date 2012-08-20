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

#ifndef OPTIMIZER_GRID_SEARCH_H_
# define OPTIMIZER_GRID_SEARCH_H_

#pragma warning( push )
#pragma warning( disable:4251 ) // std::vector need dll-interface
#pragma warning( disable:4512 ) // std::vector need dll-interface
#pragma warning( disable:4127 ) // conditional expression is constant

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Find the optimal parameters using an exhaustif search. The search step for each parameter is defined
           by <code>params[n].next(val)</code> The whole range for each parameter is searched, this method is
           NOT suitable for a complex search space (lots of parameters and search range).

           Grid search could be extremly expensive in time, but it is guaranteed that it will find the minimum
           value on this grid. If the evaluation function is quite fast, and the range search is small, the algorithm
           is suitable.

    @note the seed parameter is discarded
    */
   class OptimizerGridSearch : public Optimizer
   {
   public:
      using Optimizer::optimize;

      /**
       @brief Optimizer. Use the parameter granularities to increment the search.
      */
      virtual core::Buffer1D<double> optimize( const OptimizerClient& client, const ParameterOptimizers& params, const core::Buffer1D<double>& )
      {
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "grid search started" );
         core::Buffer1D<double> pos( params.size() );
         for ( size_t n = 0; n < params.size(); ++n )
            pos[ n ] = params[ n ].getMin();

         core::Buffer1D<double> bestSolution;
         double min = INT_MAX;
         while ( 1 )
         {
            std::stringstream sstr;
            sstr << "test point:";
            pos.print( sstr );
            sstr << "best=";
            bestSolution.print( sstr );
            sstr << "val=" << min;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sstr.str() );

            double val = client.evaluate( pos );
            if ( val < min )
            {
               min = val;
               bestSolution.clone( pos );
            }

            pos[ 0 ] = params[ 0 ].next( pos[ 0 ] );
            for ( size_t n = 0; n < params.size() - 1; ++n )
               if ( pos[ n ] >= params[ n ].getMax() )
               {
                  pos[ n ] = params[ n ].getMin();
                  pos[ n + 1 ] = params[ n + 1 ].next( pos[ n + 1 ] );
               } else break;
            if ( pos[ params.size() - 1 ] >= params[ params.size() - 1 ].getMax() )
               break;
         }

         std::stringstream sstr;
         sstr << "grid search best solution=" << min << std::endl;
         bestSolution.print( sstr );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sstr.str() );

         return bestSolution;
      }
   };
}
}

#pragma warning( pop )

#endif
