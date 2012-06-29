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

#ifndef NLL_CORE_CONVERTER_H_
# define NLL_CORE_CONVERTER_H_

//
// The purpose is to provide flexible automatic structure conversion
//
namespace nll
{
namespace core
{
   namespace impl
   {
      // 3 paramemeters: input, output, if input == output
      template <class Input, class Output, int EQUAL>
      struct Converter
      {
         static void convert( const Input& input, Output& output )
         {
            throw std::runtime_error( "conversion is not handled" );
         }
      };

      template <class Input>
      struct Converter<Input, Input, 1>
      {
         static void convert( const Input& input, Input& output )
         {
            output = input;
         }
      };

      template <class Input, class X>
      struct Converter< Input, std::vector<X>, 0 >
      {
         static void convert( const Input& input, std::vector<X>& output )
         {
            output = std::vector<X>( input.size() );
            for ( size_t n = 0; n < input.size(); ++n )
               output[ n ] = static_cast<X>( input[ n ] );
         }
      };

      template <class Input, class X>
      struct Converter< Input, Buffer1D<X>, 0 >
      {
         static void convert( const Input& input, Buffer1D<X>& output )
         {
            output = Buffer1D<X>( static_cast<size_t>( input.size() ), false );
            for ( size_t n = 0; n < input.size(); ++n )
               output[ n ] = static_cast<X>( input[ n ] );
         }
      };
   }

   template <class Input, class X>
   void convert( const Input& i, std::vector<X>& o )
   {
      impl::Converter<Input, std::vector<X>, Equal< Input, std::vector<X> >::value >::convert( i, o );
   }

   template <class Input, class X>
   void convert( const Input& i, Buffer1D<X>& o )
   {
      impl::Converter<Input, Buffer1D<X>, Equal< Input, Buffer1D<X> >::value >::convert( i, o );
   }
}
}

#endif