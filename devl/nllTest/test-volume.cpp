#include "stdafx.h"
#include <nll/nll.h>

namespace nll
{
namespace imaging
{
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

      private:
         ui32  _sxy;
         ui32  _sizex;
         ui32  _sizey;
         ui32  _sizez;
   };

   /**
    @ingroup imaging
    @brief Abstract the voxel buffer. The voxel buffer must share memory between instances.

    The mapper must provide an index given a 3D coordinate. It is initialized with the volume dimension.
    */
   template <class T, class Mapper = IndexMapperRowMajorFlat3D>
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
   protected:
      typedef VolumeMemoryBufferType   VoxelBuffer;

   public:
      typedef typename VolumeMemoryBufferType::iterator        iterator;
      typedef typename VolumeMemoryBufferType::const_iterator  const_iterator;

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
      Volume( ui32 sx, ui32 sy, ui32 sz, bool zero = true ) : _buffer( sx, sy, sz, zero )
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
   };

   /**
    @brief handle a medical volume
    */
   template <class T, class VolumeMemoryBufferType = VolumeMemoryBuffer<T> >
   class VolumeSpacial : public Volume<T, VolumeMemoryBufferType>
   {
   public:
      typedef core::Matrix<double>              Matrix;
      typedef Volume<T, VolumeMemoryBufferType> Base;
      typedef T                                 value_type;

   public:
      /**
       @brief build an empty medical volume
       */
      VolumeSpacial()
      {
      }

      /**
       @brief build a volume with a spacial location using a specific size, patient space transform and background value.
       @param pst a 4x4 matrix. It defines the rotation, translation and spacing which transforms
                  volume coordinate system into the patient coordinate system.
                  <p>
                  The matrix should be mapped like this:
                  [ r1 r2 r3 ox ]
                  [ r4 r5 r6 oy ]
                  [ r7 r8 r9 oz ]
                  [ 0  0  0  1  ]
                  <p>
                  With R the rotation matrix, and O the image origin in mm, S the image size
                  <p>
                  The origin is defined as the vector from the voxel (0, 0, 0) of the volume
                  to the world origin, distance in minimeter
                  <p>
                  The spacing is extracted from the matrix: s_x = sqrt( r1^2 + r4^2 + r7^2 ) and so on
                  for s_y and s_z spacing.
       */
      VolumeSpacial( const core::vector3ui& size,
                     const Matrix& pst,
                     T background = 0 ) : Base( size[ 0 ], size[ 1 ], size[ 2 ] ), _pst( pst ), _background( background )
      {
         _constructVolume();
      }

      /**
       @brief return the background value
       */
      value_type getBackgroundValue() const
      {
         return backgroundValue;
      }

      /**
       @brief Given a voxel index, it will be returned a position in Patient Coordinate system.
       */
      core::vector3d indexToPosition( const core::vector3d& index ) const
      {
         core::vector3d result( 0, 0, 0 );
         for ( unsigned n = 0; n < 3; ++n )
         {
            result[ 0 ] += index[ n ] * _pst( 0, n );
            result[ 1 ] += index[ n ] * _pst( 1, n );
            result[ 2 ] += index[ n ] * _pst( 2, n );
         }
         return core::vector3d( result[ 0 ] - _pst( 0, 3 ),
                                result[ 1 ] - _pst( 1, 3 ),
                                result[ 2 ] - _pst( 2, 3 ) );
      }

      /**
       @brief Given a position in Patient Coordinate system, it will be returned a voxel
              position. The integer part represent the voxel coordinate, the real part represents
              how far is the point from this voxel.
       */
      core::vector3d positionToIndex( const core::vector3d& position ) const
      {
         core::vector3d result( 0, 0, 0 );
         for ( unsigned n = 0; n < 3; ++n )
         {
            result[ 0 ] += ( position[ n ] + _pst( n, 3 ) ) * _inversedPst( 0, n );
            result[ 1 ] += ( position[ n ] + _pst( n, 3 ) ) * _inversedPst( 1, n );
            result[ 2 ] += ( position[ n ] + _pst( n, 3 ) ) * _inversedPst( 2, n );
         }
         return result;
      }

      /**
       @return the origin of the image
       */
      core::vector3d getOrigin() const
      {
         return core::vector3d( _pst( 0, 3 ),
                                _pst( 1, 3 ),
                                _pst( 2, 3 ) );
      }

      /**
       @return the spacing of the image
       */
      const core::vector3d& getSpacing() const
      {
         return _spacing;
      }

   protected:
      /**
       @brief Construct the voluem: compute the spacing and inversed pst
       */
      void _constructVolume()
      {
         ensure( _pst.sizex() == 4 &&
                 _pst.sizey() == 4, "Invalid PST. Must be a 4x4 matrix" );
         for ( ui32 n = 0; n < 3; ++n )
         {
            // the spacing can be extracted by computing the norm-2 of each column of a rotation matrix - see OpenGL ref
            _spacing[ n ] = sqrt( _pst( 0, n ) * _pst( 0, n ) +
                                  _pst( 1, n ) * _pst( 1, n ) +
                                  _pst( 2, n ) * _pst( 2, n ) );
         }

         _inversedPst.clone( _pst );
         bool inversed = core::inverse( _inversedPst );
         ensure( inversed, "error: the PST is singular, meaning the pst is malformed" );
      }

   public:
      /**
       @brief create a PST matrix
       @param rotation3x3 is a 3x3 rotation matrix used as input
              the layout should be:
              | x1 x2 x3 |
              | y1 y2 y3 |
              | z1 z2 z3 |
              With (Col1, Col2, Col3) orthonormal vectors
       @param origin is the vector from voxel(0, 0, 0) to world origin(0, 0, 0) in minimeter
       @param spacing the spacing in each dimension x, y, z
       @return is the 4x4 transformation matrix used to construct the ImageWrapper object.
       */
      static Matrix createPatientSpaceTransform( const Matrix& rotation3x3, const core::vector3d& origin, const core::vector3d& spacing )
      {
         ensure( spacing[ 0 ] > 0, "must be >0" );
         ensure( spacing[ 1 ] > 0, "must be >0" );
         ensure( spacing[ 2 ] > 0, "must be >0" );

         Matrix spacingMat( 3, 3 );
         spacingMat( 0, 0 ) = spacing[ 0 ];
         spacingMat( 1, 1 ) = spacing[ 1 ];
         spacingMat( 2, 2 ) = spacing[ 2 ];

         Matrix tmp = rotation3x3 * spacingMat;

         Matrix pst( 4, 4 );
         pst( 0, 3 ) = origin[ 0 ];
         pst( 1, 3 ) = origin[ 1 ];
         pst( 2, 3 ) = origin[ 2 ];

         for ( ui32 ny = 0; ny < 3; ++ny )
            for ( ui32 nx = 0; nx < 3; ++nx )
               pst( ny, nx ) = tmp( ny, nx );
         pst( 3, 3 ) = 1;
         return pst;
      }

   protected:
      Matrix         _pst;
      T              _background;
      core::vector3d _spacing;
      Matrix         _inversedPst;
   };
}
}

