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