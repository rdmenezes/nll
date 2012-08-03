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

#ifndef NLL_MATH_CORRELATION_H_
# define NLL_MATH_CORRELATION_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Compute the correlation of 2 vectors.
    */
   template <class T, class Mapper, class Allocator>
   double correlation( const Matrix<T, Mapper, Allocator>& v1, const Matrix<T, Mapper, Allocator>& v2 )
   {
      // assert if it is really a vector, with the same dimention
      assert( std::min( v1.sizex(), v1.sizey() ) == 1 );
      assert( std::min( v2.sizex(), v2.sizey() ) == 1 );
      assert( v1.size() == v2.size() );
      assert( v1.size() );

      double mean1 = 0, mean2 = 0;
      for ( size_t n = 0; n < v1.size(); ++n )
      {
         mean1 += v1[ n ];
         mean2 += v2[ n ];
      }
      mean1 /= v1.size();
      mean2 /= v2.size();

      double cov = 0, var1 = 0, var2 = 0;
      for ( size_t n = 0; n < v1.size(); ++n )
      {
         const double t1 = v1[ n ] - mean1;
         const double t2 = v2[ n ] - mean2;
         cov += t1 * t2;
         var1 += t1 * t1;
         var2 += t2 * t2;
      }
      return cov / sqrt( var1 * var2 );
   }
}
}

#endif
