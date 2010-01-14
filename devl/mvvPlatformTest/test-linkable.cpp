#include <tester/register.h>
#include <mvvPlatform/linkable.h>

using namespace mvv::platform;

class LinkableDouble1;
class LinkableDouble2;

class LinkableDouble1 : public LinkableDouble<LinkableDouble2*, LinkableDouble1*>
{
public:
   typedef LinkableDouble2*   ObjectToHold;

   LinkableDouble1()
   {}

   virtual ~LinkableDouble1()
   {
      removeConnections();
   }

   virtual void connect( ObjectToHold o );

   virtual void disconnect( ObjectToHold o );
};

class LinkableDouble2 : public LinkableDouble<LinkableDouble1*, LinkableDouble2*>
{
public:
   typedef LinkableDouble1*   ObjectToHold;

   LinkableDouble2()
   {}

   virtual ~LinkableDouble2()
   {
      removeConnections();
   }

   virtual void connect( ObjectToHold o );

   virtual void disconnect( ObjectToHold o );
};

void LinkableDouble1::connect( LinkableDouble1::ObjectToHold o )
{
   o->_addSimpleLink( this );
   _addSimpleLink( o );
}

void LinkableDouble1::disconnect( LinkableDouble1::ObjectToHold o )
{
   o->_eraseSimpleLink( this );
   _eraseSimpleLink( o );
}

void LinkableDouble2::connect( LinkableDouble2::ObjectToHold o )
{
   o->_addSimpleLink( this );
   _addSimpleLink( o );
}

void LinkableDouble2::disconnect( LinkableDouble2::ObjectToHold o )
{
   o->_eraseSimpleLink( this );
   _eraseSimpleLink( o );
}

struct TestLinkable
{
   void test1()
   {
      {
         LinkableDouble1 o1;
         {
            LinkableDouble2 i1;
            o1.connect( &i1 );
            i1.connect( &o1 );
         }
      }
/*
      {
         LinkableDouble2 o1;
         {
            LinkableDouble1 i1;
            o1.connect( &i1 );
            i1.connect( &o1 );
         }
      }
*/
   }
};

TESTER_TEST_SUITE(TestLinkable);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();