class TestVolume
{
public:
   /**
    Test basic voxel buffer index
    */
   void testBuffer1()
   {
      typedef nll::imaging::VolumeMemoryBuffer<double>   Buffer;

      Buffer buffer( 5, 10, 15 );
      double s = 0;
      for ( Buffer::iterator it = buffer.begin(); it != buffer.end(); ++it )
         *it = ++s;

      s = 0;
      for ( Buffer::const_iterator it = buffer.begin(); it != buffer.end(); ++it )
         TESTER_ASSERT( nll::core::equal( ++s, *it ) );

      buffer( 1, 2, 3 ) = 5.5;
      TESTER_ASSERT( nll::core::equal( buffer( 1, 2, 3 ), 5.5 ) );

      TESTER_ASSERT( buffer.getSize() == nll::core::vector3ui( 5, 10, 15 ) );
   }

   /**
    Test test volume sharing memory
    */
   void testVolume1()
   {
      typedef nll::imaging::Volume<int>   Volume;

      Volume vol;
      vol = Volume( 10, 5, 30 );
      vol( 0, 0, 0 ) = -5;
      TESTER_ASSERT( vol.size() == nll::core::vector3ui( 10, 5, 30 ) );
      TESTER_ASSERT( vol( 0, 0, 0 ) == -5 );

      Volume vol2( 40, 50, 10 );
      vol2( 0, 0, 0 ) = -15;
      TESTER_ASSERT( vol2( 0, 0, 0 ) == -15 );

      vol2 = vol;
      TESTER_ASSERT( vol2( 0, 0, 0 ) == -5 );

      Volume vol3;
      vol3.clone( vol2 );
      TESTER_ASSERT( vol3( 0, 0, 0 ) == -5 );

      vol3( 0, 0, 0 ) = 10;
      TESTER_ASSERT( vol3( 0, 0, 0 ) == 10 );
      TESTER_ASSERT( vol2( 0, 0, 0 ) == -5 );
      TESTER_ASSERT( vol( 0, 0, 0 ) == -5 );
   }

