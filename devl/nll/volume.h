#ifndef NLL_IMAGING_VOLUME_H_
# define NLL_IMAGING_VOLUME_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Abstract the voxel buffer. The voxel buffer must share memory between instances.

    The mapper must provide an index given a 3D coordinate. It is initialized with the volume dimension.
    */
   template <class T, class Mapper = core::IndexMapperRowMajorFlat3D>
   class VolumeMemoryBuffer : public core::Buffer1D<T, core::IndexMapperFlat1D>
   {
   protected:
      typedef core::Buffer1D<T>     Base;
      typedef Mapper                IndexMapper;

   public:
      typedef T*                    iterator;
      typedef const T*              const_iterator;

   public:
      /**
       @brief Construct a volume of size (sz, sy, sz)
       */
      VolumeMemoryBuffer( ui32 sx, ui32 sy, ui32 sz, bool zero = true ) : Base( sx * sy * sz, zero ), _mapper( IndexMapper( sx, sy, sz ) )
      {
         _bufferSize = sx * sy * sz;
         _size = core::vector3ui( sx, sy, sz );
      }

      /**
       @brief Construct a dummy volume of size 0
       */
      VolumeMemoryBuffer() : Base( 0 ), _mapper( IndexMapper( 0, 0, 0 ) )
      {
      }

   public:
      /**
       @brief return the value at the point (x, y, z)
       */
      inline const T operator()( const ui32 x, const ui32 y, const ui32 z ) const { return at( x, y, z ); }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline T& operator()( const ui32 x, const ui32 y, const ui32 z ) { return at( x, y, z ); }

      /**
       @brief return an iterator
       */
      iterator begin()
      {
         return this->_buffer;
      }

      /**
       @brief return the end iterator
       */
      iterator end()
      {
         return this->_buffer + _bufferSize;
      }

      /**
       @brief return an iterator
       */
      const_iterator begin() const
      {
         return this->_buffer;
      }

      /**
       @brief return the end iterator
       */
      const_iterator end() const
      {
         return this->_buffer + _bufferSize;
      }

      /**
       @brief return the size of the buffer
       */
      const core::vector3ui& getSize() const
      {
         return _size;
      }

      /**
       @brief clone the buffer. The memory is fully copied and not shred with the source.
       */
      void clone( const VolumeMemoryBuffer& buf )
      {
         _mapper = buf._mapper;
         _bufferSize = buf._bufferSize;
         _size = buf._size;
         Base::clone( buf );
      }

      bool write( std::ostream& f ) const
      {
         _size.write( f );
         Base::write( f );
         return true;
      }

      bool read( std::istream& f )
      {
         _size.read( f );
         _bufferSize = Base::size();
         Base::read( f );
         _mapper = IndexMapper( _size[ 0 ], _size[ 1 ], _size[ 2 ] );
         return true;
      }

   protected:
      /**
       @brief return the position of the pixel (x, y, z) in the buffer
       */
      inline ui32 index( const ui32 x, const ui32 y, const ui32 z ) const
      {
         return Base::IndexMapper::index( _mapper.index( x, y, z ) );
      }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline const T at( const ui32 x, const ui32 y, const ui32 z ) const
      {
         return this->_buffer[ index( x, y, z ) ];
      }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline T& at( const ui32 x, const ui32 y, const ui32 z )
      {
         return this->_buffer[ index( x, y, z ) ];
      }

   protected:
      IndexMapper          _mapper;
      ui32                 _bufferSize;
      core::vector3ui      _size;
   };

   /**
    @ingroup imaging

    @brief Store a set of voxels - volume - The voxel buffer is stored accross instances.
    
    It is assumed that the VolumeMemoryBuffer will provide accessors for data and size, and iterators.
    This buffer has to share its memory. The voxel buffer is abstracted so that it is possible to
    specify how the voxels are stored in memory as these objects are potentially memory consumming and
    specific needs may arise.
    */
   template <class T, class VolumeMemoryBufferType = VolumeMemoryBuffer<T> >
   class Volume
   {
   public:
      typedef typename VolumeMemoryBufferType::iterator        iterator;
      typedef typename VolumeMemoryBufferType::const_iterator  const_iterator;
      typedef VolumeMemoryBufferType                           VoxelBuffer;

   public:
      /**
       @brief Create an empty volume.
       */
      Volume()
      {
      }

      /**
       @brief Create a volume of a fixed size
       */
      Volume( ui32 sx, ui32 sy, ui32 sz, T background = 0, bool zero = true ) : _buffer( sx, sy, sz, zero ), _background( background )
      {
      }

      /**
       @brief Copy the current volume. Internally a new buffer is created (and not shared with the volume)
       */
      void clone( const Volume& vol )
      {
         _buffer.clone( vol._buffer );
      }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline const T operator()( const ui32 x, const ui32 y, const ui32 z ) const { return at( x, y, z ); }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline T& operator()( const ui32 x, const ui32 y, const ui32 z ) { return at( x, y, z ); }

      /**
       @return the size of the volume
       */
      const core::vector3ui& size() const
      {
         return _buffer.getSize();
      }

      bool inside( double x, double y, double z ) const
      {
         return x >= 0 && y >= 0 && z >= 0 &&
                x < _buffer.getSize()[ 0 ] &&
                y < _buffer.getSize()[ 1 ] &&
                z < _buffer.getSize()[ 2 ];
      }

      /**
       @brief return an iterator
       */
      iterator begin()
      {
         return _buffer.begin();
      }

      /**
       @brief return the end iterator
       */
      iterator end()
      {
         return _buffer.end();
      }

      /**
       @brief return an iterator
       */
      const_iterator begin() const
      {
         return _buffer.begin();
      }

      /**
       @brief return the end iterator
       */
      const_iterator end() const
      {
         return _buffer.end();
      }

      bool write( std::ostream& f ) const
      {
         _buffer.write( f );
         core::write<T>( _background, f );
         return true;
      }

      bool read( std::istream& f )
      {
         _buffer.read( f );
         core::read<T>( _background, f );
         return true;
      }

      /**
       @return the background value. This value is used for the values outside the volume for special
               operations like interpolation
       */
      T getBackgroundValue() const
      {
         return _background;
      }

      void setBackgroundValue( T b )
      {
         _background = b;
      }

   protected:
      /**
       @brief return the value at the point (x, y, z)
       */
      inline const T at( const ui32 x, const ui32 y, const ui32 z ) const
      {
         return _buffer( x, y, z );
      }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline T& at( const ui32 x, const ui32 y, const ui32 z )
      {
         return _buffer( x, y, z );
      }

   private:
      VoxelBuffer _buffer;
      T           _background;
   };
}
}

#endif
