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

#include "stdafx.h"
#include "nll.h"

namespace nll
{
namespace core
{
#if defined( _MSC_VER ) && defined( _M_IX86 )
   // IA-32 IEEE format truncate
   // eax = result
   // ebx = <<unused>>
   // ecx = exponent
   // edx = parameter
   // esi = <<unused>>
   // edi = x[1]
   __declspec(naked)
   __declspec(align(16))
   int truncateDouble(double /*x*/)
   {
      __asm
      {
         // edx = x[1];
         mov   edx, [esp+8]

         push  edi
         // eax = 0;
         xor   eax, eax

         // ecx = ((edx >> 20) & 0x7FF) - 1023;
         // edi = edx;
         mov   ecx, edx
         shr   ecx, 20
         mov   edi, edx
         and   ecx, 0x7FF
         sub   ecx, 1023
         // if (ecx < 0) return 0;
         test  ecx, ecx
         jl    done1
         // if (ecx > 31) return INT_MAX;
         // cmp   ecx, 30
         // jg    toobig
         test  ecx, 0xFFFFFFE0
         jnz   toobig

         // eax = 1;
         mov   eax, 1
         // edx &= 0xFFFFF;
         and   edx, 0xFFFFF
         // eax <<= cl;
         shl   eax, cl

         // ecx = -(20 - ecx);
         sub   ecx, 20
         // if (ecx > 0)
         test  ecx, ecx
         // {
         jle   done
         //    edx <<= ecx;
         shl   edx, cl
         //    eax |= edx;
         or    eax, edx
         //    edx = x[0];
         mov   edx, [esp+8]
         //    ecx -= 32;
         sub   ecx, 32
         // }
   done:
         // ecx = -ecx;
         neg   ecx
         // edx >>= ecx;
         shr   edx, cl
         // eax |= edx;
         or    eax, edx
   testsign:
         // if ((edi & 0x80000000) != 0)
         test  edi, 0x80000000
         jz    done1
         //    eax = -eax;
         neg   eax
   done1:
         // return eax;
         pop   edi
         ret
   toobig:
         mov   eax, 0x7FFFFFFF
         jmp   testsign
      }
   }

   __declspec(naked)
   __declspec(align(16))
   NLL_API
   int floor(double /*x*/)
   {
      __asm
      {
         // edx = x[1];
         mov   edx, [esp+8]
         test  edx, 0x80000000
         jnz   negative
         jmp   truncateDouble

   negative:
         push  ebx
         push  edi
         // eax = 0;
         xor   eax, eax
         // ebx = 0;
         xor   ebx, ebx

         // ecx = (edx >> 20) & 0x7FF;
         mov   ecx, edx
         shr   ecx, 20
         and   ecx, 0x7FF
         // if (ecx == 0) return 0;   // small inaccuracy at really, really small negative values (~ -2^-1023)! Fast clamp to zero, though.
         jz    done1
         // ecx -= 1023;
         sub   ecx, 1023
         // edi = edx & 0x80000000;
         mov   edi, edx
         and   edi, 0x80000000
         // if (ecx > 30) return UINT_MAX;
         cmp   ecx, 30
         jg    toobig

         // if (ecx < 0)
         // {
         //    ebx = 1; eax = 0;
         // }
         // else
         // {
         //    ebx = 0; eax = 1;
         // }
         test  ecx, ecx
         setl  bl
         setge al

         // eax <<= cl;
         shl   eax, cl
         
         //    eax |= edi;
         or    eax, edi

         // edx &= 0xFFFFF;
         and   edx, 0xFFFFF
         // ecx = -(20 - ecx);
         sub   ecx, 20
         // if (ecx > 0)
         // {
         jle   leftnotlower
         //    edx <<= ecx;
         shl   edx, cl
         //    eax |= edx;
         or    eax, edx
         //    ecx = -ecx;
         neg   ecx
         //    edx = x[0];
         mov   edx, [esp+12]
         //    ecx += 32;
         add   ecx, 32

         // }
         jmp   doshift
         // else
         // {
   leftnotlower:
         //    // Check the entire right part of the mantissa
         //    ebx = ebx || (x[0] != 0);
         or    ebx, [esp+12]
         //    ecx = -ecx;
         neg   ecx
         // }

   doshift:
         // if (cl <= 31)
         // {
         cmp   cl, 31
         jg    done
         //    // Shift right, but keep an OR of the bits shifted out so we can check to see if a remainder was there
         //    edi = ~(0xFFFFFFFF << cl) & edx;
         xor   edi, edi
         not   edi
         shl   edi, cl
         not   edi
         and   edi, edx
         //    ebx |= edi;
         or    ebx, edi
         
         //    edx >>= ecx;
         shr   edx, cl
         //    eax |= edx;
         or    eax, edx
         // }
   done:
         // if ((eax & 0x80000000) != 0)
         test  eax, 0x80000000
         jz    done1
         //    eax = -(eax & 0x7FFFFFFF);
         and   eax, 0x7FFFFFFF
         neg   eax
         // if (ebx != 0) eax--;
         test  ebx, ebx
         jz    done1
         dec   eax
   done1:
         // return eax;
         pop   edi
         pop   ebx
         ret
   toobig:
         mov   eax, 0x7FFFFFFF
         or    eax, edi
         jmp   done1
      }
   }

   // IA-32 IEEE format truncate to unsigned int
   // eax = result
   // ebx = <<unused>>
   // ecx = exponent
   // edx = parameter
   // esi = <<unused>>
   // edi = x[1]
   __declspec(naked)
   __declspec(align(16))
   NLL_API
   unsigned int truncateu(double /*x*/)
   {
      __asm
      {
         // edx = x[1];
         mov   edx, [esp+8]

         // eax = 0;
         xor   eax, eax

         // ecx = ((edx >> 20) & 0x7FF) - 1023;
         mov   ecx, edx
         shr   ecx, 20
         and   ecx, 0x7FF
         sub   ecx, 1023
         // if (ecx < 0) return 0;
         test  ecx, ecx
         jl    done1
         // if (ecx > 31) return UINT_MAX;
         // cmp   ecx, 31
         // jg    done1
         test  ecx, 0xFFFFFFE0
         jnz   toobig

         // eax = 1 << cl;
         inc   eax
         shl   eax, cl

         // edx &= 0xFFFFF;
         and   edx, 0xFFFFF
         // ecx = -(20 - ecx);
         sub   ecx, 20
         // if (ecx > 0)
         test  ecx, ecx
         // {
         jle   done
         //    edx <<= ecx;
         shl   edx, cl
         //    eax |= edx;
         or    eax, edx
         //    edx = x[0];
         mov   edx, [esp+4]
         //    ecx -= 32;
         sub   ecx, 32
         // }
   done:
         // ecx = -ecx;
         neg   ecx
         // edx >>= ecx;
         shr   edx, cl
         // eax |= edx;
         or    eax, edx
   done1:
         // return eax;
         ret
   toobig:
         not   eax
         ret
      }
   }

#endif


}
}
