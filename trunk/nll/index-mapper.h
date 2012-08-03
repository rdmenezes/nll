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

#ifndef NLL_INDEX_MAPPER_H_
# define NLL_INDEX_MAPPER_H_

# include "types.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief define how a 1D buffer is mapped in memory. Here linearly.
    */
   struct IndexMapperFlat1D
   {
      inline static size_t index( const size_t i ){ return i; }
   };

   /**
    @ingroup core
    @brief define how a 2D buffer is mapped in memory. Here row major.
    */
   struct IndexMapperRowMajorFlat2D
   {
      /**
       @brief initialize with the correct dimension of the image
       */
      IndexMapperRowMajorFlat2D( const size_t sizex, const size_t sizey ) : _sizex( sizex ), _sizey( sizey ){}

      /**
       @brief return the index in memory that map to the position (x, y)
       */
      inline size_t index( const size_t x, const size_t y ) const
      {
        assert( x < _sizex );
        assert( y < _sizey );
        return x + y * _sizex;
      }

      /**
       @brief from the index, return mapped position (x, y) of the buffer
       */
      inline void indexInverse( const size_t index, size_t& out_x, size_t& out_y ) const
      {
         assert( index < _sizex * _sizey );
         out_x = index % _sizex;
         out_y = index / _sizex;
      }

      /**
       @brief return the index shifted by x, based on a previous index
       */
      inline size_t addx( size_t index, i32 size ) const
      {
         return index + size;
      }

      /**
       @brief return the index shifted by y, based on a previous index
       */
      inline size_t addy( i32 index, i32 size ) const
      {
         return index + size * _sizex;
      }

   private:
      size_t  _sizex;
      size_t  _sizey;
   };

   /**
    @ingroup core
    @brief define how a 2D buffer is mapped in memory. Here column major.
    */
   struct IndexMapperColumnMajorFlat2D
   {
      /**
       @brief initialize with the correct dimension of the image
       */
      IndexMapperColumnMajorFlat2D( const size_t sizex, const size_t sizey ) : _sizex( sizex ), _sizey( sizey ){}

      /**
       @brief return the index in memory that map to the position (x, y)
       */
      inline size_t index( const size_t x, const size_t y ) const
      {
        assert( x < _sizex );
        assert( y < _sizey );
        return y + x * _sizey;
      }

      /**
       @brief return the index shifted by x, based on a previous index
       */
      inline size_t addx( size_t index, i32 size ) const
      {
         return index + size * _sizey;
      }

      /**
       @brief return the index shifted by y, based on a previous index
       */
      inline size_t addy( size_t index, i32 size ) const
      {
         return index + size;
      }

      /**
       @brief from the index, return mapped position (x, y) of the buffer
       */
      inline void indexInverse( const size_t index, size_t& out_x, size_t& out_y ) const
      {
         assert( index < _sizex * _sizey );
         out_y = index % _sizey;
         out_x = index / _sizey;
      }

   private:
      size_t  _sizex;
      size_t  _sizey;
   };

   /**
    @ingroup core
    @brief define how a 3D buffer is mapped in memory. Here row major and color component linear.
    */
   struct IndexMapperRowMajorFlat2DColorRGBn
   {
      /**
       @brief initialize with the correct dimension of the image
       */
      IndexMapperRowMajorFlat2DColorRGBn( const size_t sizex, const size_t sizey, const size_t nbComponents ) : _sizex( sizex ), _sizey( sizey ), _nbComponents( nbComponents ), _cacheSxMulComp( sizex * nbComponents ){}

      /**
       @brief return the index in memory that map to the position (x, y)
       */
      inline size_t index( const size_t x, const size_t y, const size_t comp ) const
      {
        return (x + y * _sizex) * _nbComponents + comp;
      }

      inline size_t addx( size_t index, i32 size ) const
      {
         return index + _nbComponents * size;
      }

      inline size_t addy( size_t index, i32 size ) const
      {
         return index + _cacheSxMulComp * size;
      }

      inline size_t addz( size_t index, i32 size ) const
      {
         return index + size;
      }

      private:
         size_t  _sizex;
         size_t  _sizey;
         size_t  _nbComponents;

         size_t  _cacheSxMulComp;
   };

   /**
    @ingroup core
    @brief define how a mask image should map its memory. Here row major.
    
    For internal use only. Only mask image should use this index mapper.
    */
   struct IndexMapperRowMajorFlat2DColorRGBnMask
   {
      IndexMapperRowMajorFlat2DColorRGBnMask( const size_t sizex, const size_t sizey, const size_t /*nbComponents*/ ) : _sizex( sizex ), _sizey( sizey ) {}

      inline size_t index( const size_t x, const size_t y, const size_t /*comp*/ ) const
      {
        return x + y * _sizex;
      }

      inline size_t addx( size_t index, i32 size ) const
      {
         return index + size;
      }

      inline size_t addy( size_t index, i32 size ) const
      {
         return index + _sizex * size;
      }

      inline size_t addz( size_t, i32 ) const
      {
         assert( 0 ); // invalid
         return 0;
      }

      private:
         size_t  _sizex;
         size_t  _sizey;
   };

   /**
    @ingroup core
    @brief defines how a continuous 3D buffer should be mapper to memory
    */
   struct IndexMapperRowMajorFlat3D
   {
      /**
       @brief initialize with the correct dimension of the image
       */
      IndexMapperRowMajorFlat3D( const size_t sizex, const size_t sizey, const size_t sizez ) : _sizex( sizex ), _sizey( sizey ), _sizez( sizez )
      {
         _sxy = _sizex * _sizey;
      }

      /**
       @brief return the index in memory that map to the position (x, y)
       */
      inline size_t index( const size_t x, const size_t y, const size_t z ) const
      {
         assert( x < _sizex );
         assert( y < _sizey );
         assert( z < _sizez );
         return x + y * _sizex + z * _sxy;
      }

      inline size_t addx( size_t index, i32 size ) const
      {
         return index + size;
      }

      inline size_t addy( size_t index, i32 size ) const
      {
         return index + _sizex * size;
      }

      inline size_t addz( size_t index, i32 size ) const
      {
         return index + _sxy * size;
      }

      private:
         size_t  _sxy;
         size_t  _sizex;
         size_t  _sizey;
         size_t  _sizez;
   };
}
}

#endif
