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

# pragma warning( push )
# pragma warning( disable:4996 ) // checked iterator warning

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief represent a potential constructed as discrete events

    Table is stored as: consider a new base DomainVar3 | DomainVar2 | DomainVar1,
    potential(DomainVar3 = v3, DomainVar2 = v2, DomainVar1 = v1 ) = x
    and is stored at index = ( v3 * #DomainVar2 * #DomainVar1 +
                               v2 * #DomainVar1 +
                               v1 )
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

       Cardinality = [2 2] // for each variable, there are 2 discrete possibilities
       event  A B
       ex: T[ 0 0 ] = 0.01 | index = 0
           T[ 0 1 ] = 0.25 | index = 1
           T[ 1 0 ] = 0.05 | index = 2
           T[ 1 1 ] = 0.75 | index = 3

        For example index = 1, represents p(A=false, B=true) = 0.25
       */
      PotentialTable( const Vector& table, const VectorI domain, const VectorI& cardinality )
      {
         const ui32 expectedTableSize = getTableSize( cardinality );
         ensure( expectedTableSize == table.size(), "missing table entries" );
         ensure( domain.size() == cardinality.size(), "missing id" );
         ensure( isDomainSorted( domain ), "the domain must be sorted!" );
         _domain = domain;
         _cardinality = cardinality;
         _table = table;

         ensure( ( domain.size() < 8 * sizeof( value_typei ) ), "the number of joined variable is way too big! (exponential in the size of the id)" );
      }

      /**
       @brief same as before but set all probabilities to zero
       */
      PotentialTable( const VectorI domain, const VectorI& cardinality )
      {
         ensure( domain.size() == cardinality.size(), "missing id" );
         ensure( isDomainSorted( domain ), "the domain must be sorted!" );
         _domain = domain;
         _cardinality = cardinality;
         _table = Vector( domain.size() );

         ensure( ( domain.size() < 8 * sizeof( value_typei ) ), "the number of joined variable is way too big! (exponential in the size of the id)" );
      }

      void print( std::ostream& o ) const
      {
         o << "Potential Table:" << std::endl;
         o << "Domain=" ;
         _domain.print( o );
         o << "Cardinality=" ;
         _cardinality.print( o );
         o << "Table=" << std::endl;
         _table.print( o );
      }

      /**
       @brief Given a representing PDF p( X, Y ), compute p( X ) by integrating over Y
       @param varIndexToRemove refering to the Y above
       */
      PotentialTable marginalization( const VectorI& varIndexToRemove ) const
      {
         ensure( varIndexToRemove.size(), "empty set" );

         int size = (int)varIndexToRemove.size();
         PotentialTable p = marginalization( varIndexToRemove[ 0 ] );
         for ( int n = 1; n < size; ++n )
         {
            p = p.marginalization( varIndexToRemove[ n ] );
         }
         return p;
      }

      /**
       @brief returns the probability stored in the table and associated to the provided event
       @note this is a convenient way to initialize the table but it is not very performant...
       @param evidence the evidence for all the table variable!
       */
      value_type& getProbability( const VectorI& event )
      {
         ensure( event.size() == _domain.size(), "all variables must be specified!" );

         // first compute the strides
         std::vector<ui32> strides( _domain.size() );
         ui32 stride = 1;
         for ( ui32 n = 0; n < _domain.size(); ++n )
         {
            strides[ n ] = stride;
            stride *= _cardinality[ n ];
         }

         // now compute the table index
         ui32 index = 0;
         for ( ui32 n = 0; n < _domain.size(); ++n )
         {
            index += event[ n ] * strides[ n ];
         }

         return _table[ index ];
      }

      /**
       @brief computes p(X | Y=y) i.e., entering evidence
       @param vars the evidence of Y=y
       @param varIndexToRemove the index of Y's, must be sorted 0->+inf
       */
      PotentialTable conditioning( const VectorI& evidence, const VectorI& varIndexToRemove ) const
      {
         ensure( varIndexToRemove.size(), "empty set" );

         int size = (int)varIndexToRemove.size();
         PotentialTable p = conditioning( evidence[ 0 ], varIndexToRemove[ 0 ] );
         for ( int n = 1; n < size; ++n )
         {
            p = p.conditioning( evidence[ n ], varIndexToRemove[ n ] );
         }
         return p;
      }

      PotentialTable operator*( const PotentialTable& g2 ) const
      {
         PotentialTable extended1 = extendDomain( g2.getDomain(), g2.getCardinality() );
         PotentialTable extended2 = g2.extendDomain( _domain, _cardinality );
         const ui32 size = extended1.getTable().size();
         for ( ui32 n = 0; n < size; ++n )
         {
            extended1._table[ n ] *= extended2.getTable()[ n ];
         }
         return extended1;
      }

      /**
       @brief Extend the domain of the table by adding new domain variable with
              associated probability of 0
       */
      PotentialTable extendDomain( const VectorI& domain, const VectorI& cardinality ) const
      {
         ensure( isDomainSorted( domain ), "domain must be sorted first" );

         VectorI newDomain;
         VectorI newCardinality;
         Vector newTable = extend( domain, cardinality, newDomain, newCardinality );
         return PotentialTable( newTable, newDomain, newCardinality );
      }

      const VectorI& getDomain() const
      {
         return _domain;
      }

      const VectorI& getCardinality() const
      {
         return _cardinality;
      }

      const Vector& getTable() const
      {
         return _table;
      }

   private:
      static bool isDomainSorted( const VectorI& domain )
      {
         if ( domain.size() == 0 )
            return true;
         for ( ui32 n = 0; n < domain.size() - 1; ++n )
         {
            if ( domain[ n ] >= domain[ n + 1 ] )
               return false;
         }
         return true;
      }

      template <class VectorT>
      ui32 getTableSize( const VectorT& cardinality ) const
      {
         ui32 tableSize = 1;
         for ( ui32 n = 0; n < cardinality.size(); ++n )
         {
            tableSize *= cardinality[ n ];
         }
         return tableSize;
      }

      // compute P( X | E = e ) = P( X, E ) / P( E )
      PotentialTable conditioning( ui32 evidence, ui32 varIndexToRemove ) const
      {
         ensure( _domain.size(), "domain is empty!" );
         VectorI newDomain;
         VectorI newCardinality;
         int removedIndex = -1;
         std::vector<ui32> strides;
         computeIndexToRemove( varIndexToRemove, removedIndex, newDomain, newCardinality, strides );
         ui32 stride = strides[ removedIndex ];

         // create the result table
         const ui32 newSize = getTableSize( newCardinality );
         Vector newTable( newSize );

         // compute p(E=e)
         value_type pe = 0;
         ui32 indexDst = 0;
         for ( ui32 index = evidence * stride; index < _table.size(); index += stride * _cardinality[ removedIndex ] )
         {
            // collect evidence and store the accessed index in the same order
            for ( ui32 c = 0; c < stride; ++c )
            {
               const ui32 indexSrc = index + c;
               const value_type val = _table[ indexSrc ];
               pe += val;
               newTable[ indexDst++ ] = val;
            }
         }
         ensure( pe > 0, "p(E = e) <= 0" );
         
         // now just normalize
         for ( ui32 index = 0; index < newSize; index++ )
         {
            newTable[ index ] /= pe;
         }

         return PotentialTable( newTable, newDomain, newCardinality );
      }


      void computeIndexToRemove( ui32 varIndexToRemove, int& removedIndex, VectorI& newDomain, VectorI& newCardinality, std::vector<ui32>& strides ) const
      {
         newDomain = VectorI( _domain.size() - 1 );
         newCardinality = VectorI( _domain.size() - 1 );
         strides = std::vector<ui32>( _domain.size() );

         // create the new domain, cardinality and computes the stride necessary to marginalize this variable
         removedIndex = -1;
         ui32 stride = 1;
         const ui32 oldSize = _domain.size();
         ui32 index = 0;
         for ( ui32 n = 0; n < oldSize; ++n )
         {
            const ui32 id = _domain[ n ];
            if ( id == varIndexToRemove )
            {
               removedIndex = n;
            } else {
               newDomain[ index ] = id;
               newCardinality[ index ] = _cardinality[ n ];
               ++index;
            }
            strides[ n ] = stride;
            stride *= _cardinality[ n ];
         }
         ensure( removedIndex != -1, "variable not found in domain" );
      }

      // assuming a partition X = [ U V ], computes integral(-inf,+inf)p(U)dV
      PotentialTable marginalization( ui32 varIndexToRemove ) const
      {
         // create the new domain, cardinality and computes the stride necessary to marginalize this variable
         ensure( _domain.size(), "domain is empty!" );
         VectorI newDomain;
         VectorI newCardinality;
         int removedIndex = -1;
         std::vector<ui32> strides;
         computeIndexToRemove( varIndexToRemove, removedIndex, newDomain, newCardinality, strides );
         ui32 stride = strides[ removedIndex ];
         

         const ui32 newSize = getTableSize( newCardinality );
         Vector newTable( newSize );
         std::vector<char> used( _table.size() );
         ui32 indexWrite = 0;
         for ( ui32 index = 0; index < _table.size(); ++index )
         {
            if ( used[ index ] == 0 )
            {
               value_type accum = 0;
               for ( ui32 nn = 0; nn < _cardinality[ removedIndex ]; ++nn )
               {
                  const ui32 indexRef = index + nn * stride;
                  accum += _table[ indexRef ];
                  used[ indexRef ] = 1;
               }
               newTable[ indexWrite++ ] = accum;
            }
         }

         return PotentialTable( newTable, newDomain, newCardinality );
      }

      // given the current domain, extend the table to another domain
      Vector extend( const VectorI& domain, const VectorI& cardinality, VectorI& newDomain_out, VectorI& newCardinality_out ) const
      {
         ensure( domain.size() == cardinality.size(), "args don't match" );

         std::vector<ui32> newDomain;
         std::vector<ui32> newCardinality;
         std::vector<char> newDomainBelongs;
         std::vector<ui32> stride;
         joinDomain( _domain, _cardinality, domain, cardinality, newDomain, newCardinality, newDomainBelongs, stride );
         stride.push_back( 0 );  // we add an extra cell to facilitate counting

         const ui32 size = getTableSize( newCardinality );
         Vector newTable( size );

         ui32 index = 0;
         const ui32 nbId = (ui32)newDomain.size();
         std::vector<ui32> cpt( newDomain.size() + 1 );
         ui32 indexTable = 0;
         while ( cpt[ nbId ] == 0)
         {
            newTable[ indexTable ] = _table[ index ];

            ++cpt[ 0 ];
            ++indexTable;
            if ( newDomainBelongs[ 0 ] == 0 )
            {
               ++index;
            }
            for ( ui32 id = 0; id < nbId; ++id )
            {
               if ( cpt[ id ] >= newCardinality[ id ] )
               {
                  index -= stride[ id ] * newCardinality[ id ];
                  index += stride[ id + 1 ];
                  ++cpt[ id + 1 ];
                  cpt[ id ] = 0;
               } else {
                  break;
               }
            }
         }

         newDomain_out      = VectorI( nbId );
         newCardinality_out = VectorI( nbId );
         std::copy( newDomain.begin(), newDomain.end(), newDomain_out.begin() );
         std::copy( newCardinality.begin(), newCardinality.end(), newCardinality_out.begin() );

         return newTable;
      }

      // join the two domains <d1, d2> resulting in <outNewDomain> holding the ids and <outDomainBelongs> = 0, means this id is from domain 1, else only domain 2 (if d1 & d2, it prefers d1)
      // the stride is the index displacement relating to the domain d1
      static void joinDomain( const VectorI& d1, const VectorI& cardinality1,
                              const VectorI& d2, const VectorI& cardinality2,
                              std::vector<ui32>& newDomain, std::vector<ui32>& newCardinality, std::vector<char>& newDomainBelongs, std::vector<ui32>& stride )
      {
         ui32 n1 = 0;
         ui32 n2 = 0;
         ui32 accum = 1;
         while (1)
         {
            const ui32 id1 = ( n1 < d1.size() ) ? d1[ n1 ] : std::numeric_limits<ui32>::max();
            const ui32 id2 = ( n2 < d2.size() ) ? d2[ n2 ] : std::numeric_limits<ui32>::max();

            if ( id1 < id2 )
            {
               newDomain.push_back( id1 );
               newDomainBelongs.push_back( 0 );
               newCardinality.push_back( cardinality1[ n1 ] );
               stride.push_back( accum );
               accum *= cardinality1[ n1 ];
               ++n1;
            } else if ( id2 < id1 )
            {
               newDomain.push_back( id2 );
               newDomainBelongs.push_back( 1 );
               newCardinality.push_back( cardinality2[ n2 ] );
               stride.push_back( 0 );
               ++n2;
            } else {
               // equal!
               newDomain.push_back( id1 );
               newDomainBelongs.push_back( 0 );
               newCardinality.push_back( cardinality1[ n1 ] );
               assert( cardinality1[ n1 ] == cardinality2[ n2 ] );   // if we have the same domain id, we must have the same cardinality

               stride.push_back( accum );
               accum *= cardinality1[ n1 ];

               ++n1;
               ++n2;
            }

            if ( n1 == d1.size() && n2 == d2.size() )
               break;
         }
      }

   private:
      Vector   _table;
      VectorI  _cardinality;
      VectorI  _domain;
   };
}
}

# pragma warning( pop )

#endif