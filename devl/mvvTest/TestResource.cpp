#include "stdafx.h"
#include <mvv/resource.h>
#include <mvv/resource-manager.h>

using namespace mvv;

class EngineTest : public Engine
{
public:
   bool isNotified()
   {
      return _needToRecompute;
   }

   void _run()
   {
      _needToRecompute = false;
   }

   void consume( Order* )
   {
      // nothing to do
   }
};

class OrderProviderTest : public OrderProvider
{
public:
   virtual Orders getOrdersAndClear()
   {
      return orders;
   }

   Orders orders;
};

class TestResource
{
public:
   void test1()
   {
      ResourceVector3d vector1( 1, 2, 3 );

      EngineTest engine1;
      engine1.run();
      engine1.attach( vector1 ); // specify vector1 is an input

      TESTER_ASSERT( !engine1.isNotified() );
      vector1.setValue( 0, 6 );

      TESTER_ASSERT( engine1.isNotified() );
      engine1.run();

      vector1.getValue( 0 );
      TESTER_ASSERT( !engine1.isNotified() );
   }

   void test2()
   {
      ResourceVector3d& pos = ResourceManager::instance().positionMpr;
      pos.setValue( 0, 10 );
      pos.setValue( 1, 20 );
      pos.setValue( 2, 30 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestResource);
//TESTER_TEST(test1);
//TESTER_TEST(test2);
TESTER_TEST_SUITE_END();
#endif