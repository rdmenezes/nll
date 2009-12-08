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
                  m[ 3 ] * ( m[ 1 ] * m[ 8 ] - m[ 7 ] * m[ 2 ] ) +
                  m[ 6 ] * ( m[ 1 ] * m[ 5 ] - m[ 4 ] * m[ 2 ] );

      if ( core::equal<T>( c, 0 ) )
         return false;

      m[ 0 ] = ( m[ 4 ] * m[ 8 ] - m[ 7 ] * m[ 5 ] ) / c;
      m[ 1 ] = ( m[ 7 ] * m[ 2 ] - m[ 1 ] * m[ 8 ] ) / c;
      m[ 2 ] = ( m[ 1 ] * m[ 5 ] - m[ 4 ] * m[ 2 ] ) / c;

      m[ 3 ] = ( m[ 6 ] * m[ 5 ] - m[ 3 ] * m[ 8 ] ) / c;
      m[ 4 ] = ( m[ 0 ] * m[ 8 ] - m[ 6 ] * m[ 2 ] ) / c;
      m[ 5 ] = ( m[ 3 ] * m[ 2 ] - m[ 0 ] * m[ 5 ] ) / c;

      m[ 6 ] = ( m[ 3 ] * m[ 7 ] - m[ 6 ] * m[ 4 ] ) / c;
      m[ 7 ] = ( m[ 6 ] * m[ 1 ] - m[ 0 ] * m[ 7 ] ) / c;
      m[ 8 ] = ( m[ 0 ] * m[ 4 ] - m[ 3 ] * m[ 1 ] ) / c;
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
}
}

#endif