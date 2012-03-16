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

#ifndef NLL_CORE_CUSTOM_H_
# define NLL_CORE_CUSTOM_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief A custom fast 3x3 multiplication with vector, computes: xA
    */

   template <class T, class Vector>
   inline Vector mat3Mulv( Vector& v, const core::Matrix<T, core::IndexMapperColumnMajorFlat2D>& m )
   {
      assert( m.sizex() == 3 && m.sizey() == 3 );
      return Vector( v[ 0 ] * m[ 0 ] + v[ 1 ] * m[ 3 ] + v[ 2 ] * m[ 6 ],
                     v[ 0 ] * m[ 1 ] + v[ 1 ] * m[ 4 ] + v[ 2 ] * m[ 7 ],
                     v[ 0 ] * m[ 2 ] + v[ 1 ] * m[ 5 ] + v[ 2 ] * m[ 8 ] );
   }

   /**
    @ingroup core
    @brief A custom fast 4x4 multiplication with vector, computes: xA
    */
   template <class T, class Vector>
   inline Vector mat4Mulv( Vector& v, const core::Matrix<T, core::IndexMapperColumnMajorFlat2D>& m )
   {
      assert( m.sizex() == 4 && m.sizey() == 4 );
      return Vector( v[ 0 ] * m[ 0 ] + v[ 1 ] * m[ 4 ] + v[ 2 ] * m[ 8 ]  + v[ 3 ] * m[ 12 ],
                     v[ 0 ] * m[ 1 ] + v[ 1 ] * m[ 5 ] + v[ 2 ] * m[ 9 ]  + v[ 3 ] * m[ 13 ],
                     v[ 0 ] * m[ 2 ] + v[ 1 ] * m[ 6 ] + v[ 2 ] * m[ 10 ] + v[ 3 ] * m[ 14 ],
                     v[ 0 ] * m[ 3 ] + v[ 1 ] * m[ 7 ] + v[ 2 ] * m[ 11 ] + v[ 3 ] * m[ 15 ] );
   }

   /**
    @ingroup core
    @brief A custom fast 3x3 multiplication with vector, computes: Ax
    */
   template <class T, class Vector>
   inline Vector mat3Mulv( const core::Matrix<T, core::IndexMapperColumnMajorFlat2D>& m, Vector& v )
   {
      assert( m.sizex() == 3 && m.sizey() == 3 );
      return Vector( v[ 0 ] * m[ 0 ] + v[ 1 ] * m[ 1 ] + v[ 2 ] * m[ 2 ],
                     v[ 0 ] * m[ 3 ] + v[ 1 ] * m[ 4 ] + v[ 2 ] * m[ 5 ],
                     v[ 0 ] * m[ 6 ] + v[ 1 ] * m[ 7 ] + v[ 2 ] * m[ 8 ] );
   }

   /**
    @ingroup core
    @brief A custom fast 4x4 multiplication with vector, computes: Ax
    */
   template <class T, class Vector>
   inline Vector mat4Mulv( const core::Matrix<T, core::IndexMapperColumnMajorFlat2D>& m, Vector& v )
   {
      assert( m.sizex() == 4 && m.sizey() == 4 );
      return Vector( v[ 0 ] * m[ 0 ] + v[ 1 ] * m[ 1 ] + v[ 2 ] * m[ 2 ] + v[ 3 ] * m[ 3 ],
                     v[ 0 ] * m[ 4 ] + v[ 1 ] * m[ 5 ] + v[ 2 ] * m[ 6 ] + v[ 3 ] * m[ 7 ],
                     v[ 0 ] * m[ 8 ] + v[ 1 ] * m[ 9 ] + v[ 2 ] * m[ 10 ]+ v[ 3 ] * m[ 11 ],
                     v[ 0 ] * m[ 12 ]+ v[ 1 ] * m[ 13 ]+ v[ 2 ] * m[ 14 ]+ v[ 3 ] * m[ 15 ] );
   }

   /**
    @ingroup core
    @brief A custom fast 3x3 inverse.
    @note the matrix is directly modified!
    */
   template <class T>
   inline bool inverse3x3( core::Matrix<T, core::IndexMapperColumnMajorFlat2D>& m )
   {
      
      assert( m.sizex() == 3 && m.sizey() == 3 );

      const T c = m[ 0 ] * ( m[ 4 ] * m[ 8 ] - m[ 7 ] * m[ 5 ] ) -
                  m[ 1 ] * ( m[ 8 ] * m[ 3 ] - m[ 5 ] * m[ 6 ] ) +
                  m[ 2 ] * ( m[ 7 ] * m[ 3 ] - m[ 4 ] * m[ 6 ] );

      if ( core::equal<T>( c, 0 ) )
         return false;

      // we need to use extra storage...
      const T a0 = ( m[ 8 ] * m[ 4 ] - m[ 5 ] * m[ 7 ] ) / c;
      const T a1 = ( m[ 2 ] * m[ 7 ] - m[ 8 ] * m[ 1 ] ) / c;
      const T a2 = ( m[ 5 ] * m[ 1 ] - m[ 2 ] * m[ 4 ] ) / c;

      const T a3 = ( m[ 5 ] * m[ 6 ] - m[ 8 ] * m[ 3 ] ) / c;
      const T a4 = ( m[ 8 ] * m[ 0 ] - m[ 2 ] * m[ 6 ] ) / c;
      const T a5 = ( m[ 2 ] * m[ 3 ] - m[ 5 ] * m[ 0 ] ) / c;

      const T a6 = ( m[ 7 ] * m[ 3 ] - m[ 4 ] * m[ 6 ] ) / c;
      const T a7 = ( m[ 6 ] * m[ 1 ] - m[ 0 ] * m[ 7 ] ) / c;
      const T a8 = ( m[ 0 ] * m[ 4 ] - m[ 3 ] * m[ 1 ] ) / c;
      
      // update the matrix
      m[ 0 ] = a0;
      m[ 1 ] = a1;
      m[ 2 ] = a2;
      m[ 3 ] = a3;
      m[ 4 ] = a4;
      m[ 5 ] = a5;
      m[ 6 ] = a6;
      m[ 7 ] = a7;
      m[ 8 ] = a8;
      return true;
   }

   /**
    @ingroup core
    @param inverse the 3x3 region of a 4x4 matrix, leaving the other elements the same
    @note the matrix is directly modified!
    */
   template <class T>
   inline bool inverse3x3M4( core::Matrix<T, core::IndexMapperColumnMajorFlat2D>& m )
   {
      
      assert( m.sizex() == 4 && m.sizey() == 4 );

      const T c = m[ 0 ] * ( m[ 5 ] * m[ 10 ] - m[ 9 ] * m[ 6 ] ) -
                  m[ 1 ] * ( m[ 10 ] * m[ 4 ] - m[ 6 ] * m[ 8 ] ) +
                  m[ 2 ] * ( m[ 9 ] * m[ 4 ] - m[ 5 ] * m[ 8 ] );

      if ( core::equal<T>( c, 0 ) )
         return false;

      // we need to use extra storage...
      const T a0 = ( m[ 10 ] * m[ 5 ] - m[ 6 ] * m[ 9 ] ) / c;
      const T a1 = ( m[ 2 ] * m[ 9 ] - m[ 10 ] * m[ 1 ] ) / c;
      const T a2 = ( m[ 6 ] * m[ 1 ] - m[ 2 ] * m[ 5 ] ) / c;

      const T a3 = ( m[ 6 ] * m[ 8 ] - m[ 10 ] * m[ 4 ] ) / c;
      const T a4 = ( m[ 10 ] * m[ 0 ] - m[ 2 ] * m[ 8 ] ) / c;
      const T a5 = ( m[ 2 ] * m[ 4 ] - m[ 6 ] * m[ 0 ] ) / c;

      const T a6 = ( m[ 9 ] * m[ 4 ] - m[ 5 ] * m[ 8 ] ) / c;
      const T a7 = ( m[ 8 ] * m[ 1 ] - m[ 0 ] * m[ 9 ] ) / c;
      const T a8 = ( m[ 0 ] * m[ 5 ] - m[ 4 ] * m[ 1 ] ) / c;
      
      // update the matrix
      m[ 0 ] = a0;
      m[ 1 ] = a1;
      m[ 2 ] = a2;
      m[ 4 ] = a3;
      m[ 5 ] = a4;
      m[ 6 ] = a5;
      m[ 8 ] = a6;
      m[ 9 ] = a7;
      m[ 10 ] = a8;
      return true;
   }

   /**
     @ingroup core
     @brief apply 3x3 transformation (rotation+scale only) defined in a 4x4 matrix
            The matrix must be a 4x4 transformation matrix defined by the volume.
            
            Compute vM using only the upper left 3x3 part of M.
    */
   template <class T, class Mapper, class Allocator, class Vector>
   Vector mul4Rot( const core::Matrix<T, Mapper, Allocator>& m, const Vector& v )
   {
      assert( m.sizex() == 4 && m.sizey() == 4 );
      return Vector( v[ 0 ] * m( 0, 0 ) + v[ 1 ] * m( 0, 1 ) + v[ 2 ] * m( 0, 2 ),
                     v[ 0 ] * m( 1, 0 ) + v[ 1 ] * m( 1, 1 ) + v[ 2 ] * m( 1, 2 ),
                     v[ 0 ] * m( 2, 0 ) + v[ 1 ] * m( 2, 1 ) + v[ 2 ] * m( 2, 2 ) );
   }



   /**
    @ingroup core
    @brief Transform a 3-vector with an affine 4x4 transformation matrix, computing Mv', with v' = [ v[ x, y, z], 1 ]
    */
   template <class T, class Mapper, class Allocator, class Vector>
   Vector transf4( const core::Matrix<T, Mapper, Allocator>& m, const Vector& v )
   {
      assert( m.sizex() == 4 && m.sizey() == 4 );
      return Vector( v[ 0 ] * m( 0, 0 ) + v[ 1 ] * m( 0, 1 ) + v[ 2 ] * m( 0, 2 ) + m( 0, 3 ),
                     v[ 0 ] * m( 1, 0 ) + v[ 1 ] * m( 1, 1 ) + v[ 2 ] * m( 1, 2 ) + m( 1, 3 ),
                     v[ 0 ] * m( 2, 0 ) + v[ 1 ] * m( 2, 1 ) + v[ 2 ] * m( 2, 2 ) + m( 2, 3 ) );
   }

   /**
    @ingroup core
    @brief Transform a 2-vector with an affine 3x3 transformation matrix, computing Mv', with v' = [ v[ x, y ], 1 ]
    */
   template <class T, class Mapper, class Allocator, class Vector>
   Vector transf3( const core::Matrix<T, Mapper, Allocator>& m, const Vector& v )
   {
      assert( m.sizex() == 3 && m.sizey() == 3 );
      return Vector( v[ 0 ] * m( 0, 0 ) + v[ 1 ] * m( 0, 1 ) + m( 0, 2 ),
                     v[ 0 ] * m( 1, 0 ) + v[ 1 ] * m( 1, 1 ) + m( 1, 2 ) );
   }


   /**
    @ingroup core
    @brief Create a rotation matrix on the x-axis
    @note the matrix is directly modified!
    */
   template <class T, class Mapper, class Allocator>
   void matrix4x4RotationX( core::Matrix<T, Mapper, Allocator>& m, float angleRadian )
   {
      if ( m.sizex() != 4 || m.sizey() != 4 )
         m = core::Matrix<T, Mapper, Allocator>( 4, 4, true );
      m( 0, 0 ) = 1;
      m( 1, 1 ) = cos( angleRadian );
      m( 1, 2 ) = -sin( angleRadian );
      m( 2, 1 ) = sin( angleRadian );
      m( 2, 2 ) = cos( angleRadian );
      m( 3, 3 ) = 1;
   }

   /**
    @ingroup core
    @brief Create a rotation matrix on the z-axis
    @note the matrix is directly modified!
    */
   template <class T, class Mapper, class Allocator>
   void matrix4x4RotationZ( core::Matrix<T, Mapper, Allocator>& m, float angleRadian )
   {
      if ( m.sizex() != 4 || m.sizey() != 4 )
         m = core::Matrix<T, Mapper, Allocator>( 4, 4, true );
      m( 0, 0 ) = cos( angleRadian );
      m( 0, 1 ) = -sin( angleRadian );
      m( 1, 0 ) = sin( angleRadian );
      m( 1, 1 ) = cos( angleRadian );
      m( 2, 2 ) = 1;
      m( 3, 3 ) = 1;
   }

      /**
    @ingroup core
    @brief Create a rotation matrix on the y-axis
    @note the matrix is directly modified!
    */
   template <class T, class Mapper, class Allocator>
   void matrix4x4RotationY( core::Matrix<T, Mapper, Allocator>& m, float angleRadian )
   {
      if ( m.sizex() != 4 || m.sizey() != 4 )
         m = core::Matrix<T, Mapper, Allocator>( 4, 4, true );
      m( 0, 0 ) = cos( angleRadian );
      m( 0, 2 ) = sin( angleRadian );
      m( 2, 0 ) = -sin( angleRadian );
      m( 2, 2 ) = cos( angleRadian );
      m( 1, 1 ) = 1;
      m( 3, 3 ) = 1;
   }

   inline core::Matrix<float> getRotation4Zf( float angleRadian )
   {
      core::Matrix<float> m = identityMatrix< core::Matrix<float> >( 4 );
      matrix4x4RotationZ( m, angleRadian );
      return m;
   }

   inline core::Matrix<float> getRotation4Yf( float angleRadian )
   {
      core::Matrix<float> m = identityMatrix< core::Matrix<float> >( 4 );
      matrix4x4RotationY( m, angleRadian );
      return m;
   }

   inline core::Matrix<float> getRotation4Xf( float angleRadian )
   {
      core::Matrix<float> m = identityMatrix< core::Matrix<float> >( 4 );
      matrix4x4RotationX( m, angleRadian );
      return m;
   }
}
}

#endif