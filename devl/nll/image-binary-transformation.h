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

#ifndef NLL_BINARY_TRANSFORMATION_H_
# define NLL_BINARY_TRANSFORMATION_H_

# include "utility.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Adds 2 points.
    */
   template <class T>
   struct BinaryAdd
   {
      inline void operator()(const T* c1, const T* c2, T* out, size_t comp ) const
      {
         for ( size_t n = 0; n < comp; ++n )
         {
            f32 sum = static_cast<f32>( c1[ n ] ) + static_cast<f32>( c2[ n ] );
            out[ n ] = (T)NLL_BOUND( sum, std::numeric_limits<T>::min(), std::numeric_limits<T>::max() );
         }
      }
   };

   /**
    @ingroup core
    @brief Substracts 2 points.
    */
   template <class T>
   struct BinarySub
   {
      inline void operator()(const T* c1, const T* c2, T* out, size_t comp ) const
      {
         for ( size_t n = 0; n < comp; ++n )
         {
            f32 sum = static_cast<f32>( c1[ n ] ) - static_cast<f32>( c2[ n ] );
            out[ n ] = (T)NLL_BOUND( sum, std::numeric_limits<T>::min(), std::numeric_limits<T>::max() );
         }
      }
   };

   /**
    @ingroup core
    @brief Selects the maximum of each component of the 2 points.
    */
   template <class T>
   struct BinaryMax
   {
      inline void operator()(const T* c1, const T* c2, T* out, size_t comp ) const
      {
         for ( size_t n = 0; n < comp; ++n )
         {
            f32 sum = static_cast<f32>( c1[ n ] ) + static_cast<f32>( c2[ n ] );
            out[ n ] = std::max( c1[ n ], c2[ n ] );
         }
      }
   };

   /**
    @ingroup core
    @brief Selects the minimum of each component of the 2 points.
    */
   template <class T>
   struct BinaryMin
   {
      inline void operator()(const T* c1, const T* c2, T* out, size_t comp ) const
      {
         for ( size_t n = 0; n < comp; ++n )
         {
            f32 sum = static_cast<f32>( c1[ n ] ) + static_cast<f32>( c2[ n ] );
            out[ n ] = std::min( c1[ n ], c2[ n ] );
         }
      }
   };

   /**
    @ingroup core
    @brief Transform the image with a defined operation
    @sa BinaryMin, BinaryMax, BinaryAdd, BinarySub.
    */
   template <class type, class mapper, class allocator, class Transformation>
	Image<type, mapper, allocator> transform(const Image<type, mapper, allocator>& img1, const Image<type, mapper, allocator>& img2, const Transformation& transf, allocator alloc = allocator())
	{
      assert( img1.sizex() == img2.sizex() );
      assert( img1.sizey() == img2.sizey() );
      assert( img1.getNbComponents() == img2.getNbComponents() );
		Image<type, mapper, allocator> tmp( img1.sizex(), img1.sizey(), img1.getNbComponents(), false, alloc );
		for ( size_t y = 0; y < img1.sizey(); ++y )
			for ( size_t x = 0; x < img1.sizex(); ++x )
			{
            const type* t1 = img1.point( x, y );
			   const type* t2 = img2.point( x, y );
            type* t3 = tmp.point( x, y );
				transf( t1, t2, t3, img1.getNbComponents() );
         }
		return tmp;
	}
}
}

#endif
