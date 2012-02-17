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

#ifndef NLL_IMAGE_SPATIAL_H_
# define NLL_IMAGE_SPATIAL_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Define a 2D image with spatial transformation associated
    */
   template <class T, class Mapper = IndexMapperRowMajorFlat2DColorRGBn, class Alloc = std::allocator<T> >
   class ImageSpatial : public Image<T, Mapper, Alloc>
   {
      typedef Image<T, Mapper, Alloc> Base;

   public:
      typedef core::Matrix<float>   Matrix;

      ImageSpatial( ui32 sizex, ui32 sizey, ui32 nbComponents, const Matrix& tfm, bool zero = true, Alloc alloc = Alloc() ) : Base( sizex, sizey, nbComponents, zero, alloc ), _geometry( tfm )
      {}

      ImageSpatial( Base& b, const Matrix& tfm ) : Base( b ), _geometry( tfm )
      {}

      ImageSpatial( Alloc alloc = Alloc() ) : Base( alloc ), _geometry( core::identityMatrix<Matrix>( 3 ) )
      {}

      core::vector2f indexToPosition( const core::vector2f& index ) const
      {
         return _geometry.indexToPosition( index );
      }

      core::vector2f positionToIndex( const core::vector2f& position ) const
      {
         return _geometry.positionToIndex( position );
      }

      const Matrix& getPst() const
      {
         return _geometry.getPst();
      }

      const Matrix& getInvertedPst() const
      {
         return _geometry.getInvertedPst();
      }

      core::vector2f getOrigin() const
      {
         return core::vector2f( getPst()( 0, 2 ), getPst()( 1, 2 ) );
      }

   private:
      VolumeGeometry2d _geometry;
   };
}
}

#endif
