#ifndef NLL_IMAGE_H_
# define NLL_IMAGE_H_

# include <assert.h>
# include "buffer1D.h"
# include "type-traits.h"
# include "index-mapper.h"

namespace nll
{
namespace core
{
   template <class T, class Mapper> class Image;

   // forward declaration
   template <class T, class Mapper> bool readBmp( Image<T, Mapper>& out_i, const std::string& file );

   /**
    @ingroup core
    @brief Define a 2D image. Buffers of images are automatically shared

    @param Mapper must provide:
           -inline ui32 index( const ui32 x, const ui32 y, const ui32 comp ) const
           -and ensure that components are NOT interleaved ( it is assumed that the color components
            are continuous). This assumption is used in several algorithms...
           -addx, addy, addz to compute a new position from a previous position (but less expensive in computation time)
           The mapper is used to define how the memory-pixels are mapped.

     @note for resampled images, it is important to note that the pixel center is
           the top-left corner of the pixel and not the center of the voxel which can
           produce strange results if not aware of!
    */
   template <class T, class Mapper = IndexMapperRowMajorFlat2DColorRGBn>
   class Image : public Buffer1D<T, IndexMapperFlat1D>
   {
   public:
      typedef Mapper                         IndexMapper;

      /**
       @brief An image iterator. It allows to iterate over all pixels, over columns, lines and colors. It is also able to pick without moving
              in one of the 3 possible directions.
       @note addx, addy, addz, pickx, picky, pickz beware of the bounds as they are not checked!
       */
      class DirectionalIterator
      {
      public:
         DirectionalIterator( ui32 index, T* buf, ui32 sx, ui32 sy, ui32 sz, Mapper& mapper ) : _index( index ), _buf( buf ), _sx( sx ),
            _sy( sy ), _sz( sz ), _mapper( mapper )
         {}

         /**
          @brief get the value pointed by the iterator. It is only valid if the iterator is pointing on a pixel!
          */
         T& operator*() const
         {
            return _buf[ _index ];
         }

         /**
          @brief move to the next pixel, the component pointed will be the same than currently
          */
         DirectionalIterator& operator++()
         {
            _index += _sz;
            return *this;
         }

         /**
          @brief move the iterator on a new x
          */
         DirectionalIterator& addx( i32 n = 1 )
         {
            _index = _mapper.addx( _index, n );
            return *this;
         }

         /**
          @brief move the iterator on a new y
          */
         DirectionalIterator& addy( i32 n = 1 )
         {
            _index = _mapper.addy( _index, n );
            return *this;
         }

         /**
          @brief move the iterator on a new color
          @note it operator++ is called, it is moved to the next pixel, pointing at the same component
          */
         DirectionalIterator& addcol( i32 n = 1 )
         {
            _index = _mapper.addz( _index, n );
            return *this;
         }

         /**
          @brief pick a value on the same y, col but different x
          */
         T& pickx( i32 n = 1 ) const
         {
            return _buf[ _mapper.addx( _index, n ) ];
         }

         /**
          @brief pick a value on the same x, col but different y
          */
         T& picky( i32 n = 1 ) const
         {
            return _buf[ _mapper.addy( _index, n ) ];
         }

         /**
          @brief pick a value on the same x, y but different color
          */
         T& pickcol( i32 n = 1 ) const
         {
            return _buf[ _mapper.addz( _index, n ) ];
         }

         /**
          @brief test if the iterators are pointing at the same position.
          */
         bool operator==( const DirectionalIterator& i )
         {
            assert( _buf == i._buf );
            return _index == i._index;
         }

         /**
          @brief test if the iterators are pointing at the same position.
          */
         bool operator!=( const DirectionalIterator& i )
         {
            assert( _buf == i._buf );
            return _index != i._index;
         }

         DirectionalIterator& operator=( const DirectionalIterator& i )
         {
            _index = i._index;
            _buf = i._buf;
            _sx = i._sx;
            _sy = i._sy;
            _sz = i._sz;
            _mapper = i._mapper;
            return *this;
         }

      protected:
         ui32     _index;
         T*       _buf;
         ui32     _sx;
         ui32     _sy;
         ui32     _sz;
         Mapper&  _mapper;
      };

      /**
       @brief An image iterator. It allows to iterate over all pixels, over columns, lines and colors. It is also able to pick without moving
              in one of the 3 possible directions.
       @note addx, addy, addz, pickx, picky, pickz beware of the bounds as they are not checked!
       */
      class ConstDirectionalIterator : public DirectionalIterator
      {
      public:
         ConstDirectionalIterator( ui32 index, const T* buf, ui32 sx, ui32 sy, ui32 sz, Mapper& mapper ) : DirectionalIterator( index, (T*)buf, sx, sy, sz, mapper )
         {}

         ConstDirectionalIterator( const DirectionalIterator& it ) : DirectionalIterator( it )
         {}

         /**
          @brief move to the next pixel, the component pointed will be the same than currently
          */
         ConstDirectionalIterator& operator++()
         {
            this->_index += this->_sz;
            return *this;
         }

         T operator*() const
         {
            return this->_buf[ this->_index ];
         }

