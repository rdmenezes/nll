#ifndef NLL_CORE_CUSTOM_H_
# define NLL_CORE_CUSTOM_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief A custom fast 3x3 multiplication with vector
    */
   template <class T, class Vector>
   inline Vector mat3Mulv( const core::Matrix<T, core::IndexMapperColumnMajorFlat2D>& m, Vector& v )
   {
      assert( m.sizex() == 3 && m.sizey() == 3 );
      return Vector( v[ 0 ] * m[ 0 ] + v[ 1 ] * m[ 3 ] + v[ 2 ] * m[ 6 ],
                     v[ 0 ] * m[ 1 ] + v[ 1 ] * m[ 4 ] + v[ 2 ] * m[ 7 ],
                     v[ 0 ] * m[ 2 ] + v[ 1 ] * m[ 5 ] + v[ 2 ] * m[ 8 ] );
   }

   /**
    @ingroup core
    @brief A custom fast 3x3 inverse.
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
     @brief apply 3x3 transformation (rotation+scale only) defined in a 4x4 matrix
            The matrix must be a 4x4 transformation matrix defined by the volume.
            
            Compute Mv using only the upper left 3x3 part of M.
    */
   template <class T, class Mapper, class Allocator, class Vector>
   Vector mul4Rot( const core::Matrix<T, Mapper, Allocator>& m, Vector& v )
   {
      assert( m.sizex() == 4 && m.sizey() == 4 );
      return Vector( v[ 0 ] * m( 0, 0 ) + v[ 1 ] * m( 0, 1 ) + v[ 2 ] * m( 0, 2 ),
                     v[ 0 ] * m( 1, 0 ) + v[ 1 ] * m( 1, 1 ) + v[ 2 ] * m( 1, 2 ),
                     v[ 0 ] * m( 2, 0 ) + v[ 1 ] * m( 2, 1 ) + v[ 2 ] * m( 2, 2 ) );
   }

   /**
    @brief Transform a 3-vector with an affine 4x4 transformation matrix
    */
   template <class T, class Mapper, class Allocator, class Vector>
   Vector transf4( const core::Matrix<T, Mapper, Allocator>& m, Vector& v )
   {
      assert( m.sizex() == 4 && m.sizey() == 4 );
      return Vector( v[ 0 ] * m( 0, 0 ) + v[ 1 ] * m( 0, 1 ) + v[ 2 ] * m( 0, 2 ) + m( 0, 3 ),
                     v[ 0 ] * m( 1, 0 ) + v[ 1 ] * m( 1, 1 ) + v[ 2 ] * m( 1, 2 ) + m( 1, 3 ),
                     v[ 0 ] * m( 2, 0 ) + v[ 1 ] * m( 2, 1 ) + v[ 2 ] * m( 2, 2 ) + m( 2, 3 ) );
   }


   template <class T, class Mapper, class Allocator>
   void matrix4x4RotationX( core::Matrix<T, Mapper, Allocator>& m, float angleRadian )
   {
      if ( m.sizex() != 4 || m.sizey() != 4 )
         m = core::Matrix<T, Mapper, Allocator>( 4, 4, true );
      m( 0, 0 ) = 1;
      m( 1, 1 ) = cos( angleRadian );
      m( 2, 1 ) = -sin( angleRadian );
      m( 1, 2 ) = sin( angleRadian );
      m( 2, 2 ) = cos( angleRadian );
      m( 3, 3 ) = 1;
   }

   template <class T, class Mapper, class Allocator>
   void matrix4x4RotationZ( core::Matrix<T, Mapper, Allocator>& m, float angleRadian )
   {
      if ( m.sizex() != 4 || m.sizey() != 4 )
         m = core::Matrix<T, Mapper, Allocator>( 4, 4, true );
      m( 0, 0 ) = cos( angleRadian );
      m( 1, 0 ) = -sin( angleRadian );
      m( 0, 1 ) = sin( angleRadian );
      m( 1, 1 ) = cos( angleRadian );
      m( 2, 2 ) = 1;
      m( 3, 3 ) = 1;
   }
}
}

#endif