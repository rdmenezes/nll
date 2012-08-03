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
   template <class T, class Mapper, class Allocator>
   void sortRow( Matrix<T, Mapper, Allocator>& m, const std::vector<size_t>& rang )
   {
      typedef std::pair<size_t, size_t> Pair;
      typedef std::vector<Pair>     Pairs;

      assert( m.sizey() == rang.size() );

      // build a temporary array to avoid copy of rows during sorting
      Pairs pairs( m.sizey() );
      for ( size_t n = 0; n < m.sizey(); ++n )
         pairs[ n ] = std::make_pair( rang[ n ], n );
      std::sort( pairs.begin(), pairs.end() );

      Matrix<T, Mapper, Allocator> cpy( m.sizey(), m.sizex(), false );
      for ( size_t ny = 0; ny < m.sizey(); ++ny )
      {
         const size_t newIndex = pairs[ ny ].second;
         for ( size_t nx = 0; nx < m.sizex(); ++nx )
            cpy( ny, nx ) = m( newIndex, nx );
      }
      m = cpy;
   }

   /**
    @ingroup core
    @brief sort the matrices by column.
    @param rang a value is attributed for each column. Columns are sorted using this value by increasing order. Stable.
    */
   template <class T, class Mapper, class Allocator>
   void sortCol( Matrix<T, Mapper, Allocator>& m, const std::vector<size_t>& rang )
   {
      typedef std::pair<size_t, size_t> Pair;
      typedef std::vector<Pair>     Pairs;

      assert( m.sizex() == rang.size() );

      // build a temporary array to avoid copy of rows during sorting
      Pairs pairs( m.sizex() );
      for ( size_t n = 0; n < m.sizex(); ++n )
         pairs[ n ] = std::make_pair( rang[ n ], n );
      std::sort( pairs.begin(), pairs.end() );

      Matrix<T, Mapper, Allocator> cpy( m.sizey(), m.sizex(), false );
      
      for ( size_t nx = 0; nx < m.sizex(); ++nx )
      {
         const size_t newIndex = pairs[ nx ].second;
         for ( size_t ny = 0; ny < m.sizey(); ++ny )
         
            cpy( ny, nx ) = m( ny, newIndex );
      }
      m = cpy;
   }

}
}

#endif
