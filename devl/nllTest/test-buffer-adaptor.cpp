#include "stdafx.h"
#include <nll/nll.h>

class TestBufferAdaptor
{
public:
   void testAdaptor1()
   {
      srand( 0 );
      typedef std::vector<unsigned>   Array;
      typedef std::vector<Array>      ArrayArray;

      const unsigned size = 100;
      ArrayArray aa( size );
      for ( unsigned n = 0; n < size; ++n )
      {
         const unsigned nb = 1 + ( rand() % 100 );
         Array a( nb );
         for ( unsigned nn = 0; nn < nb; ++nn )
            a[ nn ] = rand();
         aa[ n ] = a;
      }

      typedef nll::core::BufferSelectionAdaptor<ArrayArray> Adaptor;
      for ( unsigned n = 0; n < aa.size(); ++n )
      {
         Adaptor adaptor( aa, n );
         for ( unsigned nn = 0; nn < aa[ n ].size(); ++nn )
         {
            unsigned val_ref = aa[ n ][ nn ];
            unsigned val = adaptor[ nn ];
            TESTER_ASSERT( val == val_ref );
         }
      }
   }

   void testConstAdaptor1()
   {
      srand( 0 );
      typedef std::vector<unsigned>   Array;
      typedef std::vector<Array>      ArrayArray;

      const unsigned size = 100;
      ArrayArray aa( size );
      for ( unsigned n = 0; n < size; ++n )
      {
         const unsigned nb = 1 + ( rand() % 100 );
         Array a( nb );
         for ( unsigned nn = 0; nn < nb; ++nn )
            a[ nn ] = rand();
         aa[ n ] = a;
      }

      typedef nll::core::BufferSelectionConstAdaptor<ArrayArray> ConstAdaptor;
      for ( unsigned n = 0; n < aa.size(); ++n )
      {
         ConstAdaptor adaptor( aa, n );
         for ( unsigned nn = 0; nn < aa[ n ].size(); ++nn )
         {
            unsigned val_ref = aa[ n ][ nn ];
            unsigned val = adaptor[ nn ];
            TESTER_ASSERT( val == val_ref );
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestBufferAdaptor);
TESTER_TEST(testAdaptor1);
TESTER_TEST(testConstAdaptor1);
TESTER_TEST_SUITE_END();
#endif