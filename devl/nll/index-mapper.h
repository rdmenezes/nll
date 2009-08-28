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
      inline static ui32 index( const ui32 i ){ return i; }
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
      IndexMapperRowMajorFlat2D( const ui32 sizex, const ui32 sizey ) : _sizex( sizex ), _sizey( sizey ){}

      /**
       @brief return the index in memory that map to the position (x, y)
       */
      inline ui32 index( const ui32 x, const ui32 y ) const
      {
        assert( x < _sizex );
        assert( y < _sizey );
        return x + y * _sizex;
      }

      /**
       @brief from the index, return mapped position (x, y) of the buffer
       */
      inline void indexInverse( const ui32 index, ui32& out_x, ui32& out_y ) const
      {
         assert( index < _sizex * _sizey );
         out_x = index % _sizex;
         out_y = index / _sizex;
      }

   private:
      ui32  _sizex;
      ui32  _sizey;
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
      IndexMapperColumnMajorFlat2D( const ui32 sizex, const ui32 sizey ) : _sizex( sizex ), _sizey( sizey ){}

      /**
       @brief return the index in memory that map to the position (x, y)
       */
      inline ui32 index( const ui32 x, const ui32 y ) const
      {
        assert( x < _sizex );
        assert( y < _sizey );
        return y + x * _sizey;
      }

      /**
       @brief from the index, return mapped position (x, y) of the buffer
       */
      inline void indexInverse( const ui32 index, ui32& out_x, ui32& out_y ) const
      {
         assert( index < _sizex * _sizey );
         out_y = index % _sizey;
         out_x = index / _sizey;
      }

   private:
      ui32  _sizex;
      ui32  _sizey;
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
      IndexMapperRowMajorFlat2DColorRGBn( const ui32 sizex, const ui32 sizey, const ui32 nbComponents ) : _sizex( sizex ), _sizey( sizey ), _nbComponents( nbComponents ), _cacheSxMulComp( sizex * nbComponents ){}

      /**
       @brief return the index in memory that map to the position (x, y)
       */
      inline ui32 index( const ui32 x, const ui32 y, const ui32 comp ) const
      {
        return (x + y * _sizex) * _nbComponents + comp;
      }

      inline ui32 addx( i32 index, ui32 size ) const
      {
         return index + _nbComponents * size;
      }

      inline ui32 addy( i32 index, ui32 size ) const
      {
         return index + _cacheSxMulComp * size;
      }

      inline ui32 addz( i32 index, ui32 size ) const
      {
         return index + size;
      }

      private:
         ui32  _sizex;
         ui32  _sizey;
         ui32  _nbComponents;

         ui32  _cacheSxMulComp;
   };

   /**
    @ingroup core
    @brief define how a mask image should map its memory. Here row major.
    
    For internal use only. Only mask image should use this index mapper.
    */
   struct IndexMapperRowMajorFlat2DColorRGBnMask
   {
      IndexMapperRowMajorFlat2DColorRGBnMask( const ui32 sizex, const ui32 sizey, const ui32 /*nbComponents*/ ) : _sizex( sizex ), _sizey( sizey ) {}

      inline ui32 index( const ui32 x, const ui32 y, const ui32 /*comp*/ ) const
      {
        return x + y * _sizex;
      }

      inline ui32 addx( i32 index, ui32 size ) const
      {
         return index + size;
      }

      inline ui32 addy( i32 index, ui32 size ) const
      {
         return index + _sizex * size;
      }

      inline ui32 addz( i32, ui32 ) const
      {
         assert( 0 ); // invalid
         return 0;
      }

      private:
         ui32  _sizex;
         ui32  _sizey;
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
      IndexMapperRowMajorFlat3D( const ui32 sizex, const ui32 sizey, const ui32 sizez ) : _sizex( sizex ), _sizey( sizey ), _sizez( sizez )
      {
         _sxy = _sizex * _sizey;
      }

      /**
       @brief return the index in memory that map to the position (x, y)
       */
      inline ui32 index( const ui32 x, const ui32 y, const ui32 z ) const
      {
         assert( x < _sizex );
         assert( y < _sizey );
         assert( z < _sizez );
         return x + y * _sizex + z * _sxy;
      }

      inline ui32 addx( i32 index, ui32 size ) const
      {
         return index + size;
      }

      inline ui32 addy( i32 index, ui32 size ) const
      {
         return index + _sizex * size;
      }

      inline ui32 addz( i32 index, ui32 size ) const
      {
         return index + _sxy * size;
      }

      private:
         ui32  _sxy;
         ui32  _sizex;
         ui32  _sizey;
         ui32  _sizez;
   };
}
}

#endif
