/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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

#ifndef NLL_STOPPING_CONDITION_H_
# define NLL_STOPPING_CONDITION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief define a stopping condition criteria
    */
   class StopCondition
   {
   public:
      /**
       @brief return true if algorithm needs to be stopped else return false
       */
      virtual bool stop( f64 fitness ) const = 0;

      /**
       @brief reitinialize the stop condition if necessary
       */
      virtual void reinit(){}

      virtual ~StopCondition()
      {}
   };

   /**
    @ingroup algorithm
    @brief define a stopping condition only based on the number of iterations
    */
   class StopConditionIteration : public StopCondition
   {
   public:
      StopConditionIteration( ui32 nbIterMax ) : _nbIterMax( nbIterMax ), _iter( 0 )
      {}

      /**
       @brief increment the iteration counter each this method is called
       */
      virtual bool stop( f64 /*fitness*/ ) const
      {
         if ( _iter >= _nbIterMax )
            return true;
         ++_iter;
         return false;
      }

      virtual void reinit()
      {
         _iter = 0;
      }

   private:
      mutable ui32 _iter;
      ui32 _nbIterMax;
   };

   /**
    @ingroup algorithm
    @brief define a stopping condition only based on the fitness
    */
   class StopConditionFitness : public StopCondition
   {
   public:
      StopConditionFitness( f64 minFitness ) : _minFitness( minFitness )
      {}

      /**
       @brief increment the iteration counter each this method is called
       */
      virtual bool stop( f64 fitness ) const
      {
         return _minFitness <= fitness;
      }

   private:
      f64 _minFitness;
   };
}
}

#endif
