#ifndef NLL_MATRIX_SORT_H_
# define NLL_MATRIX_SORT_H_

# include <vector>
# include <algorithm>
# include "matrix.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief sort the matrices by row.
    @param rang a value is attributed for each row. Rows are sorted using this value by increasing order. Stable.
    */
   template <class T, class Mapper>
   void sortRow( Matrix<T, Mapper>& m, const std::vector<ui32>& rang )
   {
      typedef std::pair<ui32, ui32> Pair;
      typedef std::vector<Pair>     Pairs;

      assert( m.sizey() == rang.size() );

      // build a temporary array to avoid copy of rows during sorting
      Pairs pairs( m.sizey() );
      for ( ui32 n = 0; n < m.sizey(); ++n )
         pairs[ n ] = std::make_pair( rang[ n ], n );
      std::sort( pairs.begin(), pairs.end() );

      Matrix<T, Mapper> cpy( m.sizey(), m.sizex(), false );
      for ( ui32 ny = 0; ny < m.sizey(); ++ny )
      {
         const ui32 newIndex = pairs[ ny ].second;
         for ( ui32 nx = 0; nx < m.sizex(); ++nx )
            cpy( ny, nx ) = m( newIndex, nx );
      }
      m = cpy;
   }

   /**
    @ingroup core
    @brief sort the matrices by column.
    @param rang a value is attributed for each column. Columns are sorted using this value by increasing order. Stable.
    */
   template <class T, class Mapper>
   void sortCol( Matrix<T, Mapper>& m, const std::vector<ui32>& rang )
   {
      typedef std::pair<ui32, ui32> Pair;
      typedef std::vector<Pair>     Pairs;

      assert( m.sizex() == rang.size() );

      // build a temporary array to avoid copy of rows during sorting
      Pairs pairs( m.sizex() );
      for ( ui32 n = 0; n < m.sizex(); ++n )
         pairs[ n ] = std::make_pair( rang[ n ], n );
      std::sort( pairs.begin(), pairs.end() );

      Matrix<T, Mapper> cpy( m.sizey(), m.sizex(), false );
      
      for ( ui32 nx = 0; nx < m.sizex(); ++nx )
      {
         const ui32 newIndex = pairs[ nx ].second;
         for ( ui32 ny = 0; ny < m.sizey(); ++ny )
         
            cpy( ny, nx ) = m( ny, newIndex );
      }
      m = cpy;
   }

}
}

#endif
