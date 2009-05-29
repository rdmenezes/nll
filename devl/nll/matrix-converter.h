#ifndef NLL_MATRIX_CONVERTER_H_
# define NLL_MATRIX_CONVERTER_H_

# include "matrix.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Convert a matrix.
    
    Simply static_cast<To> the values for the new matrix
    */
   template <class Ti, class Mapperi, class To, class Mappero>
   Matrix<To, Mappero> convertMatrix( const Matrix<Ti, Mapperi>& i )
   {
      Matrix<To, Mappero> o( i.sizey(), i.sizex(), false );
      for ( ui32 nx = 0; nx < i.sizex(); ++nx )
         for ( ui32 ny = 0; ny < i.sizey(); ++ny )
            o( ny, nx ) = static_cast<To> ( i( ny, nx ) );
      return o;
   }
}
}

#endif
