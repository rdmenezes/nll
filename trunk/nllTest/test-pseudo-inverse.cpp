#include <nll/nll.h>
#include <iostream>
#include <tester/register.h>

struct TestPseudoInverse
{
   void testSingular()
   {
      nll::core::Matrix<double> m( 3, 3 );

      int n = 0;
      for ( unsigned y = 0; y < m.sizey(); ++y )
      {
         for ( unsigned x = 0; x < m.sizex(); ++x )
         {
            m( y, x ) = n;
            ++n;
         }
      }

      nll::core::Matrix<double> pm = nll::core::pseudoInverse( m );
      nll::core::Matrix<double> t = m * pm * m;

      for ( unsigned y = 0; y < m.sizey(); ++y )
      {
         for ( unsigned x = 0; x < m.sizex(); ++x )
         {
            TESTER_ASSERT( fabs( t( y, x ) - m( y, x ) ) < 1e-10 );
         }
      }
   }

   void testMultiple()
   {
      std::cout << "test pseudo inverse:";
      for ( unsigned  nn = 0; nn < 1000; ++nn )
      {
         if ( nn % 50 == 0 )
            std::cout << "#";
         nll::core::Matrix<double> m( ( rand() % 100 ) + 1, ( rand() % 100 ) + 1 );
         for ( unsigned y = 0; y < m.sizey(); ++y )
         {
            for ( unsigned x = 0; x < m.sizex(); ++x )
            {
               m( y, x ) = nll::core::generateUniformDistribution( -1000, 1000 );
            }
         }

         nll::core::Matrix<double> pm = nll::core::pseudoInverse( m );
         nll::core::Matrix<double> t = m * pm * m;

         for ( unsigned y = 0; y < m.sizey(); ++y )
         {
            for ( unsigned x = 0; x < m.sizex(); ++x )
            {
               TESTER_ASSERT( fabs( t( y, x ) - m( y, x ) ) < 1e-4 );
            }
         }
      }
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestPseudoInverse);
 TESTER_TEST(testSingular);
 TESTER_TEST(testMultiple);
TESTER_TEST_SUITE_END();
#endif
