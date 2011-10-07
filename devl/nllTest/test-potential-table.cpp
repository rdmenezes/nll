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
         ensure( _domain.size() == cardinality.size(), "missing id" );
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
         PotentialTable extended1 = extendDomain( g2.getDomain(), g2.getCardinality() );
         PotentialTable extended2 = g2.extendDomain( _domain, _cardinality );
         const ui32 size = extended1.getTable().size();
         for ( ui32 n = 0; n < size; ++n )
         {
            
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

      // given the current domain, extend the table to another domain
      Vector extend( const VectorI& domain, const VectorI& cardinality, VectorI& newDomain_out, VectorI& newCardinality_out ) const
      {
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
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestPotentialTable);
TESTER_TEST(testMul1);
TESTER_TEST(testMul2);
TESTER_TEST(testMul3);
TESTER_TEST_SUITE_END();
#endif