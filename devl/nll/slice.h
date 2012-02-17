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
    @note the 2D slice coordinate system is centered on the slice origin: (0, 0) = sliceOrigin
    */
   template <class T>
   class Slice
   {
   public:
      typedef core::Image<T> Storage;

   public:
      // defines interpolators
      typedef core::InterpolatorLinear2D<typename Storage::value_type, typename Storage::IndexMapper, typename Storage::Allocator>           BilinearInterpolator;
      typedef core::InterpolatorNearestNeighbor2D<typename Storage::value_type, typename Storage::IndexMapper, typename Storage::Allocator>  NearestNeighbourInterpolator;

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

       @param size the number of voxels. The third component indicates the dimenstionality of the value.
       @param axisx the x axis of the slice
       @param axisy the y axis of the slice
       @param origin the origin (vector (0, 0, 0) in world coordinate to the center of the slice in mm)
       @param spacing of the pixels in mm
       */
      Slice( const core::vector3ui& size,
             const core::vector3f& axisx,
             const core::vector3f& axisy,
             const core::vector3f& origin,
             const core::vector2f& spacing ) :
         _storage( size[ 0 ], size[ 1 ], size[ 2 ], false )
      {
         setGeometry( axisx, axisy, origin, spacing );
      }

      /**
       @brief set a new slice geometry
       */
      void setGeometry( const core::vector3f& axisx,
                        const core::vector3f& axisy,
                        const core::vector3f& origin,
                        const core::vector2f& spacing )
      {
         _axisx = axisx;
         _axisy = axisy;
         _spacing = spacing;

         // normalize
         float sx = static_cast<float>( _axisx.norm2() );
         float sy = static_cast<float>( _axisy.norm2() );
         _axisx = _axisx / sx;
         _axisy = _axisy / sy;

         assert( spacing[ 0 ] > 0 && spacing[ 1 ] > 0 );
         _plane = core::GeometryPlane( origin, _axisx * spacing[ 0 ], _axisy * spacing[ 1 ] );
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
         return _plane.getOrigin();
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
         return _plane.getNormal();
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
       @brief Returns true if the point, in world coordinate (in mm) is in the same plane defined by
              this slice.
       */
      bool isInPlane( const core::vector3f pos, float tol = 1e-4 ) const
      {
         return _plane.contains( pos, tol );
      }

      /**
       @brief Returns true if the position in mm is in the slice
       */
      bool contains( float x, float y, float z, float tol = 1e-4 ) const
      {
         return contains( core::vector3f( x, y, z, tol ), tol );
      }

      /**
       @brief Returns true if the position in mm is in the slice
       */
      bool contains( const core::vector3f pos, float tol = 1e-4 ) const
      {
         if ( !_plane.contains( pos, tol ) )
            return false;

         // get the point in the slice coordinate system
         // compare is < or > slice.size * spacing / 2
         try
         {
            core::vector2f slicePos = worldToSliceCoordinate( pos );
            const float sx = static_cast<float>( this->size()[ 0 ] ) / 2;
            const float sy = static_cast<float>( this->size()[ 1 ] ) / 2;
            if ( slicePos[ 0 ] - 1 <= -sx ||
                 slicePos[ 0 ] + 1 >= sx ||
                 slicePos[ 1 ] - 1 <= -sy ||
                 slicePos[ 1 ] + 1 >= sy )
              return false;
            return true;
         } catch (...)
         {
            // something wrong happened...
            // it should not be the case if the slice geometry is correctly defined
            return false;
         }
      }

      bool contains( const core::vector2f pos ) const
      {
         const float sx = static_cast<float>( this->size()[ 0 ] ) / 2;
         const float sy = static_cast<float>( this->size()[ 1 ] ) / 2;
         if ( pos[ 0 ] - 1 <= -sx ||
              pos[ 0 ] + 1 >= sx ||
              pos[ 1 ] - 1 <= -sy ||
              pos[ 1 ] + 1 >= sy )
           return false;
         return true; 
      }

      /**
       @brief Deep copy the slice
       */
      Slice& clone( const Slice& rhs )
      {
         _axisx = rhs._axisx;
         _axisy = rhs._axisy;
         _spacing = rhs._spacing;
         _plane = rhs._plane;
         _storage.clone( rhs._storage );
         return *this;
      }

      /**
       @brief Copy the slice. the internal storage
       */
      Slice& operator=( const Slice& rhs )
      {
         _axisx = rhs._axisx;
         _axisy = rhs._axisy;
         _spacing = rhs._spacing;
         _storage = rhs._storage;
         _plane = rhs._plane;
         return *this;
      }

      /**
       @brief Returns the internal storage
       */
      Storage& getStorage()
      {
         return _storage;
      }

      /**
       @brief Returns the internal storage
       */
      const Storage& getStorage() const
      {
         return _storage;
      }

      /**
       @brief Transform a world coordinate (standard x, y, z coordinate system, in mm) to slice coordinate, with (0, 0) the center on the origin (=center of the slice)
       @param v a position in world coordinate in mm. It must be located on the plane of the slice!
       */
      core::vector2f worldToSliceCoordinate( const core::vector3f& v ) const
      {
         return _plane.worldToPlaneCoordinate( v );
      }

      /**
       @brief Transform the slice coordinate to a 3D point in world coordinate (in mm)
       */
      core::vector3f sliceToWorldCoordinate( const core::vector2f& v ) const
      {
         return _plane.planeToWorldCoordinate( v );
      }

      /**
       @brief Get the orthogonal projection of a point on the plane of the slice
       */
      core::vector3f getOrthogonalProjection( const core::vector3f& p ) const
      {
         return _plane.getOrthogonalProjection( p );
      }

   protected:
      Storage              _storage;
      core::vector3f       _axisx;
      core::vector3f       _axisy;
      core::vector2f       _spacing;
      core::GeometryPlane  _plane;
   };
}
}

#endif