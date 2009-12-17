#include <tester/register.h>
#include <mvvPlatform/resource.h>

using namespace mvv::platform;

struct TestRefcounted
{
   void func1( const Refcounted& ref, unsigned refExpected )
   {
      TESTER_ASSERT( ref.getNumberOfReference() == refExpected );
   }

   void func2( Refcounted ref, unsigned refExpected )
   {
      TESTER_ASSERT( ref.getNumberOfReference() == refExpected );
   }

   void func3( const Refcounted ref, unsigned refExpected )
   {
      TESTER_ASSERT( ref.getNumberOfReference() == refExpected );
   }

   void test1()
   {
      Refcounted ref1;
      Refcounted ref2 = ref1;
      unsigned nbref = ref2.getNumberOfReference();

      TESTER_ASSERT( ref2.getNumberOfReference() == 2 );
      TESTER_ASSERT( ref1.getNumberOfReference() == 2 );

      func1( ref1, nbref );
      TESTER_ASSERT( ref1.getNumberOfReference() == nbref );
      func2( ref1, nbref + 1 );
      TESTER_ASSERT( ref1.getNumberOfReference() == nbref );
      func3( ref1, nbref + 1 );
      TESTER_ASSERT( ref1.getNumberOfReference() == nbref );
   }

   void test2()
   {
      typedef RefcountedTyped<int>  RefcountedTypedI;

      int* t = new int;
      *t = 42;

      RefcountedTypedI ref1( t );
      RefcountedTypedI ref2 = ref1;
      unsigned nbref = ref2.getNumberOfReference();

      TESTER_ASSERT( ref2.getNumberOfReference() == 2 );
      TESTER_ASSERT( ref1.getNumberOfReference() == 2 );

      func1( ref1, nbref );
      TESTER_ASSERT( ref1.getNumberOfReference() == nbref );
      func2( ref1, nbref + 1 );
      TESTER_ASSERT( ref1.getNumberOfReference() == nbref );
      func3( ref1, nbref + 1 );
      TESTER_ASSERT( ref1.getNumberOfReference() == nbref );

      TESTER_ASSERT( *ref1 == 42 );
   }
};

TESTER_TEST_SUITE(TestRefcounted);
TESTER_TEST(test1);
TESTER_TEST(test2);
TESTER_TEST_SUITE_END();