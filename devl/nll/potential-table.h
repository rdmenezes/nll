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

#ifndef NLL_ALGORITHM_POTENTIAL_TABLE_H_
# define NLL_ALGORITHM_POTENTIAL_TABLE_H_

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

    The table must always model p( a | b, c, ... x ) so that domain[ a ] = 0
                                                             domain[ b ] = 1
                                                             ...
                                                             domain[ x ] = 26
    (this is used in normalization and <BayesianNetworkSampling>)

    The domain must also be ordered so that domain[ 0 ] < domain[ 1 ] < domain[ 2 ] ... meaning that in a graph,
    the parent nodes must have a higher domain than the children.

    (this is used in <BayesianNetworkSampling> to help with predecessors, and helpful for performance e.g., domain merge)
    */
   class PotentialTable
   {
   public:
      typedef double                      value_type;
      typedef ui32                        value_typei;
      typedef core::Matrix<value_type>    Matrix;
      typedef core::Buffer1D<value_type>  Vector;
      typedef core::Buffer1D<ui32>        VectorI;
      typedef VectorI                     EvidenceValue;

   public:
      /**
       @brief table the table of events annotated with the corresponding probability

       Cardinality = [2 2] // for each variable, there are 2 discrete possibilities
       event  A B
       ex: T[ 0 0 ] = 0.01 | index = 0
           T[ 0 1 ] = 0.25 | index = 1
           T[ 1 0 ] = 0.05 | index = 2
           T[ 1 1 ] = 0.75 | index = 3

        For example index = 1, represents p(A=false, B=true) = 0.25

        Format that must be followed:
          for a bayesian network the parent nodes must have a higher domain than the current node, consequently
          the table will be stored with domain[0] being the probability

          See a working example: BNT of 4 nodes with Cloudy->Sprinkler, Cloudy->Rainy, Sprinkler->Wet, Rainy->Wet dependencies
                                         Cloudy (domain = 3)
                    Sprinkler (domain = 2)             Rainy (domain = 1)
                                         Wet (domain = 0)

           The table will be stored as:
            Cloudy  p(C=t)    p(C=f)  =>   table encoding = 0.5  table index = 0
                     0.5        0.5                         0.5                1
            Rainy  C P(R=t)   p(R=f)  =>   table encoding = 0.2                0
                   f 0.8      0.2                           0.8                1
                   t 0.2      0.8                           0.8                2
                                                            0.2                3
            Wet  S R P(W=f)  P(W=t)   =>   table encoding = 1
                 f f 1       0                              0
                 t f 0.1     0.9                            0.1
                 f t 0.1     0.9                            0.9
                 t t 0.01    0.99                           0.1
                                                            0.9
                                                            0.01
                                                            0.99
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

         /*
         // check we have a correctly formated table! (i.e., at least sum(p(x|p)) = 1)
         for ( ui32 n = 0; n < table.size(); n += cardinality[ 0 ] )
         {
            value_type sum = 0;
            for ( ui32 nn = 0; nn < cardinality[ 0 ]; ++nn )
            {
               sum += table[ n + nn ];
            }
            ensure( fabs( sum - 1 ) < 1e-2, "sum_x(x|p(x)) == 1 for a correct table!" );
         }*/

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

      /**
       @brief Create a neutral potential
       */
      PotentialTable()
      {
         _table = Vector( 1 );
         _table[ 0 ] = 1;
      }

      /**
       @brief Utility function to reorder tables into a sorted domain

       This is a convenient function. Often the table statistics don't follow the order constraint, hence this function.

       For exemple, lets say we have a domain of 3 vars [ 1 2 3 ] and for each var 2 posibilities but our table was stored
       in this order [ 1 3 2 ], we want to express this table in the original order (it must be sorted for performance reasons).

       domain: 1 3 2 probability index         1 2 3 index in original table
       value   0 0 0 1.0         0             0 0 0 0
               0 0 1 0.0         1             0 0 1 2
               0 1 0 0.9         2     sorted  0 1 0 1
               0 1 1 0.1         3       =>    0 1 1 3
               1 0 0 0.8         4             1 0 0 4
               1 0 1 0.2         5             1 0 1 6
               1 1 0 0.01        6             1 1 0 5
               1 1 1 0.99        7             1 1 1 7
       */
      static PotentialTable reorderTable( const Vector& table, const VectorI& domain, const VectorI& domainCardinality )
      {
         ensure( domain.size() == domainCardinality.size(), "cardinality and domain must have same dimensionality" );

         // now sort the domain and keep track of the original order
         std::vector< std::pair< ui32, ui32 > > domainSorted;
         domainSorted.reserve( domain.size() );
         for ( ui32 n = 0; n < domain.size(); ++n )
         {
            domainSorted.push_back( std::make_pair( domain[ n ], n ) );
         }
         std::sort( domainSorted.begin(), domainSorted.end() );

         ui32 stride = 1;
         std::vector<ui32> strides( domain.size() );
         for ( ui32 n = 0; n < domain.size(); ++n )
         {
            const ui32 indexInOriginalDomain = domainSorted[ n ].second;
            strides[ indexInOriginalDomain ] = stride;
            stride *= domainCardinality[ n ];
         }

         // now reorder the table
         VectorI newDomain( domain.size() );
         VectorI newCardinality( domain.size() );
         Vector newTable( table.size() );
         for ( ui32 n = 0; n < domain.size(); ++n )
         {
            const ui32 indexInOriginalDomain = domainSorted[ n ].second;
            newDomain[ n ] = domain[ indexInOriginalDomain ];
            newCardinality[ n ] = domainCardinality[ indexInOriginalDomain ];
         }

         std::vector<ui32> counts( domain.size() );
         ui32 currentIndex = 0;

         // Currently, a counter is used to backtrack the source index...
         for ( ui32 n = 0; n < table.size(); ++n )
         {
            // check the counter bounds (i.e., it must be within domainCardinality, if not backtrack the source index),
            for ( ui32 i = 0; i < domain.size(); ++i )
            {
               if ( counts[ i ] == newCardinality[ i ] )
               {
                  ++counts[ i + 1 ];
                  for ( int j = (int)i; j >= 0; --j )
                  {
                     currentIndex -= strides[ j ] * counts[ j ];
                     counts[ j ] = 0;
                  }
                  currentIndex += strides[ i + 1 ];
               } else break;
            }

            // handling the table
            newTable[ n ] = table[ currentIndex ];

            // finally increase the counter
            ++counts[ 0 ];
            currentIndex += strides[ 0 ];
         }

         return PotentialTable( newTable, newDomain, newCardinality );
      }

      /**
       @brief This method will sample the value 'domain[0]' wich we call main domain given the other evidence of the other variables
       @param inout_evidenceWithoutMainDomain the evidence. If inout_evidenceWithoutMainDomain[ 0 ] != 0, it will be discarded. Only
              inout_evidenceWithoutMainDomain[ 0 ] will be updated
       @TODO relatively inefficient with repeated <computeStrides> calls and pbs copies...
       */
      void sample( EvidenceValue& inout_evidenceWithoutMainDomain ) const
      {
         ensure( _domain.size(), "no domain, can't do sampling!" );
         inout_evidenceWithoutMainDomain[ 0 ] = 0; // remove main domain if it was set

         // get the index in the table. As the main domain is alwyas domain[0], the probabilities we are interested in are contiguous and of size cardinality[0]
         // now given this set of probabilities, randomly sample a value
         std::vector<ui32> strides;
         computeStrides( strides );

         core::Buffer1D<float> pbs( _cardinality[ 0 ] );
         const ui32 baseIndex = getIndexFromEvent( strides, inout_evidenceWithoutMainDomain );
         for ( ui32 i = 0; i < _cardinality[ 0 ]; ++i )
         {
            pbs[ i ] = (float)_table[ baseIndex + i ];
         }
         core::Buffer1D<ui32> samples = core::sampling( pbs, 1 );
         inout_evidenceWithoutMainDomain[ 0 ] = samples[ 0 ];  // export the sampled main domain
      }

      /**
       @brief Given a fully observed dataset, find table parameters using maximum likelihood estimation
       @param domainData the domain of the data stored in <fullyObservedData>
       @param fullyObservedData the data observed. It is assumed each instance is iid.
       */
      void maximumLikelihoodEstimate( const VectorI& domainData, const std::vector<EvidenceValue>& fullyObservedData )
      {
         if ( _domain.size() == 0 )
            return;

         // first for efficiency, compute the evidence index related to this potential table domain
         std::vector<ui32> domainMapper( _domain.size() );
         for ( ui32 n = 0; n < _domain.size(); ++n )
         {
            VectorI::const_iterator it = std::find( domainData.begin(), domainData.end(), _domain[ n ] );
            ensure( it != domainData.end(), "a domain in CPD can't be found in the fully observed data" );
            domainMapper[ n ] = it - domainData.begin();
         }

         // reset the table;
         for ( ui32 n = 0; n < _table.size(); ++n )
         {
            _table[ n ] = 0;
         }

         // do the counting
         std::vector<ui32> strides;
         computeStrides( strides );
         EvidenceValue event( _domain.size() );
         for ( size_t dataId = 0; dataId < fullyObservedData.size(); ++dataId )
         {
            const EvidenceValue& fullEvent = fullyObservedData[ dataId ];
            for ( ui32 n = 0; n < _domain.size(); ++n )
            {
               event[ n ] = fullEvent[ domainMapper[ n ] ];
            }
            const ui32 index = getIndexFromEvent( strides, event );
            ++_table[ index ];
         }

         // finally normalize and we are done
         normalize();
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
         std::vector<ui32> strides;
         computeStrides( strides );

         // now compute the table index
         const ui32 index = getIndexFromEvent( strides, event );
         return _table[ index ];
      }

      /**
       @brief computes p(X | Y=y) i.e., entering evidence
       @param evidenceValue the evidence of Y=y
       @param varIndexToRemove the index of Y's, must be sorted 0->+inf
       */
      PotentialTable conditioning( const EvidenceValue& evidenceValue, const VectorI& varIndexToRemove ) const
      {
         ensure( varIndexToRemove.size(), "empty set" );

         int size = (int)varIndexToRemove.size();
         PotentialTable p = conditioning( evidenceValue[ 0 ], varIndexToRemove[ 0 ] );
         for ( int n = 1; n < size; ++n )
         {
            p = p.conditioning( evidenceValue[ n ], varIndexToRemove[ n ] );
         }
         return p;
      }

      /**
       @brief Normalize the potential so that it represents a PDF i.e. integral(-inf,+inf)p(x | Z) = 1, with x = domain[ 0 ], Z = rest of the domain

       This is valid only if there is an associated domain, else no normalization is done
       */
      void normalize()
      {
         if ( _domain.size() )
         {
            // if there is no domain, we don't want to normalize even if the table is not empty!
            // here we want that sum_x p(x | Y ) = 1, so we need to normalize the domain[ 0 ] by summing all its categories probabilities and normalize
            const ui32 nbCategories = _cardinality[ 0 ];
            for ( ui32 index = 0; index < _table.size(); index += nbCategories )
            {
               value_type sum = 0;
               for ( ui32 n = index; n < index + nbCategories; ++n ) // we don't need to check the bounds: we know the table's size is multiple of cardinality[ 0 ]
               {
                  sum += _table[ n ];
               }
               if ( sum >= std::numeric_limits<value_type>::epsilon() )
               {
                  // handle the case where we don't have any counts: just set the proba to 0...
                  for ( ui32 n = index; n < index + nbCategories; ++n )
                  {
                     _table[ n ] /= sum;
                  }
               }
            }
         }
      }

      /**
       @brief Normalize the table so that each entry is a probability (and not just domain[0] as in <normalize>)
       */
      void normalizeFull()
      {
         if ( _domain.size() )
         {
            value_type sum = 0;
            for ( ui32 index = 0; index < _table.size(); ++index )
            {
               sum += _table[ index ];
            }

            if ( sum > 1e-5 )
            {
               for ( ui32 index = 0; index < _table.size(); ++index )
               {
                  _table[ index ] /= sum;
               }
            }
         }
      }

      /**
       @brief Combine the two table potentials
       */
      PotentialTable operator*( const PotentialTable& g2 ) const
      {
         // no more computations, the other one is empty!
         if ( getDomain().size() == 0 && g2.getDomain().size() == 0 )
         {
            return PotentialTable();
         }
         if ( g2.getDomain().size() == 0 )
            return *this;
         if ( getDomain().size() == 0 )
            return g2;

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
      /**
       @brief Given the strides of the domain's variables and an 'event' vector, return the corresponding table index
       */
      template <class VectorT>
      int getIndexFromEvent( const std::vector<ui32>& strides, const VectorT& evidenceValue ) const
      {
         int index = 0;
         ensure( strides.size() == _domain.size(), "must be the same size!" );
         ensure( evidenceValue.size() == _domain.size(), "must be the same size!" );

         for ( ui32 i = 0; i < evidenceValue.size(); ++i )
         {
            index += evidenceValue[ i ] * strides[ i ];
         }
         return index;
      }

      /**
       @brief Given the internal domain and domain's cardinality, computes for each domain variable its stride (i.e., each time a variable is increased by one,
              the index in the table domain is updated by <stride>
       */
      void computeStrides( std::vector<ui32>& strides ) const
      {
         strides = std::vector<ui32>( _domain.size() );
         ui32 stride = 1;
         for ( ui32 n = 0; n < _domain.size(); ++n )
         {
            strides[ n ] = stride;
            stride *= _cardinality[ n ];
         }
      }

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

      /**
       @brief compute P( X, E = e ), i.e. this is unormalized
       @param pe_out if != 0, computes and export P(E) to easily compute P( X | E = e ) = P( X, E ) / P( E )
       */
      PotentialTable conditioning( EvidenceValue::value_type evidence, ui32 varIndexToRemove, value_type* pe_out = 0 ) const
      {
         assert( std::binary_search( _domain.begin(), _domain.end(), varIndexToRemove ) ); // can't find the variable!

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
         ensure( pe >= 0, "p(E = e) <= 0" );

         if ( pe_out )
         {
            *pe_out = pe;
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