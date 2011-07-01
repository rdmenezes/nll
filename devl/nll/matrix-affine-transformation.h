/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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

#ifndef NLL_TRANSFORMATION_AFFINE_2D_H_
# define NLL_TRANSFORMATION_AFFINE_2D_H_

namespace nll
{
namespace core
{
   /**
    @brief Create a 2D transformation matrix, given a rotation, scaling and translation parameters
    @param roation the rotation angle from the x-axis in radian

    the matrix has this format:
                       | scalingx * rx0 ; scalingy * ry0 ; tx |
                   M = | scalingx * rx1 ; scalingy * ry1 ; ty |
                       |     0          ;     0          ; 1  |
    */
   inline core::Matrix<double>
   createTransformationAffine2D( double rotation,
                                 const core::vector2d& scaling,
                                 const core::vector2d& translation )
   {
      const double co = cos( rotation );
      const double si = sin( rotation );
      core::Matrix<double> tfm( 3, 3, false );
      tfm( 0, 0 ) =  co * scaling[ 0 ];
      tfm( 0, 1 ) = -si * scaling[ 1 ];
      tfm( 0, 2 ) =  translation[ 0 ];
      tfm( 1, 0 ) =  si * scaling[ 0 ];
      tfm( 1, 1 ) =  co * scaling[ 1 ];
      tfm( 1, 2 ) =  translation[ 1 ];
      tfm( 2, 0 ) =  0;
      tfm( 2, 1 ) =  0;
      tfm( 2, 2 ) =  1;

      return tfm;
   }

   /**
    @brief Returns the roation contained in a 4x4 affine transformation
    */
   template <class type, class mapper, class allocator>
   Matrix<type, mapper, allocator> getRotation4x4( const Matrix<type, mapper, allocator>& m )
   {
      ensure( m.sizex() == 4 && m.sizey() == 4, "must be a 4x4 matrix" );
      StaticVector<type, 3> spacing;
      for ( ui32 x = 0; x < 3; ++x )
      {
         spacing[ x ] = std::sqrt( core::sqr( m( 0, x ) ) +
                                   core::sqr( m( 1, x ) ) +
                                   core::sqr( m( 2, x ) ) );
      }

      Matrix<type, mapper, allocator> rot( 4, 4 );
      for ( ui32 y = 0; y < 3; ++y )
      {
         for ( ui32 x = 0; x < 3; ++x )
         {
            rot( y, x ) = m( y, x ) / spacing[ x ];
         }
      }
      rot( 3, 3 ) = 1;
      return rot;
   }

   /**
    @brief Assuming a 4x4 transformation matrix defined as
                 | Rx Ry Rz Tx |
           Tfm = | Rx Ry Rz Ty |
                 | Rx Ry Rz Tz |
                 | 0  0  0  1  |
           Returns an homogeneous 4x4 matrix with only the R part
    */
   template <class type, class mapper, class allocator>
   Matrix<type, mapper, allocator> getRotationAndSpacing( const Matrix<type, mapper, allocator>& m )
   {
      ensure( m.sizex() == 4 && m.sizey() == 4, "must be a 4x4 matrix" );

      Matrix<type, mapper, allocator> rot( 4, 4 );
      for ( ui32 y = 0; y < 3; ++y )
      {
         for ( ui32 x = 0; x < 3; ++x )
         {
            rot( y, x ) = m( y, x );
         }
      }
      rot( 3, 3 ) = 1;
      return rot;
   }


   /**
    @brief Assuming a 4x4 transformation matrix defined as
                 | 1 0 0 Tx |
           Tfm = | 0 1 0 Ty |
                 | 0 0 1 Tz |
                 | 0 0 0 1  |
           Returns an homogeneous 4x4 matrix with only the T part
    */
   template <class type>
   Matrix<type> createTranslation4x4( const StaticVector<type, 3>& translation )
   {
      Matrix<type> rot = identityMatrix< Matrix<type> >( 4 );
      rot( 0, 3 ) = translation[ 0 ];
      rot( 1, 3 ) = translation[ 1 ];
      rot( 2, 3 ) = translation[ 2 ];
      return rot;
   }

   /**
    @brief Returns the spacing of a 4x4 homogeneous transformation matrix
    */
   template <class type, class mapper, class allocator>
   vector3f getSpacing4x4( const Matrix<type, mapper, allocator>& m )
   {
      ensure( m.sizex() == 4 && m.sizey() == 4, "must be a 4x4 matrix" );
      vector3f spacing;
      for ( ui32 x = 0; x < 3; ++x )
      {
         spacing[ x ] = std::sqrt( core::sqr( m( 0, x ) ) +
                                   core::sqr( m( 1, x ) ) +
                                   core::sqr( m( 2, x ) ) );
      }
      return spacing;
   }
}
}

#endif