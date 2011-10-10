#include <nll/nll.h>
#include <tester/register.h>

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

      const Vector& getTable() const
      {
         return _table;
      }

      Vector& getTable()
      {
         return _table;
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
            extended1.getTable()[ n ] *= extended2.getTable()[ n ];
         }
         return extended1;
      }

      PotentialTable extendDomain( const VectorI& domain, const VectorI& cardinality ) const
      {
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

         // compute p(E=e)
         value_type pe = 0;
         for ( ui32 index = evidence * stride; index < _table.size(); index += stride * _cardinality[ removedIndex ] )
         {
            for ( ui32 c = 0; c < _cardinality[ removedIndex ]; ++c )
            {
               pe += _table[ index + c ];
            }
         }
         ensure( pe > 0, "p(E = e) <= 0" );
         
         // now remove the evidence from the table and normalize
         const ui32 newSize = getTableSize( newCardinality );
         Vector newTable( newSize );
         for ( ui32 index = 0; index < newSize; )
         {
            const ui32 indexSrc = stride * ( evidence + index * _cardinality[ removedIndex ] );
            for ( ui32 nn = 0; nn < _cardinality[ removedIndex ]; ++nn )
            {
               const ui32 indexRef = indexSrc + nn;
               newTable[ index++ ] = _table[ indexRef ] / pe;
            }
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

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;

class TestPotentialTable
{
public:
   void testMul1()
   {
      PotentialTable::VectorI domain1( 2 );
      domain1[ 0 ] = 0;
      domain1[ 1 ] = 2;
      PotentialTable::VectorI multiplicity1( 2 );
      multiplicity1[ 0 ] = 3;
      multiplicity1[ 1 ] = 2;
      PotentialTable::VectorI domain2( 1 );
      domain2[ 0 ] = 1;
      PotentialTable::VectorI multiplicity2( 1 );
      multiplicity2[ 0 ] = 2;
      PotentialTable::Vector table( 6 );
      for ( ui32 n = 0; n < table.size(); ++n )
      {
         table[ n ] = n + 1;
      }

      PotentialTable p1( table, domain1, multiplicity1 );
      PotentialTable p2 = p1.extendDomain( domain2, multiplicity2 );
      p2.getTable().print( std::cout );

      TESTER_ASSERT( p2.getTable().size() == 12 );
      TESTER_ASSERT( p2.getTable()[ 0 ] == 1 );
      TESTER_ASSERT( p2.getTable()[ 1 ] == 2 );
      TESTER_ASSERT( p2.getTable()[ 2 ] == 3 );
      TESTER_ASSERT( p2.getTable()[ 3 ] == 1 );
      TESTER_ASSERT( p2.getTable()[ 4 ] == 2 );
      TESTER_ASSERT( p2.getTable()[ 5 ] == 3 );
      TESTER_ASSERT( p2.getTable()[ 6 ] == 4 );
      TESTER_ASSERT( p2.getTable()[ 7 ] == 5 );
      TESTER_ASSERT( p2.getTable()[ 8 ] == 6 );
      TESTER_ASSERT( p2.getTable()[ 9 ] == 4 );
      TESTER_ASSERT( p2.getTable()[ 10 ] == 5 );
      TESTER_ASSERT( p2.getTable()[ 11 ] == 6 );
   }

   void testMul2()
   {
      PotentialTable::VectorI domain1( 2 );
      domain1[ 0 ] = 1;
      domain1[ 1 ] = 2;
      PotentialTable::VectorI multiplicity1( 2 );
      multiplicity1[ 0 ] = 3;
      multiplicity1[ 1 ] = 2;
      PotentialTable::VectorI domain2( 1 );
      domain2[ 0 ] = 0;
      PotentialTable::VectorI multiplicity2( 1 );
      multiplicity2[ 0 ] = 2;
      PotentialTable::Vector table( 6 );
      for ( ui32 n = 0; n < table.size(); ++n )
      {
         table[ n ] = n + 1;
      }

      PotentialTable p1( table, domain1, multiplicity1 );
      PotentialTable p2 = p1.extendDomain( domain2, multiplicity2 );
      p2.getTable().print( std::cout );

      TESTER_ASSERT( p2.getTable().size() == 12 );
      TESTER_ASSERT( p2.getTable()[ 0 ] == 1 );
      TESTER_ASSERT( p2.getTable()[ 1 ] == 1 );
      TESTER_ASSERT( p2.getTable()[ 2 ] == 2 );
      TESTER_ASSERT( p2.getTable()[ 3 ] == 2 );
      TESTER_ASSERT( p2.getTable()[ 4 ] == 3 );
      TESTER_ASSERT( p2.getTable()[ 5 ] == 3 );
      TESTER_ASSERT( p2.getTable()[ 6 ] == 4 );
      TESTER_ASSERT( p2.getTable()[ 7 ] == 4 );
      TESTER_ASSERT( p2.getTable()[ 8 ] == 5 );
      TESTER_ASSERT( p2.getTable()[ 9 ] == 5 );
      TESTER_ASSERT( p2.getTable()[ 10 ] == 6 );
      TESTER_ASSERT( p2.getTable()[ 11 ] == 6 );
   }

   void testMul3()
   {
      PotentialTable::VectorI domain1( 2 );
      domain1[ 0 ] = 0;
      domain1[ 1 ] = 1;
      PotentialTable::VectorI multiplicity1( 2 );
      multiplicity1[ 0 ] = 3;
      multiplicity1[ 1 ] = 2;
      PotentialTable::VectorI domain2( 1 );
      domain2[ 0 ] = 2;
      PotentialTable::VectorI multiplicity2( 1 );
      multiplicity2[ 0 ] = 2;
      PotentialTable::Vector table( 6 );
      for ( ui32 n = 0; n < table.size(); ++n )
      {
         table[ n ] = n + 1;
      }

      PotentialTable p1( table, domain1, multiplicity1 );
      PotentialTable p2 = p1.extendDomain( domain2, multiplicity2 );
      p2.getTable().print( std::cout );

      TESTER_ASSERT( p2.getTable().size() == 12 );
      TESTER_ASSERT( p2.getTable()[ 0 ] == 1 );
      TESTER_ASSERT( p2.getTable()[ 1 ] == 2 );
      TESTER_ASSERT( p2.getTable()[ 2 ] == 3 );
      TESTER_ASSERT( p2.getTable()[ 3 ] == 4 );
      TESTER_ASSERT( p2.getTable()[ 4 ] == 5 );
      TESTER_ASSERT( p2.getTable()[ 5 ] == 6 );
      TESTER_ASSERT( p2.getTable()[ 6 ] == 1 );
      TESTER_ASSERT( p2.getTable()[ 7 ] == 2 );
      TESTER_ASSERT( p2.getTable()[ 8 ] == 3 );
      TESTER_ASSERT( p2.getTable()[ 9 ] == 4 );
      TESTER_ASSERT( p2.getTable()[ 10 ] == 5 );
      TESTER_ASSERT( p2.getTable()[ 11 ] == 6 );
   }

   void testMul4()
   {
      PotentialTable::VectorI domain1( 2 );
      domain1[ 0 ] = 0;
      domain1[ 1 ] = 2;
      PotentialTable::VectorI multiplicity1( 2 );
      multiplicity1[ 0 ] = 2;
      multiplicity1[ 1 ] = 2;
      PotentialTable::VectorI domain2( 2 );
      domain2[ 0 ] = 1;
      domain2[ 1 ] = 3;
      PotentialTable::VectorI multiplicity2( 2 );
      multiplicity2[ 0 ] = 2;
      multiplicity2[ 1 ] = 2;
      PotentialTable::Vector table( 4 );
      for ( ui32 n = 0; n < table.size(); ++n )
      {
         table[ n ] = n + 1;
      }

      PotentialTable p1( table, domain1, multiplicity1 );
      PotentialTable p2 = p1.extendDomain( domain2, multiplicity2 );
      p2.getTable().print( std::cout );

      TESTER_ASSERT( p2.getTable().size() == 16 );
      TESTER_ASSERT( p2.getTable()[ 0 ] == 1 );
      TESTER_ASSERT( p2.getTable()[ 1 ] == 2 );
      TESTER_ASSERT( p2.getTable()[ 2 ] == 1 );
      TESTER_ASSERT( p2.getTable()[ 3 ] == 2 );
      TESTER_ASSERT( p2.getTable()[ 4 ] == 3 );
      TESTER_ASSERT( p2.getTable()[ 5 ] == 4 );
      TESTER_ASSERT( p2.getTable()[ 6 ] == 3 );
      TESTER_ASSERT( p2.getTable()[ 7 ] == 4 );

      TESTER_ASSERT( p2.getTable()[ 8 ] == 1 );
      TESTER_ASSERT( p2.getTable()[ 9 ] == 2 );
      TESTER_ASSERT( p2.getTable()[ 10 ] == 1 );
      TESTER_ASSERT( p2.getTable()[ 11 ] == 2 );
      TESTER_ASSERT( p2.getTable()[ 12 ] == 3 );
      TESTER_ASSERT( p2.getTable()[ 13 ] == 4 );
      TESTER_ASSERT( p2.getTable()[ 14 ] == 3 );
      TESTER_ASSERT( p2.getTable()[ 15 ] == 4 );
   }

   void testMarginalization1()
   {
      double vals[] =
      {
         0.25, 0.35, 0.08, 0.16, 0.05, 0.07, 0, 0, 0.15, 0.21, 0.09, 0.18
      };

      PotentialTable::VectorI domain1( 3 );
      domain1[ 0 ] = 0;
      domain1[ 1 ] = 1;
      domain1[ 2 ] = 2;

      PotentialTable::VectorI multiplicity1( 3 );
      multiplicity1[ 0 ] = 2;
      multiplicity1[ 1 ] = 2;
      multiplicity1[ 2 ] = 3;

      PotentialTable::Vector table( vals, 12, false );
      PotentialTable p1( table, domain1, multiplicity1 );

      PotentialTable::VectorI var( 1 );
      var[ 0 ] = 1;

      PotentialTable p2 = p1.marginalization( var );

      TESTER_ASSERT( p2.getTable().size() == 6 );
      TESTER_ASSERT( p2.getTable()[ 0 ] == 0.33 );
      TESTER_ASSERT( p2.getTable()[ 1 ] == 0.51 );
      TESTER_ASSERT( p2.getTable()[ 2 ] == 0.05 );
      TESTER_ASSERT( p2.getTable()[ 3 ] == 0.07 );
      TESTER_ASSERT( p2.getTable()[ 4 ] == 0.24 );
      TESTER_ASSERT( p2.getTable()[ 5 ] == 0.39 );
   }

   void testMarginalization2()
   {
      double vals[] =
      {
         0.25, 0.35, 0.08, 0.16, 0.05, 0.07, 0, 0, 0.15, 0.21, 0.09, 0.18
      };

      PotentialTable::VectorI domain1( 3 );
      domain1[ 0 ] = 0;
      domain1[ 1 ] = 1;
      domain1[ 2 ] = 2;

      PotentialTable::VectorI multiplicity1( 3 );
      multiplicity1[ 0 ] = 2;
      multiplicity1[ 1 ] = 2;
      multiplicity1[ 2 ] = 3;

      PotentialTable::Vector table( vals, 12, false );
      PotentialTable p1( table, domain1, multiplicity1 );

      PotentialTable::VectorI var( 1 );
      var[ 0 ] = 0;

      PotentialTable p2 = p1.marginalization( var );
      p2.print( std::cout );

      TESTER_ASSERT( p2.getTable().size() == 6 );
      TESTER_ASSERT( core::equal( p2.getTable()[ 0 ], 0.6, 1e-3 ) );
      TESTER_ASSERT( core::equal( p2.getTable()[ 1 ], 0.24, 1e-3 ) );
      TESTER_ASSERT( core::equal( p2.getTable()[ 2 ], 0.12, 1e-3 ) );
      TESTER_ASSERT( core::equal( p2.getTable()[ 3 ], 0.0, 1e-3 ) );
      TESTER_ASSERT( core::equal( p2.getTable()[ 4 ], 0.36, 1e-3 ) );
      TESTER_ASSERT( core::equal( p2.getTable()[ 5 ], 0.27, 1e-3 ) );
   }

   void testMarginalization3()
   {
      double vals[] =
      {
         0.25, 0.35, 0.08, 0.16, 0.05, 0.07, 0, 0, 0.15, 0.21, 0.09, 0.18
      };

      PotentialTable::VectorI domain1( 3 );
      domain1[ 0 ] = 0;
      domain1[ 1 ] = 1;
      domain1[ 2 ] = 2;

      PotentialTable::VectorI multiplicity1( 3 );
      multiplicity1[ 0 ] = 2;
      multiplicity1[ 1 ] = 2;
      multiplicity1[ 2 ] = 3;

      PotentialTable::Vector table( vals, 12, false );
      PotentialTable p1( table, domain1, multiplicity1 );

      PotentialTable::VectorI var( 1 );
      var[ 0 ] = 2;

      PotentialTable p2 = p1.marginalization( var );
      p2.print( std::cout );

      TESTER_ASSERT( p2.getTable().size() == 4 );
      TESTER_ASSERT( core::equal( p2.getTable()[ 0 ], 0.45, 1e-3 ) );
      TESTER_ASSERT( core::equal( p2.getTable()[ 1 ], 0.63, 1e-3 ) );
      TESTER_ASSERT( core::equal( p2.getTable()[ 2 ], 0.17, 1e-3 ) );
      TESTER_ASSERT( core::equal( p2.getTable()[ 3 ], 0.34, 1e-3 ) );
   }
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestPotentialTable);
TESTER_TEST(testMul1);
TESTER_TEST(testMul2);
TESTER_TEST(testMul3);
TESTER_TEST(testMul4);
TESTER_TEST(testMarginalization1);
TESTER_TEST(testMarginalization2);
TESTER_TEST(testMarginalization3);
TESTER_TEST_SUITE_END();
#endif