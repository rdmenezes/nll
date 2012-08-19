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
      typedef T                     value_type;

      /**
       @brief Utility class allowing to create a similar buffer for another type
       */
      template <class U>
      struct Rebind
      {
         typedef VolumeMemoryBuffer<U, Mapper>  value_type;
      };

      /**
       @brief An image iterator. It allows to iterate over all voxels, slices, over columns, lines. It is also able to pick without moving
              in one of the 3 possible directions.
       @note addx, addy, addz, pickx, picky, pickz beware of the bounds as they are not checked!
       */
      class DirectionalIterator
      {
      public:
         typedef T   value_type;

         DirectionalIterator( size_t index, T* buf, size_t sx, size_t sy, size_t sz, const Mapper& mapper ) : _index( index ), _buf( buf ), _sx( sx ),
            _sy( sy ), _sz( sz ), _mapper( mapper )
         {}

         /**
          @brief get the value pointed by the iterator. It is only valid if the iterator is pointing on a voxel!
          */
         T& operator*() const
         {
            return _buf[ _index ];
         }

         /**
          @brief move to the next moxel
          */
         DirectionalIterator& operator++()
         {
            ++_index;
            return *this;
         }

         DirectionalIterator operator++( int )
         {
            DirectionalIterator ans = *this;
            ++_index;
            return ans;
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
          @brief move the iterator on a new z
          */
         DirectionalIterator& addz( i32 n = 1 )
         {
            _index = _mapper.addz( _index, n );
            return *this;
         }

         /**
          @brief pick a value on the same y, z but different x
          */
         T pickx( i32 n = 1 ) const
         {
            return _buf[ _mapper.addx( _index, n ) ];
         }

         /**
          @brief pick a value on the same x, z but different y
          */
         T picky( i32 n = 1 ) const
         {
            return _buf[ _mapper.addy( _index, n ) ];
         }

         /**
          @brief pick a value on the same x, y but different z
          */
         T pickz( i32 n = 1 ) const
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

      protected:
         size_t     _index;
         T*       _buf;
         size_t     _sx;
         size_t     _sy;
         size_t     _sz;
         Mapper   _mapper;
      };

      /**
       @brief An image iterator. It allows to iterate over all voxels, slices, over columns, lines. It is also able to pick without moving
              in one of the 3 possible directions.
       @note addx, addy, addz, pickx, picky, pickz beware of the bounds as they are not checked!
       */
      class ConstDirectionalIterator : public DirectionalIterator
      {
      public:
         typedef T   value_type;

         ConstDirectionalIterator( size_t index, const T* buf, size_t sx, size_t sy, size_t sz, const Mapper& mapper ) : 
            DirectionalIterator( index, (T*)buf, sx, sy, sz, mapper )
         {}

         ConstDirectionalIterator( const DirectionalIterator& i ) : DirectionalIterator( i )
         {}

         T operator*() const
         {
            return this->_buf[ this->_index ];
         }

         /**
          @brief move to the next moxel
          */
         ConstDirectionalIterator& operator++()
         {
            ++this->_index;
            return *this;
         }

         ConstDirectionalIterator operator++( int )
         {
            ConstDirectionalIterator ans = *this;
            ++this->_index;
            return ans;
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
          @brief move the iterator on a new z
          */
         ConstDirectionalIterator& addz( i32 n = 1 )
         {
            this->_index = this->_mapper.addz( this->_index, n );
            return *this;
         }

         /**
          @brief pick a value on the same y, z but different x
          */
         T pickx( i32 n = 1 ) const
         {
            return this->_buf[ this->_mapper.addx( this->_index, n ) ];
         }

         /**
          @brief pick a value on the same x, z but different y
          */
         T picky( i32 n = 1 ) const
         {
            return this->_buf[ this->_mapper.addy( this->_index, n ) ];
         }

         /**
          @brief pick a value on the same x, y but different z
          */
         T pickz( i32 n = 1 ) const
         {
            return this->_buf[ this->_mapper.addz( this->_index, n ) ];
         }
      };

   public:
      /**
       @brief Construct a volume of size (sz, sy, sz)
       */
      VolumeMemoryBuffer( size_t sx, size_t sy, size_t sz, bool zero = true ) : Base( sx * sy * sz, zero ), _mapper( IndexMapper( sx, sy, sz ) )
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
      inline const T operator()( const size_t x, const size_t y, const size_t z ) const { return at( x, y, z ); }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline T& operator()( const size_t x, const size_t y, const size_t z ) { return at( x, y, z ); }

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

      size_t sizex() const
      {
         return _size[ 0 ];
      }

      size_t sizey() const
      {
         return _size[ 1 ];
      }

      size_t sizez() const
      {
         return _size[ 2 ];
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

      /**
       @brief returns a const iterator on the first voxel
       */
      ConstDirectionalIterator beginDirectional() const
      {
         return ConstDirectionalIterator( 0, this->_buffer, _size[ 0 ], _size[ 1 ], _size[ 2 ], _mapper );
      }

      /**
       @brief returns a const iterator on the last voxel + 1
       @note this is a costly operation! it must be cahced
       */
      ConstDirectionalIterator endDirectional() const
      {
         return ConstDirectionalIterator( _size[ 0 ] * _size[ 1 ] * _size[ 2 ], this->_buffer, _size[ 0 ], _size[ 1 ], _size[ 2 ], _mapper );
      }

      /**
       @brief returns an iterator on the specified voxel
       */
      ConstDirectionalIterator getIterator( size_t x, size_t y, size_t z ) const
      {
         return ConstDirectionalIterator( _mapper.index( x, y, z ), this->_buffer, _size[ 0 ], _size[ 1 ], _size[ 2 ], _mapper );
      }

      /**
       @brief returns an iterator on the first voxel
       */
      DirectionalIterator beginDirectional()
      {
         return DirectionalIterator( 0, this->_buffer, _size[ 0 ], _size[ 1 ], _size[ 2 ], _mapper );
      }

      /**
       @brief returns an iterator on the last voxel + 1
       @note this is a costly operation! it must be cahced
       */
      DirectionalIterator endDirectional()
      {
         return DirectionalIterator( _size[ 0 ] * _size[ 1 ] * _size[ 2 ], this->_buffer, _size[ 0 ], _size[ 1 ], _size[ 2 ], _mapper );
      }

      /**
       @brief returns an iterator on the specified voxel
       */
      DirectionalIterator getIterator( size_t x, size_t y, size_t z )
      {
         return DirectionalIterator( _mapper.index( x, y, z ), this->_buffer, _size[ 0 ], _size[ 1 ], _size[ 2 ], _mapper );
      }

   protected:
      /**
       @brief return the position of the pixel (x, y, z) in the buffer
       */
      inline size_t index( const size_t x, const size_t y, const size_t z ) const
      {
         return _mapper.index( x, y, z );
      }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline const T at( const size_t x, const size_t y, const size_t z ) const
      {
         return this->_buffer[ index( x, y, z ) ];
      }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline T& at( const size_t x, const size_t y, const size_t z )
      {
         return this->_buffer[ index( x, y, z ) ];
      }

   protected:
      IndexMapper          _mapper;
      size_t                 _bufferSize;
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
      typedef T                                                value_type;
      typedef VolumeMemoryBufferType                           VoxelBuffer;

      typedef typename VolumeMemoryBufferType::DirectionalIterator        DirectionalIterator;
      typedef typename VolumeMemoryBufferType::ConstDirectionalIterator   ConstDirectionalIterator;

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
      Volume( size_t sx, size_t sy, size_t sz, T background = 0, bool zero = true ) : _buffer( sx, sy, sz, zero ), _background( background )
      {
      }

      /**
       @brief Copy the current volume. Internally a new buffer is created (and not shared with the volume)
       */
      void clone( const Volume& vol )
      {
         _buffer.clone( vol._buffer );
         _background = vol._background;
      }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline const T operator()( const size_t x, const size_t y, const size_t z ) const { return at( x, y, z ); }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline T& operator()( const size_t x, const size_t y, const size_t z ) { return at( x, y, z ); }

      /**
       @return the size of the volume
       */
      const core::vector3ui& size() const
      {
         return _buffer.getSize();
      }

      /**
       @brief Returns the size in x
       @note to be prefered over <core::vector3ui& size> as it saves the construction of a 3-vector
       */
      size_t sizex() const
      {
         return _buffer.sizex();
      }

      /**
       @brief Returns the size in y
       @note to be prefered over <core::vector3ui& size> as it saves the construction of a 3-vector
       */
      size_t sizey() const
      {
         return _buffer.sizey();
      }

      /**
       @brief Returns the size in z
       @note to be prefered over <core::vector3ui& size> as it saves the construction of a 3-vector
       */
      size_t sizez() const
      {
         return _buffer.sizez();
      }

      bool inside( double x, double y, double z ) const
      {
         return x >= 0 && y >= 0 && z >= 0 &&
                x < _buffer.sizex() &&
                y < _buffer.sizey() &&
                z < _buffer.sizex();
      }

      bool inside( int x, int y, int z ) const
      {
         return x >= 0 && y >= 0 && z >= 0 &&
                x < static_cast<int>( _buffer.sizex() ) &&
                y < static_cast<int>( _buffer.sizey() ) &&
                z < static_cast<int>( _buffer.sizez() );
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

      /**
       @brief returns a const iterator on the first voxel
       */
      ConstDirectionalIterator beginDirectional() const
      {
         return _buffer.beginDirectional();
      }

      /**
       @brief returns a const iterator on the last voxel + 1
       @note this is a costly operation! it must be cahced
       */
      ConstDirectionalIterator endDirectional() const
      {
         return _buffer.endDirectional();
      }

      /**
       @brief returns an iterator on the specified voxel
       */
      ConstDirectionalIterator getIterator( size_t x, size_t y, size_t z ) const
      {
         return _buffer.getIterator( x, y, z );
      }

      /**
       @brief returns a const iterator on the first voxel
       */
      DirectionalIterator beginDirectional()
      {
         return _buffer.beginDirectional();
      }

      /**
       @brief returns an iterator on the last voxel + 1
       @note this is a costly operation! it must be cahced
       */
      DirectionalIterator endDirectional()
      {
         return _buffer.endDirectional();
      }

      /**
       @brief returns an iterator on the specified voxel
       */
      DirectionalIterator getIterator( size_t x, size_t y, size_t z )
      {
         return _buffer.getIterator( x, y, z );
      }

      /**
       @brief Fill the buffer with a value
       */
      void fill( const T value )
      {
         for ( typename VoxelBuffer::iterator it = _buffer.begin(); it != _buffer.end(); ++it )
         {
            *it = value;
         }
      }

      /**
       @brief Return the internal storage
       */
      const VoxelBuffer& getStorage() const
      {
         return _buffer;
      }

      /**
       @brief Return the internal storage
       */
      VoxelBuffer& getStorage()
      {
         return _buffer;
      }

   protected:
      /**
       @brief return the value at the point (x, y, z)
       */
      inline const T at( const size_t x, const size_t y, const size_t z ) const
      {
         return _buffer( x, y, z );
      }

      /**
       @brief return the value at the point (x, y, z)
       */
      inline T& at( const size_t x, const size_t y, const size_t z )
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
