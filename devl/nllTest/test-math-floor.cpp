#include <nll/nll.h>
#include <tester/register.h>

struct TestMathFloor
{
   void testFloor()
   {
      nll::core::Timer t1;
      double val = 0.1;
      int sum = 0;
      for ( unsigned n = 0; n < 85000000; ++n, val += 0.4 )
      {
         int valn = nll::core::floor( val );
         sum += valn;
      }
      double time1 = t1.getCurrentTime();

      nll::core::Timer t2;
      double val2 = 0.1;
      int sum2 = 0;
      for ( unsigned n = 0; n < 85000000; ++n, val2 += 0.4 )
      {
         int valn = (int)std::floor( val2 );
         sum2 += valn;
      }
      double time2 = t2.getCurrentTime();
      std::cout << "Time core::floor=" << time1 << std::endl;
      std::cout << "Time std::floor=" << time2 << std::endl;
      TESTER_ASSERT( sum2 == sum ); // check we have the same result!
#if defined( _MSC_VER ) && defined( _M_IX86 )
         TESTER_ASSERT( time1 <= time2 ); // the assembly one must be faster, else equal if std version is used
#endif
   }

   void testTruncateu()
   {
      nll::core::Timer t1;
      double val = 0.1;
      unsigned int sum = 0;
      for ( unsigned n = 0; n < 85000000; ++n, val += 0.4 )
      {
         unsigned int valn = nll::core::truncateu( val );
         sum += valn;
      }
      double time1 = t1.getCurrentTime();

      nll::core::Timer t2;
      double val2 = 0.1;
      unsigned int sum2 = 0;
      for ( unsigned n = 0; n < 85000000; ++n, val2 += 0.4 )
      {
         unsigned int valn = (unsigned int)( val2 );
         sum2 += valn;
      }
      double time2 = t2.getCurrentTime();
      std::cout << "Time core::truncateu=" << time1 << std::endl;
      std::cout << "Time default" << time2 << std::endl;
      TESTER_ASSERT( sum2 == sum ); // check we have the same result!

#if defined( _MSC_VER ) && defined( _M_IX86 )
      TESTER_ASSERT( time1 <= time2 ); // else default implementation so useless to test...
#endif
   }

   /*
   void testTruncate()
   {
      nll::core::Timer t1;
      double val = 0.1;
      int sum = 0;
      for ( unsigned n = 0; n < 85000000; ++n, val += 0.4 )
      {
         int valn = nll::core::truncate( val );
         sum += valn;
      }
      double time1 = t1.getCurrentTime();

      nll::core::Timer t2;
      double val2 = 0.1;
      int sum2 = 0;
      for ( unsigned n = 0; n < 85000000; ++n, val2 += 0.4 )
      {
         int valn = (int)( val2 );
         sum2 += valn;
      }
      double time2 = t2.getCurrentTime();
      std::cout << "Time core::truncate=" << time1 << std::endl;
      std::cout << "Time default" << time2 << std::endl;
      TESTER_ASSERT( sum2 == sum ); // check we have the same result!
      TESTER_ASSERT( time1 <= time2 ); // the assembly one must be faster, else equal if std version is used
   }*/
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMathFloor);
 TESTER_TEST(testFloor);
 TESTER_TEST(testTruncateu);
 /*TESTER_TEST(testTruncate);*/
TESTER_TEST_SUITE_END();
#endif