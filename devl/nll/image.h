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
           -and ensure that components are NOT interleaved
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
         return Base::IndexMapper::index( _mapper.index( x, y, c ) );
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

   private:
      Mapper      _mapper;
      ui32        _sizex;
      ui32        _sizey;
      ui32        _nbcomp;
   };
}
}

#endif
