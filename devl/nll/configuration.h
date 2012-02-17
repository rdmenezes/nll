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

#ifndef NLL_CORE_CONFIGURATION_H_
# define NLL_CORE_CONFIGURATION_H_

# ifndef NLL_DISABLE_SSE_SUPPORT
#  include <xmmintrin.h>
#  include <emmintrin.h>
#endif

namespace nll
{
namespace core
{
   namespace impl
   {
      /**
       @ingroup core
       @brief Detect & publish the processor capabilities

       This class is used at runtime to launch optimized routines
       */
      class NLL_API _Configuration
      {
      public:
         // detect CPU features at initialisation
         _Configuration();
         
         bool isSupportedSSE() const
         {
            return _supportSSE;
         }

         bool isSupportedSSE2() const
         {
            return _supportSSE2;
         }

         bool isSupportedSSE3() const
         {
            return _supportSSE3;
         }

         bool isSupportedMMX() const
         {
            return _supportMMX;
         }

         void disableSSE()
         {
            _supportSSE = false;
            _supportSSE2 = false;
            _supportSSE3 = false;
         }

      private:
         bool  _supportSSE;
         bool  _supportSSE2;
         bool  _supportSSE3;
         bool  _supportMMX;
      };
   }

   typedef Singleton<impl::_Configuration>   Configuration;
}
}

#endif