         /**
          @brief move the iterator on a new x
          */
         ConstDirectionalIterator& addx( i32 n = 1 )
         {
            this->_index = this->_mapper.addx( this->_index, n );
            return *this;
         }

         /**
          @brief move the iterator on a new y
          */
         ConstDirectionalIterator& addy( i32 n = 1 )
         {
            this->_index = this->_mapper.addy( this->_index, n );
            return *this;
         }

         /**
          @brief move the iterator on a new color
          @note it operator++ is called, it is moved to the next pixel, pointing at the same component
          */
         ConstDirectionalIterator& addcol( i32 n = 1 )
         {
            this->_index = this->_mapper.addz( this->_index, n );
            return *this;
         }

         /**
          @brief pick a value on the same y, col but different x
          */
         T& pickx( i32 n = 1 ) const
         {
            return this->_buf[ this->_mapper.addx( this->_index, n ) ];
         }

         /**
          @brief pick a value on the same x, col but different y
          */
         T& picky( i32 n = 1 ) const
         {
            return this->_buf[ this->_mapper.addy( this->_index, n ) ];
         }

         /**
          @brief pick a value on the same x, y but different color
          */
         T& pickcol( i32 n = 1 ) const
         {
            return this->_buf[ this->_mapper.addz( this->_index, n ) ];
         }

         ConstDirectionalIterator& operator=( const ConstDirectionalIterator& i )
         {
            this->_index = i._index;
            this->_buf = i._buf;
            this->_sx = i._sx;
            this->_sy = i._sy;
            this->_sz = i._sz;
            this->_mapper = i._mapper;
            return *this;
         }
      };

   protected:
      typedef Buffer1D<T, IndexMapperFlat1D> Base;

   public:
      /**
       @brief construct an image of a specific size.
       @param zero if false, the buffer is not initialized
       */
      Image( ui32 sizex, ui32 sizey, ui32 nbComponents, bool zero = true ) : Base( sizex * sizey * nbComponents, zero), _mapper( sizex, sizey, nbComponents ), _sizex( sizex ), _sizey( sizey ), _nbcomp( nbComponents )
      {}

      /**
       @brief construct an empty image
       */
      Image() : _sizex( 0 ), _sizey( 0 ), _mapper( 0, 0, 0 ), _nbcomp( 0 )
      {}

      /**
       @brief contruct an image from a buffer. Ensure the dimensions are correct
       */
      explicit Image( Base& buf, ui32 sizex, ui32 sizey, ui32 nbcmp ) : Base( buf ), _mapper( sizex, sizey, nbcmp ), _sizex( sizex ), _sizey( sizey ), _nbcomp( nbcmp )
      {
         assert( ( buf.size() % (sizex * sizey * nbcmp ) ) == 0 );
      }

      /**
       @brief load a BMP image from a file
       @todo when several formats, add a dispatcher for good format
       */
      Image( const std::string& file ) : _sizex( 0 ), _sizey( 0 ), _mapper( 0, 0, 0 ), _nbcomp( 0 )
      {
         nll::core::readBmp( *this, file );
      }

      /**
       @brief return the position of the pixel (x, y, c) in the buffer
       */
      inline ui32 index( const ui32 x, const ui32 y, const ui32 c ) const
      {
         // previously Base::IndexMapper::index( _mapper.index( x, y, c ) );
         // the base index was taken into account. Instead, it is now discarded
         // as we can still simulate the effect in the image's mapper (and operation
         // on the iterator as column, line or color directions are now always valid)
         return _mapper.index( x, y, c );
      }

      /**
       @brief return the value at the point (x, y, c)
       */
      inline const T at( const ui32 x, const ui32 y, const ui32 c) const
      {
         assert( x < _sizex );
         assert( y < _sizey );
         assert( c < _nbcomp );
         return this->_buffer[ index( x, y, c ) ];
      }

      /**
       @brief return the value at the point (x, y, c)
       */
      inline T& at( const ui32 x, const ui32 y, const ui32 c)
      {
         assert( x < _sizex );
         assert( y < _sizey );
         assert( c < _nbcomp );
         return this->_buffer[ index( x, y, c ) ];
      }

      /**
       @brief return the value at the point (x, y, c)
       */
      inline const T operator()( const ui32 x, const ui32 y, const ui32 c) const { return at( x, y, c ); }

      /**
       @brief return the value at the point (x, y, c)
       */
      inline T& operator()( const ui32 x, const ui32 y, const ui32 c) { return at( x, y, c ); }

      /**
       @brief return the buffer of the pixel (x, y)
       */
      inline T* point( const ui32 x, const ui32 y ){ return this->_buffer + index( x, y, 0 ); }

      /**
       @brief return the buffer of the pixel (x, y)
       */
      inline const T* point( const ui32 x, const ui32 y ) const { return this->_buffer + index( x, y, 0 ); }

      /**
       @brief return the number of components
       */
      ui32 getNbComponents() const { return _nbcomp;}

      /**
       @brief return the size of the image
       */
      ui32 sizex() const { return _sizex;}

      /**
       @brief return the size of the image
       */
      ui32 sizey() const { return _sizey;}