   void testVolumeIterator()
   {
      typedef nll::imaging::Volume<int>   Volume;
      Volume buffer( 5, 10, 15 );
      int s = 0;
      for ( Volume::iterator it = buffer.begin(); it != buffer.end(); ++it )
         *it = ++s;

      s = 0;
      for ( Volume::const_iterator it = buffer.begin(); it != buffer.end(); ++it )
         TESTER_ASSERT( nll::core::equal( ++s, *it ) );
   }

   void testVolumeSpacial1()
   {
      typedef nll::imaging::VolumeSpacial<double>  Vol;

      Vol volume;

      Vol::Matrix pst;
      Vol::Matrix rot3x3 = nll::core::identity<double, Vol::Matrix::IndexMapper>( 3 );

      pst = Vol::createPatientSpaceTransform( rot3x3, nll::core::vector3d( -10, 5, 30 ), nll::core::vector3d( 10, 20, 30 ) );
      volume = Vol( nll::core::vector3ui( 10, 20, 30 ), pst );
      TESTER_ASSERT( nll::core::equal<double>( pst( 0, 3 ), -10 ) );
      TESTER_ASSERT( nll::core::equal<double>( pst( 1, 3 ), 5 ) );
      TESTER_ASSERT( nll::core::equal<double>( pst( 2, 3 ), 30 ) );

      TESTER_ASSERT( nll::core::equal<double>( pst( 0, 0 ), 10 ) );
      TESTER_ASSERT( nll::core::equal<double>( pst( 1, 1 ), 20 ) );
      TESTER_ASSERT( nll::core::equal<double>( pst( 2, 2 ), 30 ) );

      TESTER_ASSERT( volume.getOrigin() == nll::core::vector3d( -10, 5, 30 ) );
      TESTER_ASSERT( volume.getSpacing() == nll::core::vector3d( 10, 20, 30 ) );
   }

   void testIndexToPos()
   {
      for ( unsigned nn = 0; nn < 100; ++nn )
      {
         nll::core::vector3ui size( 40, 50, 35 );
         typedef nll::imaging::VolumeSpacial<double>  Volume; 

         // define a random rotation matrix around x axis
         // define a random translation vector in [0..99]
         // define a random spacing [2, 0.01]
         double a = static_cast<double>( rand() ) / RAND_MAX * 3.141 / 2;

         nll::core::vector3d spacingT( rand() % 2 / (double)10 + 0.01,
                                       rand() % 2 / (double)10 + 0.01,
                                       rand() % 2 / (double)10 + 0.01 );
         nll::core::vector3d origin( rand() % 100,
                                     rand() % 100,
                                     rand() % 100 );
         nll::core::Matrix<double> rotation( 3, 3 );
         rotation( 0, 0 ) = 1;
         rotation( 1, 1 ) = cos( a );
         rotation( 2, 1 ) = sin( a );
         rotation( 1, 2 ) = -sin( a );
         rotation( 2, 2 ) = cos( a );

         nll::core::Matrix<double> pst = Volume::createPatientSpaceTransform( rotation, origin, spacingT );

         // create the wrapper
         Volume image( size, pst );
         for ( Volume::iterator it = image.begin(); it != image.end(); ++it )
            *it = rand() % 5000;
/*
         Vector3d spacing = image.getSpacing();
         for ( unsigned n = 0; n < 100; ++n )
         {
            Vector3d index( rand() % ( size[ 0 ] - 1 ),
                            rand() % ( size[ 1 ] - 1 ),
                            rand() % ( size[ 2 ] - 1 ) );

            Vector3d position = image.indexToPosition( index );
            Vector3d indexTransf = image.positionToIndex( position );
            CPPUNIT_ASSERT( fabs( index[ 0 ] - indexTransf[ 0 ] ) < 1e-8 );
            CPPUNIT_ASSERT( fabs( index[ 1 ] - indexTransf[ 1 ] ) < 1e-8 );
            CPPUNIT_ASSERT( fabs( index[ 2 ] - indexTransf[ 2 ] ) < 1e-8 );
         }
         */
      }
   }
};

//#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolume);
TESTER_TEST(testBuffer1);
TESTER_TEST(testVolume1);
TESTER_TEST(testVolumeIterator);
TESTER_TEST(testVolumeSpacial1);
TESTER_TEST(testIndexToPos);
TESTER_TEST_SUITE_END();
//#endif