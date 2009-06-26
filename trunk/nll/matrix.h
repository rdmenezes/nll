#ifndef NLL_MATRIX_H_
# define NLL_MATRIX_H_

# include <limits>
# include <cmath>
# include "buffer1D.h"

namespace nll
{
namespace core
{

   /**
    @ingroup core
    @brief Define the basic Matric structure. Memory are shared accross intances

    @param IndexMapper2D defines how memory and index are mapped
   
    IndexMapper must define:
      - IndexMapper(const ui32 sizex, const ui32 sizey)
      - ui32 index(const ui32 x, const ui32 y) const
      - void indexInverse( const ui32 index, ui32& out_x, ui32& out_y ) const
   */
   template <class T, class IndexMapper2D = IndexMapperColumnMajorFlat2D>
   class Matrix : public Buffer1D<T, IndexMapperFlat1D>
   {
   public:
      typedef IndexMapper2D                  IndexMapper;
      typedef Buffer1D<T, IndexMapperFlat1D> Base;

   public:
      /**
       @brief contruct a new matrix from a buffer. The buffer will be viewed as a matrix (size, 1).
       */
      explicit Matrix( const Base& buf ) : Base( buf ), _sizex ( buf.size() ), _sizey( 1 ), _indexMapper( buf.size(), 1 ){}

      /**
       @brief contruct a new matrix from a buffer.
       */
      explicit Matrix( const Base& buf, ui32 sy, ui32 sx ) : Base( buf ), _sizex ( sx ), _sizey( sy ), _indexMapper( sx, sy )
      {
         ensure( sx * sy == buf.size(), "size error" );
      }

      /**
       @brief build a matrix with a fixed size.
       @param zero if true the memory is set to 0, else undetermined.
       */
      Matrix( ui32 sizey, ui32 sizex, bool zero = true ) : Base( sizex * sizey, zero ), _sizex( sizex ), _sizey( sizey ), _indexMapper( sizex, sizey )
      {}

      /**
       @brief make an alias of the source matrix
       */
      Matrix( const Matrix& mat ) : Base( mat ), _sizex( mat.sizex() ), _sizey( mat.sizey() ), _indexMapper( mat.sizex(), mat.sizey() )
      {}

      /**
       @brief make an unintialized matrix
       */
      Matrix() : _sizex( 0 ), _sizey( 0 ), _indexMapper( 0, 0 )
      {}

      /**
       @brief make a matrix from a raw memory buffer
       @param ownsBuffer if yes memory will be handled (and destroyed at the end of life of the object). Else ensure buffer is valid until the matrix is alive.
       */
      Matrix( T* buf, ui32 sizey, ui32 sizex, bool ownsBuffer ) : Base( buf, sizex * sizey, ownsBuffer ), _sizex( sizex ), _sizey( sizey ), _indexMapper( sizex, sizey )
      {}

      /**
       @brief clone a matrix (memory is copied)
       */
      void clone( const Matrix& mat )
      {
         dynamic_cast<Base*>(this)->clone( mat );
         _sizex = mat._sizex;
         _sizey = mat._sizey;
         _indexMapper = mat._indexMapper;
      }

      /**
       @brief Import a matrix that uses a different mapper/form...
       */
      template <class TT, class Mapper>
      void import( const Matrix<TT, Mapper>& m )
      {
         *this = Matrix( m.sizey(), m.sizex() );
         for ( ui32 ny = 0; ny < m.sizey(); ++ny )
            for ( ui32 nx = 0; nx < m.sizex(); ++nx )
               at( ny, nx ) = static_cast<value_type>( m( nx, ny ) );
      }

      /**
       @brief make an alias (memory is shared)
       */
      void copy( const Matrix& mat )
      {
         dynamic_cast<Base*>(this)->copy( mat );
         _sizex = mat._sizex;
         _sizey = mat._sizey;
         _indexMapper = mat._indexMapper;
      }

      /**
       @brief return the actual index in memory of the point(x, y)
       */
      inline ui32 index( const ui32 y, const ui32 x ) const
      {
         return Base::IndexMapper::index( _indexMapper.index( y, x ) );
      }

      /**
       @brief return the value at position (x, y)
       */
      inline typename BestReturnType<T>::type at( const ui32 y, const ui32 x ) const
      {
         return this->_buffer[ index( x, y ) ];
      }

      /**
       @brief return the value at position (x, y)
       */
      inline T& at( const ui32 y, const ui32 x )
      {
         return this->_buffer[ index( x, y ) ];
      }

      /**
       @brief return the value at position (x, y)
       */
      inline typename BestReturnType<T>::type operator()( const ui32 y, const ui32 x ) const
      {
         return at( y, x );
      }

      /**
       @brief return the value at position (x, y)
       */
      inline T& operator()( const ui32 y, const ui32 x )
      {
         return at( y, x );
      }

      /**
       @brief print the matrix to a stream
       */
      void print( std::ostream& o ) const
      {
         if ( this->_buffer )
         {
            o << "Buffer2D(" << *this->_cpt << ") size=" << this->_size << std::endl;

            for ( ui32 ny = 0; ny < _sizey; ++ny )
            {
               for ( ui32 nx = 0; nx < _sizex; ++nx )
                  o << at( ny, nx ) << "\t";
               o << std::endl;
            }
            o << std::endl;
         } else {
            o << "Buffer2D(NULL)" << std::endl;
         }
      }

      /**
       @brief return the size
       */
      ui32 sizex() const{ return _sizex; }

      /**
       @brief return the size
       */
      ui32 sizey() const{ return _sizey; }

      /**
       @brief return the size
       */
      ui32 nrows() const{ return _sizex; }

      /**
       @brief return the size
       */
      ui32 ncols() const{ return _sizey; }

      /**
       @brief write the matrix to a stream
       */
      void write( std::ostream& o ) const
      {
         nll::core::write<ui32>( _sizex, o );
         nll::core::write<ui32>( _sizey, o );
         Base::write( o );
      }

      /**
       @brief read the matrix from a stream
       */
      void read( std::istream& i )
      {
         nll::core::read<ui32>( _sizex, i );
         nll::core::read<ui32>( _sizey, i );
         _indexMapper = IndexMapper2D( _sizex, _sizey );
         Base::read( i );
      }

      /**
       @brief make an alias of the matrix
       */
      Matrix& operator=( const Matrix& cpy )
      {
         copy( cpy );
         return *this;
      }

      /**
       @brief test if the matrices are semantically equal
       */
      template <class Mapper>
      inline bool equal( const Matrix<T, Mapper>& op, T tolerance = std::numeric_limits<T>::epsilon() ) const
      {
         if ( _sizex != op.sizex() || _sizey != op.sizey() )
            return false;
         if ( ! op.getBuf() || !this->_buffer )
            return false;
         if ( op.getBuf() == this->_buffer )
            return true;
         for ( ui32 nx = 0; nx < _sizex; ++nx )
            for ( ui32 ny = 0; ny < _sizey; ++ny )
               if ( absolute( op( ny, nx ) - at( ny, nx ) ) > tolerance )
                  return false; 
         return true;
      }

      /**
       @brief test if the matrices are semantically equal
       */
      inline bool operator==( const Matrix& op ) const
      {
         return equal( op, std::numeric_limits<T>::epsilon() );
      }

   private:
      ui32					_sizex;
      ui32					_sizey;
      IndexMapper2D		_indexMapper;
   };

   typedef Matrix<f32> Matrixf;
   typedef Matrix<f64> Matrixd;
   typedef Matrix<i32> Matrixi;

}
}

#endif
