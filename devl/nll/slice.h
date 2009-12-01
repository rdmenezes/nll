#ifndef NLL_IMAGING_SLICE_H_
# define NLL_IMAGING_SLICE_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Defines a single slice. A slice is a 2D image, with a position, orientation and spacing info.
    @note copy by value should be quick, hence storage is shared accross slices when copied
    */
   template <class T>
   class Slice
   {
   public:
      typedef core::Image<T> Storage;

   public:
      typedef core::Matrix<f32>                                            Matrix;
      typedef typename Storage::DirectionalIterator                        DirectionalIterator;
      typedef typename Storage::ConstDirectionalIterator                   ConstDirectionalIterator;
      typedef typename Storage::iterator                                   iterator;
      typedef typename Storage::const_iterator                             const_iterator;
      typedef T                                                            value_type;

   public:
      Slice()
      {
      }

      /**
       @brief Construct a slice. The storage is allocated but not initialized.

       @param size the number of voxels. The third component indicated the dimenstionality of the value.
       @param axisx the x axis of the slice
       @param axisy the y axis of the slice
       @param origin the origin (vector (0, 0, 0) to the center of the slice in mm)
       @param spacing of the pixels in mm
       */
      Slice( const core::vector3ui& size,
             const core::vector3f& axisx,
             const core::vector3f& axisy,
             const core::vector3f& origin,
             const core::vector2f& spacing ) :
         _storage( size[ 0 ], size[ 1 ], size[ 2 ], false ), _axisx( axisx ), _axisy( axisy ), _origin( origin ), _spacing( spacing )
      {
         // normalize
         float sx = static_cast<float>( _axisx.norm2() );
         float sy = static_cast<float>( _axisy.norm2() );
         _axisx = _axisx / sx;
         _axisy = _axisy / sy;

         // computes the normal
         _orthonorm = core::cross( _axisx, _axisy );

         // computes constant
         _planed = - _origin.dot( _orthonorm );
      }

      /**
       @brief Returns the x-axis of the slice
       */
      const core::vector3f& getAxisX() const
      {
         return _axisx;
      }

      /**
       @brief Returns the y-axis of the slice
       */
      const core::vector3f& getAxisY() const
      {
         return _axisy;
      }

      /**
       @brief Returns the origin
       */
      const core::vector3f& getOrigin() const
      {
         return _origin;
      }

      /**
       @brief Returns the spacing
       */
      const core::vector2f& getSpacing() const
      {
         return _spacing;
      }

      /**
       @brief Returns the normal with norm( normal ) = 1
       */
      const core::vector3f& getNormal() const
      {
         return _orthonorm;
      }

      /**
       @brief Returns the value at this position
       @param x a pixel x coordinate
       @param y a pixel y coordinate
       @param col a pixel col coordinate
       */
      value_type operator()( ui32 x, ui32 y, ui32 col ) const
      {
         return _storage( x, y, col );
      }

      /**
       @brief Returns the value at this position
       @param x a pixel x coordinate
       @param y a pixel y coordinate
       @param col a pixel col coordinate
       */
      value_type& operator()( ui32 x, ui32 y, ui32 col )
      {
         return _storage( x, y, col );
      }

      /**
       @brief Returns a directional iterator on this position
       */
      DirectionalIterator getIterator( ui32 x, ui32 y )
      {
         return _storage.getIterator( x, y, 0 );
      }

      /**
       @brief Returns a directional iterator on this position
       */
      ConstDirectionalIterator getIterator( ui32 x, ui32 y ) const
      {
         return _storage.getIterator( x, y, 0 );
      }

      /**
       @brief Returns an iterator at the begining
       */
      iterator begin()
      {
         return _storage.begin();
      }

      /**
       @brief Returns an iterator at the end
       */
      iterator end()
      {
         return _storage.end();
      }

      /**
       @brief Returns a const iterator at the begining
       */
      const_iterator begin() const
      {
         return _storage.begin();
      }

      /**
       @brief Returns a const iterator at the end
       */
      const_iterator end() const
      {
         return _storage.end();
      }

      /**
       @brief Returns the size of the slice
       */
      const core::vector3ui size() const
      {
         return core::vector3ui( _storage.sizex(), _storage.sizey(), _storage.getNbComponents() );
      }

      /**
       @brief Returns true if the position in mm is in the slice
       */
      bool contains( float x, float y, float z, float tol = 1e-4 ) const
      {
         return core::vector3f( x, y, z, tol );
      }

      /**
       @brief Returns true if the position in mm is in the slice
       */
      bool contains( const core::vector3f pos, float tol = 1e-4 ) const
      {
         // check first it is in the same plan
         // N . x + d = 0 means we are in the plan
         bool isInPlan = ( fabs( _orthonorm.dot( pos ) + _planed ) < tol );
         if ( !isInPlan )
            return false;

         // transform world coordinates to slice coordinates

         return true;
      }

      Slice& clone( const Slice& rhs )
      {
         _axisx = rhs._axisx;
         _axisy = rhs._axisy;
         _origin = rhs._origin;
         _spacing = rhs._spacing;
         _orthonorm = rhs._orthonorm;
         _planed = rhs._planed;

         _storage.clone( rhs._storage );
         return *this;
      }

      Slice& operator=( const Slice& rhs )
      {
         _axisx = rhs._axisx;
         _axisy = rhs._axisy;
         _origin = rhs._origin;
         _spacing = rhs._spacing;
         _orthonorm = rhs._orthonorm;
         _planed = rhs._planed;
         _storage = rhs._storage;
         return *this;
      }

      Storage& getStorage()
      {
         return _storage;
      }

      const Storage& getStorage() const
      {
         return _storage;
      }

      /**
       @brief Transform a world coordinate (standard x, y, z coordinate system, in mm) to slice coordinate
       @param v a position in world coordinate in mm. It must be located on the slice!
       */
      core::vector2f worldToSliceCoordinate( const core::vector3f& v )
      {
         if ( !contains( v ) )
            throw std::exception( "error: the point is not on the slice" );

         // Let's have M a point on the slice plane with coordinate (x, y, z), O the the origin of the world U=(1, 0, 0) V=(0, 1, 0) W=(0, 0, 1) a base of the world coordinate system
         // O' origin of the slice, S and T base vectors of the slice. We are looking for (X, Y) coordinate in
         // slice coordinate system.

         // We have: 
         //   S = ax1 * U + ay1 * V + az1 * W
         //   T = ax2 * U + ay2 * V + az2 * W
         //   (ax1,ax2,ay1,ay2,az1,az2) are known
         // 
         // We also have:
         //   OM = OO' + O'M
         //
         //   OM = O' + X * S + Y * T
         //      = O' + X * ( ax1 * U + ay1 * V + az1 * W ) + Y * ( ax2 * U + ay2 * V + az2 * W )
         //
         //   { x = ox' + X * ax1 + Y * ax2
         //   { y = oy' + X * ay1 + Y * ay2
         //   { z = oz' + X * az1 + Y * az2
         //
         // S != 0, meaning ax1 | ay1 | az1 != 0, let's assume ax1 != 0 (we can choose ay1, az1)
         //   X = ( x - ox' - Y * ax2 ) / ax1
         //
         // S and T are not colinear AND T != 0, assuming ay2 != 0 (else we can choose ax2, az2)
         //   Y = ( y - oy' - X * ay1 ) / ay2
         //   Y = ( y - oy' - ay1 * ( x - ox' - Y * ax2 ) / ax1 ) / ay2
         //   Y = ay1 * ax2 * Y / ( ay2 * ax1 ) + ( y - oy' - ay1 * ( x - ox') / ax1 ) / ay2
         //   Y * ( ay2 * ax1 - ay1 * ax2 ) / ( ay2 * ax1 ) ) = ( y - oy' - ay1 * ( x - ox') / ax1 ) / ay2
         //   Y = ( y - oy' - ay1 * ( x - ox') / ax1 ) / ( ay2 * ( ay2 * ax1 - ay1 * ax2 ) )


         // init the data and it's index
         float m[ 3 ][ 4 ] =
         {
            { v[ 0 ], _origin[ 0 ], _axisx[ 0 ], _axisy[ 0 ] },
            { v[ 1 ], _origin[ 1 ], _axisx[ 1 ], _axisy[ 1 ] },
            { v[ 2 ], _origin[ 2 ], _axisx[ 2 ], _axisy[ 2 ] }
         };

         int i0 = -1;
         for ( ui32 n = 0; n < 3; ++n )
            if ( _axisx[ n ] != 0 )
            {
               i0 = n;
               break;
            }
         int i1 = -1;
         for ( ui32 n = 0; n < 3; ++n )
            if ( _axisy[ n ] != 0 && n != i0 )
            {
               i0 = n;
               break;
            }
         if ( i0 == -1 || i1 == -1 )
            throw std::exception( "error: the slice base is invalid" );

         const float x   = m[ i0 ][ 0 ];
         const float ox  = m[ i0 ][ 1 ];
         const float ax1 = m[ i0 ][ 2 ];
         const float ax2 = m[ i0 ][ 3 ];

         const float y   = m[ i1 ][ 0 ];
         const float oy  = m[ i1 ][ 1 ];
         const float ay1 = m[ i1 ][ 2 ];
         const float ay2 = m[ i1 ][ 3 ];

         if ( ( ay2 * ax1 - ay1 * ax2 ) == 0 )
            throw std::exception( "error: base is not valid, vectors are colinear" );

         const float Y = ( y - oy - ay1 * ( x - ox ) / ax1 ) / ( ay2 * ( ay2 * ax1 - ay1 * ax2 ) );
         const float X = ( x - ox - Y * ax2 ) / ax1;
         return core::vector2f( X, Y );
      }

   protected:
      Storage           _storage;
      core::vector3f    _axisx;
      core::vector3f    _axisy;
      core::vector3f    _origin;
      core::vector2f    _spacing;

      core::vector3f    _orthonorm;
      f32               _planed;
   };
}
}

#endif