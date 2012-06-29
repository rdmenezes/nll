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

#ifndef NLL_IMAGE_MORPHOLOGY_H_
# define NLL_IMAGE_MORPHOLOGY_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief binary morphology operation.
    */
   inline void dilate( ImageMask& mask )
   {
      ImageMask newMask( mask.sizex(), mask.sizey(), 1, true );
      for ( size_t ny = 1; ny + 1 < mask.sizey(); ++ny )
         for ( size_t nx = 1; nx + 1 < mask.sizex(); ++nx )
         {
            if ( mask( nx + 0, ny + 0, 0 ) ||
                 mask( nx + 1, ny + 0, 0 ) ||
                 mask( nx - 1, ny + 0, 0 ) ||
                 mask( nx + 0, ny + 1, 0 ) ||
                 mask( nx + 0, ny - 1, 0 ) )
              newMask( nx, ny, 0 ) = 255;
         }
      mask = newMask;
   }

   /**
    @ingroup core
    @brief binary morphology operation.
    */
   inline void erode( ImageMask& mask )
   {
      ImageMask newMask( mask.sizex(), mask.sizey(), 1, true );
      for ( size_t ny = 1; ny + 1 < mask.sizey(); ++ny )
         for ( size_t nx = 1; nx + 1 < mask.sizex(); ++nx )
         {
            if ( mask( nx + 0, ny + 0, 0 ) &&
                 mask( nx + 1, ny + 0, 0 ) &&
                 mask( nx - 1, ny + 0, 0 ) &&
                 mask( nx + 0, ny + 1, 0 ) &&
                 mask( nx + 0, ny - 1, 0 ) )
              newMask( nx, ny, 0 ) = 255;
         }
      mask = newMask;
   }

   /**
    @ingroup core
    @brief greyscale morphology operation.
    */
   template <class T, class Mapper, class Allocator>
   void dilate( Image<T, Mapper, Allocator>& i )
   {
      Image<T, Mapper, Allocator> ni( i.sizex(), i.sizey(), i.getNbComponents(), true, i.getAllocator() );
      for ( size_t c = 0; c < i.getNbComponents(); ++c )
         for ( size_t ny = 1; ny + 1 < i.sizey(); ++ny )
            for ( size_t nx = 1; nx + 1 < i.sizex(); ++nx )
            {
               T max = i( nx + 0, ny + 0, c );
               if ( i( nx + 1, ny + 0, 0 ) >= max )
                  max = i( nx + 1, ny + 0, c );
               if ( i( nx + 0, ny + 1, 0 ) >= max )
                  max = i( nx + 0, ny + 1, c );
               if ( i( nx - 1, ny + 0, 0 ) >= max )
                  max = i( nx - 1, ny + 0, c );
               if ( i( nx + 0, ny - 1, 0 ) >= max )
                  max = i( nx + 0, ny - 1, c );
               ni( nx, ny, c ) = max;
            }
      i = ni;
   }

   /**
    @ingroup core
    @brief greyscale morphology operation.
    */
   template <class T, class Mapper, class Allocator>
   void erode( Image<T, Mapper, Allocator>& i )
   {
      Image<T, Mapper, Allocator> ni( i.sizex(), i.sizey(), i.getNbComponents(), true, i.getAllocator() );
      for ( size_t c = 0; c < i.getNbComponents(); ++c )
         for ( size_t ny = 1; ny + 1 < i.sizey(); ++ny )
            for ( size_t nx = 1; nx + 1 < i.sizex(); ++nx )
            {
               T min = i( nx + 0, ny + 0, c );
               if ( i( nx + 1, ny + 0, 0 ) <= min )
                  min = i( nx + 1, ny + 0, c );
               if ( i( nx + 0, ny + 1, 0 ) <= min )
                  min = i( nx + 0, ny + 1, c );
               if ( i( nx - 1, ny + 0, 0 ) <= min )
                  min = i( nx - 1, ny + 0, c );
               if ( i( nx + 0, ny - 1, 0 ) <= min )
                  min = i( nx + 0, ny - 1, c );
               ni( nx, ny, c ) = min;
            }
      i = ni;
   }

   /**
    @ingroup core
    @brief morphology operation (greyscale or binary)
    */
   template <class T, class Mapper, class Allocator>
   inline void opening( Image<T, Mapper, Allocator>& m, size_t size )
   {
      for ( size_t n = 0; n < size; ++n )
         erode( m );
      for ( size_t n = 0; n < size; ++n )
         dilate( m );
   }

   /**
    @ingroup core
    @brief morphology operation (greyscale or binary)
    */
   template <class T, class Mapper, class Allocator>
   inline void closing( Image<T, Mapper, Allocator>& m, size_t size )
   {
      for ( size_t n = 0; n < size; ++n )
         dilate( m );
      for ( size_t n = 0; n < size; ++n )
         erode( m );
   }
}
}

#endif
