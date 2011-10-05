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

#ifndef NLL_ALGORITHM_POTENTIAL_GAUSSIAN_TABLE_H_
# define NLL_ALGORITHM_POTENTIAL_GAUSSIAN_TABLE_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief represent a potential constructed as discrete events
    */
   class PotentialTable
   {
   public:
      typedef double                      value_type;
      typedef ui32                        value_typei;
      typedef core::Matrix<value_type>    Matrix;
      typedef core::Buffer1D<value_type>  Vector;
      typedef core::Buffer1D<ui32>        VectorI;

   public:
      PotentialTable()
      {
         // nothing, unusable potential!
      }

      /**
       @brief table the table of events annotated with the corresponding probability

       event  A B
       ex: T[ 0 0 ] = 0.01 | index = 0
           T[ 0 1 ] = 0.25 | index = 1
           T[ 1 0 ] = 0.05 | index = 2
           T[ 1 1 ] = 0.75 | index = 3

        For example index = 1, represents p(A=false, B=true) = 0.25
       */
      PotentialTable( const Vector& table, const VectorI id = VectorI() )
      {
         if ( id.size() == 0 )
         {
            const double val = std::log( (double)table.size() ) / std::log( 2.0 );
            const ui32 vali = core::round( val );
            ensure( fabs( val - (double)vali ) < 1e-3, "the table size must be a power of 2 (ie. it must be a full iteration of the possible cases)" );
            // assign default ID
            _id = VectorI( vali );
            for ( ui32 n = 0; n < vali; ++n )
            {
               _id[ n ] = n;
            }
         } else {
            _id = id;
         }

         ensure( ( id.size() < 8 * sizeof( value_typei ) ), "the number of joined variable is way too big! (exponential in the size of the id)" );
         ensure( table.size() == ( (ui32)1 << id.size() ), "the table must be full!" );
      }

      const Vector& getTable() const
      {
         return _table;
      }

      PotentialTable marginalization( const VectorI& varIndexToRemove ) const
      {
         return PotentialTable();
      }

      PotentialTable conditioning( const Vector& vars, const VectorI& varsIndex ) const
      {
         return PotentialTable();
      }

      PotentialTable operator*( const PotentialTable& g2 ) const
      {
         return PotentialTable();
      }

   private:
      Vector   _table;
      VectorI  _id;
   };
}
}

#endif