      /**
       @brief copy an image (not shared)
       */
      Image& operator=(const Image& i)
      {
         copy( i );
         return *this;
      }

      /**
       @brief set the pixel (x, y) to a specific value
       */
      void setPixel( const ui32 x, const ui32 y, const T* buf )
      {
         T* b = point( x, y );
         for ( ui32 n = 0; n < _nbcomp; ++n )
            b[ n ] = buf[ n ];
      }

      /**
       @brief clone an image
       */
      void clone(const Image& i)
      {
         dynamic_cast<Base*>(this)->clone( i );
         _mapper = i._mapper;
         _sizex = i._sizex;
         _sizey = i._sizey;
         _nbcomp = i._nbcomp;
      }

      /**
       @brief copy an image (buffer are shared)
       */
      void copy(const Image& i)
      {
         dynamic_cast<Base*>(this)->copy( i );
         _mapper = i._mapper;
         _sizex = i._sizex;
         _sizey = i._sizey;
         _nbcomp = i._nbcomp;
      }

      /**
       @brief return true if 2 pixels are semantically equal
       */
      bool equal( const ui32 x, const ui32 y, const T* p ) const
      {
         const T* pp = point( x, y );
         for ( ui32 n = 0; n < _nbcomp; ++n )
            if ( p[ n ] != pp[ n ] )
               return false;
         return true;
      }

      /**
       @brief return true if 2 images are semantically equal
       */
      bool operator==( const Image& op ) const
      {
         if ( _sizex != op._sizex || _sizey != op._sizey || _nbcomp != op._nbcomp)
            return false;
         if ( op._buffer == this->_buffer )
            return true;
         if ( ! op._buffer )
            return false;
         for ( ui32 nx = 0; nx < _sizex; ++nx )
            for ( ui32 ny = 0; ny < _sizey; ++ny )
               for ( ui32 c = 0; c < _nbcomp; ++c )
               if ( op( nx, ny, c ) != at( nx, ny, c ) )
                  return false; 
         return true;
      }

      /**
       @brief export to a binary stream the image
       */
      void write( std::ostream& o ) const
      {
         nll::core::write<ui32>( _sizex, o );
         nll::core::write<ui32>( _sizey, o );
         nll::core::write<ui32>( _nbcomp, o );
         Base::write( o );
      }

      /**
       @brief import from a binary stream the image
       */
      void read( std::istream& i )
      {
         nll::core::read<ui32>( _sizex, i );
         nll::core::read<ui32>( _sizey, i );
         nll::core::read<ui32>( _nbcomp, i );
         Base::_indexMapper = IndexMapper( _sizex, _sizey, _nbcomp );
         Base::read( i );
      }

      /**
       @brief returns a const iterator on the first pixel
       */
      ConstDirectionalIterator beginDirectional() const
      {
         // we have to remove the const, to provide operator=, however it will be guaranteed that
         // no modifications will be done
         return ConstDirectionalIterator( 0, this->_buffer, _sizex, _sizey, _nbcomp, *( (Mapper*)&_mapper ) );
      }

      /**
       @brief returns a const iterator on the last pixel + 1, component 0
       */
      ConstDirectionalIterator endDirectional() const
      {
         return ConstDirectionalIterator( _sizex * _sizey * _nbcomp, this->_buffer, _sizex, _sizey, _nbcomp, _mapper );
      }

      /**
       @brief returns a const iterator on the first pixel
       */
      DirectionalIterator beginDirectional()
      {
         return DirectionalIterator( 0, this->_buffer, _sizex, _sizey, _nbcomp, _mapper );
      }

      /**
       @brief returns a const iterator on the last pixel + 1, component 0
       */
      DirectionalIterator endDirectional()
      {
         return DirectionalIterator( _sizex * _sizey * _nbcomp, this->_buffer, _sizex, _sizey, _nbcomp, _mapper );
      }

      /**
       @brief returns an iterator on the specified pixel
       */
      ConstDirectionalIterator getIterator( ui32 x, ui32 y, ui32 z ) const
      {
         // we have to remove the const, to provide operator=, however it will be guaranteed that
         // no modifications will be done
         return ConstDirectionalIterator( _mapper.index( x, y, z ), this->_buffer, _sizex, _sizey, _nbcomp, *( (Mapper*)&_mapper ) );
      }

      /**
       @brief returns an iterator on the specified pixel
       */
      DirectionalIterator getIterator( ui32 x, ui32 y, ui32 z )
      {
         return DirectionalIterator( _mapper.index( x, y, z ), this->_buffer, _sizex, _sizey, _nbcomp, _mapper );
      }

      /**
       @brief define black color
       */
      static const T* black()
	   {
		   static const T col[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		   return col;
	   }

      /**
       @brief define white color
       */
	   static const T* white()
	   {
		   static const T col[] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
		   return col;
	   }

      static const T* red()
	   {
		   static const T col[] = {0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		   return col;
	   }

      static const T* blue()
	   {
		   static const T col[] = {255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		   return col;
	   }

   private:
      Mapper      _mapper;
      ui32        _sizex;
      ui32        _sizey;
      ui32        _nbcomp;
   };
}
}

#endif
