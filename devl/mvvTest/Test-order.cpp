#include "stdafx.h"
#include <mvv/order.h>
#include <mvv/queue-orders.h>

namespace mvv
{
   class OrderResultDerived1 : public OrderResult
   {
   };

   class OrderResultDerived2 : public OrderResult
   {
   };

   class OrderResultDerived3 : public OrderResult
   {
   };

   MVV_BIND_ORDER_RESULT( -1, OrderResultDerived1 );
   MVV_BIND_ORDER_RESULT( -2, OrderResultDerived2 );
   MVV_BIND_ORDER_RESULT( -3, OrderResultDerived3 );
}

class TestOrder
{
public:
   void testBinder()
   {
      typedef mvv::BindOrderResultType<-1>::value_type v1;
      typedef mvv::BindOrderResultType<-2>::value_type v2;
      typedef mvv::BindOrderResultType<-3>::value_type v3;

      enum
      {
         VALUE1 = nll::core::Equal<v1, mvv::OrderResultDerived1>::value
      };

      enum
      {
         VALUE2 = nll::core::Equal<v2, mvv::OrderResultDerived2>::value
      };

      enum
      {
         VALUE3 = nll::core::Equal<v3, mvv::OrderResultDerived3>::value
      };

      // in case these static asserts fail, a compile error will be generated
      STATIC_ASSERT( VALUE1 );
      STATIC_ASSERT( VALUE2 );
      STATIC_ASSERT( VALUE3 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestOrder);
//TESTER_TEST(testBinder);
TESTER_TEST_SUITE_END();
#endif