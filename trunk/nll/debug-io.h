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

#ifndef NLL_DEBUG_IO_H_
# define NLL_DEBUG_IO_H_

namespace nll
{
namespace debug
{
   /**
    @ingroup debug
    @brief write a 2D buffer to a file
    */
   template <class T>
   void writeDebug( const std::string& file, T* buf, ui32 sizex, ui32 sizey )
   {
      std::ofstream f( file.c_str() );
      for ( ui32 y = 0; y < sizey; ++y )
      {
         for ( ui32 x = 0; x < sizex; ++x )
         {
            core::write<ui8>( (ui8)( ( buf[ x + sizex * y ] % 10 ) + '0' ), f );
         }
         core::write<ui8>( '\n', f );
      }
      f.close();
   }

   /**
    @ingroup debug
    @brief write a 2D buffer, full value to a file
    */
   template <class T, class ViewedAsType>
   void writeDebugValue( const std::string& file, T* buf, ui32 sizex, ui32 sizey )
   {
      std::ofstream f( file.c_str() );
      for ( ui32 y = 0; y < sizey; ++y )
      {
         for ( ui32 x = 0; x < sizex; ++x )
         {
            f << static_cast<ViewedAsType>( buf[ x + sizex * y ] ) << " ";
         }
         f << std::endl;
      }
      f.close();
   }
}
}

#endif
