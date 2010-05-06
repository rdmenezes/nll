#ifndef NLL_IMAGING_VOLUME_SPATIAL_H_
# define NLL_IMAGING_VOLUME_SPATIAL_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief handle a medical volume

    The (0, 0) of a voxel is the center of the voxel.
    */
   template <class T, class VolumeMemoryBufferType = VolumeMemoryBuffer<T> >
   class VolumeSpatial : public Volume<T, VolumeMemoryBufferType>
   {
   public:
      typedef core::Matrix<float>               Matrix;
      typedef Volume<T, VolumeMemoryBufferType> Base;
      typedef T                                 value_type;

   public:

      /**
       @brief build an empty medical volume
       */
      VolumeSpatial()
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
                  The origin is defined as the vector from the world origin to the voxel (0, 0, 0) of the volume,
                  distance in minimeter
                  <p>
                  The spacing is extracted from the matrix: s_x = sqrt( r1^2 + r4^2 + r7^2 ) and so on
                  for s_y and s_z spacing.
                  <p>
                  Positive rotation is clockwise.
       */
      VolumeSpatial( const core::vector3ui& size,
                     const Matrix& pst,
                     T background = 0,
                     bool zero = true ) : Base( size[ 0 ], size[ 1 ], size[ 2 ], background, zero ), _pst( pst )
      {
         _constructVolume();
      }

      /**
       @brief Given a voxel index, it will be returned a position in Patient Coordinate system.
       */
      core::vector3f indexToPosition( const core::vector3f& index ) const
      {
         core::vector3f result( 0, 0, 0 );
         for ( unsigned n = 0; n < 3; ++n )
         {
            result[ 0 ] += index[ n ] * _pst( 0, n );
            result[ 1 ] += index[ n ] * _pst( 1, n );
            result[ 2 ] += index[ n ] * _pst( 2, n );
         }
         return core::vector3f( result[ 0 ] + _pst( 0, 3 ),
                                result[ 1 ] + _pst( 1, 3 ),
                                result[ 2 ] + _pst( 2, 3 ) );
      }

      /**
       @brief Given a position in Patient Coordinate system, it will be returned a voxel
              position. The integer part represent the voxel coordinate, the real part represents
              how far is the point from this voxel.
       */
      core::vector3f positionToIndex( const core::vector3f& position ) const
      {
         core::vector3f result( 0, 0, 0 );
         for ( unsigned n = 0; n < 3; ++n )
         {
            result[ 0 ] += ( position[ n ] - _pst( n, 3 ) ) * _invertedPst( 0, n );
            result[ 1 ] += ( position[ n ] - _pst( n, 3 ) ) * _invertedPst( 1, n );
            result[ 2 ] += ( position[ n ] - _pst( n, 3 ) ) * _invertedPst( 2, n );
         }
         return result;
      }

      /**
       @return the origin of the image
       */
      core::vector3f getOrigin() const
      {
         return core::vector3f( _pst( 0, 3 ),
                                _pst( 1, 3 ),
                                _pst( 2, 3 ) );
      }

      /**
       @brief set the new origin
       */
      void setOrigin( const core::vector3f& o )
      {
         _pst( 0, 3 ) = o[ 0 ];
         _pst( 1, 3 ) = o[ 1 ];
         _pst( 2, 3 ) = o[ 2 ];

         // reset the inverted PST
         _invertedPst.clone( _pst );
         bool inversed = core::inverse( _invertedPst );
         ensure( inversed, "error: the PST is singular, meaning the pst is malformed" );
      }

      /**
       @return the spacing of the image
       */
      const core::vector3f& getSpacing() const
      {
         return _spacing;
      }

      /**
       @brief set a new spacing
       */
      void setSpacing( const core::vector3f& sp )
      {
         assert( sp[ 0 ] > 0 && sp[ 1 ] > 0 && sp[ 2 ] > 0 );

         // normalize the PST
         for ( ui32 n = 0; n < 3; ++n )
         {
            _pst( 0, n ) /= _spacing[ n ];
            _pst( 1, n ) /= _spacing[ n ];
            _pst( 2, n ) /= _spacing[ n ];
         }

         // apply the new scaling
         for ( ui32 n = 0; n < 3; ++n )
         {
            _pst( 0, n ) *= sp[ n ];
            _pst( 1, n ) *= sp[ n ];
            _pst( 2, n ) *= sp[ n ];
         }
         _spacing = sp;

         // reset the inverted PST
         _invertedPst.clone( _pst );
         bool inversed = core::inverse( _invertedPst );
         ensure( inversed, "error: the PST is singular, meaning the pst is malformed" );
      }

      /**
       @brief returns the rotational part of the PST
       */
      Matrix getRotation() const
      {
         Matrix m( 3, 3 );
         // normalize the PST
         for ( ui32 n = 0; n < 3; ++n )
         {
            m( 0, n ) = _pst( 0, n ) / _spacing[ n ];
            m( 1, n ) = _pst( 1, n ) / _spacing[ n ];
            m( 2, n ) = _pst( 2, n ) / _spacing[ n ];
         }

         return m;
      }

      /**
       @brief sets the rotational part of the PST. It must be a 3x3 matrix. There must be not scaling
       */
      void setRotation( const Matrix& m )
      {
         assert( m.sizex() == 3 && m.sizey() == 3 );

         // normalize the PST
         for ( ui32 n = 0; n < 3; ++n )
         {
            _pst( 0, n ) = m( 0, n ) * _spacing[ n ];
            _pst( 1, n ) = m( 1, n ) * _spacing[ n ];
            _pst( 2, n ) = m( 2, n ) * _spacing[ n ];
         }

         _invertedPst.clone( _pst );
         bool inversed = core::inverse( _invertedPst );
         ensure( inversed, "error: the PST is singular, meaning the pst is malformed" );
      }

      /**
       @return the size of the volume
       */
      const core::vector3ui& getSize() const
      {
         return Base::size();
      }

      /**
       @return patient scale transform
       */
      const Matrix& getPst() const
      {
         return _pst;
      }

      /**
       @brief Set a new PST
       */
      void setPst( const Matrix& m )
      {
         _pst = m;

         _constructVolume();
      }

      /**
       @return return the inversed Pst
       */
      const Matrix& getInvertedPst() const
      {
         return _invertedPst;
      }

      /**
       @brief save the volume to a stream
       */
      bool write( std::ostream& f ) const
      {
         Base::write( f );
         _pst.write( f );
         return true;
      }

      /**
       @brief read the volume from a stream
       */
      bool read( std::istream& f )
      {
         Base::read( f );
         _pst.read( f );

         _constructVolume();
         return true;
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

         _invertedPst.clone( _pst );
         bool inversed = core::inverse( _invertedPst );
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
      static Matrix createPatientSpaceTransform( const Matrix& rotation3x3, const core::vector3f& origin, const core::vector3f& spacing )
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
      core::vector3f _spacing;
      Matrix         _invertedPst;
   };
}
}

#endif
