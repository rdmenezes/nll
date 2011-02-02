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

#ifndef NLL_TIMER_H_
# define NLL_TIMER_H_

# include <time.h>
# include "types.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Define a simple Timer class.
   */
   class Timer
   {
   public:
      /**
       @brief Instanciate the timer and start it
       */
      Timer()
      {
         start();
         _cend = 0;
      }

      /**
       @brief restart the timer
       */
      void start()
      {
         _cstart = clock();
      }

      /**
       @brief end the timer, return the time in seconds spent
       */
      f32 end()
      {
         _cend = clock();
         return getTime();
      }

      /**
       @brief get the current time since the begining, return the time in seconds spent.
       */
      f32 getCurrentTime() const
      {
         time_t c = clock();
         return static_cast<f32>( c - _cstart ) / CLOCKS_PER_SEC;
      }

      /**
       @brief return the time in seconds spent since between starting and ending the timer. The timer needs to be ended before calling it.
       */
      f32 getTime() const
      {
         return static_cast<f32>( _cend - _cstart ) / CLOCKS_PER_SEC;
      }

   private:
      time_t   _cstart;
      time_t   _cend;
   };
}
}

#endif
