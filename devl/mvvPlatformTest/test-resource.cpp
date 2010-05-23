#include <tester/register.h>
#include <mvvPlatform/types.h>
#include <mvvPlatform/resource.h>
#include <mvvPlatform/resource-volumes.h>
#include <mvvPlatform/resource-storage-volumes.h>
#include <mvvPlatform/resource-map.h>
#include <mvvPlatform/resource-vector.h>
#include <mvvPlatform/resource-barrier.h>
#include <mvvPlatform/engine.h>


using namespace mvv;
using namespace mvv::platform;

namespace
{
   class DummyEngineHandler : public EngineHandler
   {
      typedef std::set<Engine*> Storage;

   public:
      virtual void connect( Engine& e )
      {
         engines.insert( &e );
      }

      virtual void disconnect( Engine& e )
      {
         engines.erase( &e );
      }

      virtual void run()
      {
         for ( Storage::iterator it = engines.begin(); it != engines.end(); ++it )
         {
            (*it)->run();
         }
      }

      Storage engines;
   };

   class DummyEnginea : public Engine
   {
   public:
      DummyEnginea( EngineHandler& handler, impl::Resource& r1  ) : Engine( handler ), r1( r1 )
      {
         r1.connect( this );
         hasBeenRun = false;
      }

      virtual bool _run()
      {
         hasBeenRun = true;
         return true;
      }

      void setStatus( bool run )
      {
         hasBeenRun = run;
      }

      bool getStatus() const
      {
         return hasBeenRun;
      }

      bool isNotified() const
      {
         return _needToRecompute;
      }

      impl::Resource& r1;

   private:
      bool hasBeenRun;
   };

   class DummyResource : public Resource<int>
   {
   public:
      typedef Resource<int>   Base;

      DummyResource( int* i, bool own ) : Resource<int>( i, own )
      {}

      int getValue() const
      {
         return Base::getValue();
      }

      void setValue( int v )
      {
         Base::getValue() = v;
         notify();
      }

      ~DummyResource()
      {
         std::cout << "destroyed" << std::endl;
      }
   };

   class DummyEngine : public Engine
   {
   public:
      DummyEngine( EngineHandler& handler, DummyResource& r1, DummyResource& r2  ) : Engine( handler ), r1( r1 ), r2( r2 )
      {
         r1.connect( this );
         r2.connect( this );
      }

      virtual bool _run()
      {
         res = r1.getValue() + r2.getValue();
         return true;
      }

      int getResult() const
      {
         return res;
      }

      bool needToRecompute() const
      {
         return _needToRecompute;
      }

      DummyResource& r1;
      DummyResource& r2;

   private:
      int res;
   };
}

struct TestResource
{
   void testResourceVolumes()
   {
      std::cout << "----------------------------TEST" << std::endl;
      for ( int n = 0; n < 1000; ++n )
      {
         ResourceStorageVolumes storage;
         Volume* v1 = new Volume();
         Volume* v2 = new Volume();
         Volume* v3 = new Volume();
         Volume* v4 = new Volume();

         storage.insert( SymbolVolume::create("v1"), RefcountedTyped<Volume>( v1 ) );
         storage.insert( SymbolVolume::create("v2"), RefcountedTyped<Volume>( v2 ) );
         storage.insert( SymbolVolume::create("v3"), RefcountedTyped<Volume>( v3 ) );
         storage.insert( SymbolVolume::create("v4"), RefcountedTyped<Volume>( v4 ) );

         ResourceVolumes volumes( storage );
         volumes.insert( SymbolVolume::create("v1") );
         volumes.insert( SymbolVolume::create("v2") );
         volumes.insert( SymbolVolume::create("v3") );

         // check iterator
         {
            ResourceVolumes::Iterator it = volumes.begin();
            TESTER_ASSERT( &(*it).getData() == v1 );

            ++it;
            TESTER_ASSERT( &(*it).getData() == v2 );

            ++it;
            TESTER_ASSERT( &(*it).getData() == v3 );

            TESTER_ASSERT( ++it == volumes.end() );
         }


         // same but remove storage
         {
            storage.erase( SymbolVolume::create("v1") );
            storage.erase( SymbolVolume::create("v3") );

            ResourceVolumes::Iterator it = volumes.begin();
            TESTER_ASSERT( &(*it).getData() == v2 );


            TESTER_ASSERT( ++it == volumes.end() );
         }
      }
      std::cout << "----------------------------SUCESS" << std::endl;
      for ( int n = 0; n < 4 * 1e7; ++n )
         ;
   }

   void testResourceSimpleCount()
   {
      typedef ResourceMap<int, std::string>  Map;

      Map m1;
      Map m2;

      // check operator= has been called only once
      m1.insert( 0, "0" );
      TESTER_ASSERT( m1.getNumberOfReference() == 1 );
      TESTER_ASSERT( m2.getNumberOfReference() == 1 );

      m2 = m1;
      // TODO: don't ref if same pointer!
      TESTER_ASSERT( m1.getNumberOfReference() == 2 );
      TESTER_ASSERT( m2.getNumberOfReference() == 2 );

      m2 = m1;
      TESTER_ASSERT( m1.getNumberOfReference() == 2 );
      TESTER_ASSERT( m2.getNumberOfReference() == 2 );
   }

   void testResourceMap()
   {
      typedef ResourceMap<int, std::string>  Map1;

      Map1 map1;
      map1.insert( 0, "0" );
      map1.insert( 1, "1" );
      map1.insert( 2, "2" );
      map1.insert( 3, "3" );

      int n = 0;
      for ( Map1::Iterator it = map1.begin(); it != map1.end(); ++it, ++n )
      {
         TESTER_ASSERT( n == (*it).first );
         TESTER_ASSERT( nll::core::val2str( n ) == (*it).second );
      }

      n = 0;
      for ( Map1::ConstIterator it = map1.begin(); it != map1.end(); ++it, ++n )
      {
         TESTER_ASSERT( n == (*it).first );
         TESTER_ASSERT( nll::core::val2str( n ) == (*it).second );
      }

      std::string res2;
      bool res = map1.find( 2, res2 );
      TESTER_ASSERT( res && res2 == "2" );
   }
   
   void testResourceScopedBarrier()
   {
      DummyEngineHandler handler;

      ResourceVector2ui r1;
      DummyEnginea e1( handler, r1 );

      TESTER_ASSERT( e1.isNotified() );
      handler.run();
      TESTER_ASSERT( e1.getStatus() && !e1.isNotified() && !r1.needNotification() );
      e1.setStatus( false );

      // do some modification
      {
         ResourceScopedBarrier barrier( r1 );
         r1.setValue( 0, 15 );
         r1.setValue( 1, 16 );

         TESTER_ASSERT( r1.needNotification() );
         handler.run();
         TESTER_ASSERT( !e1.isNotified() );
      }

      TESTER_ASSERT( e1.isNotified() );
      handler.run();
      TESTER_ASSERT( e1.getStatus() && !e1.isNotified() && !r1.needNotification() );
   }

   void testUpdate()
   {
      int n[] = { 1, 2, 3, 4, 5 };
      DummyEngineHandler handler;

      DummyResource n0( n + 0, false );
      DummyResource n1( n + 1, false );
      DummyResource n2( n + 2, false );
      DummyResource n3( n + 3, false );

      // basic test
      DummyEngine e1( handler, n0, n1 );
      handler.run();
      TESTER_ASSERT( e1.getResult() == 3 );

      // now update a resource with a basic one
      n0 = n2;
      n1 = n2;

      handler.run();
      TESTER_ASSERT( e1.getResult() == 6 );

      // now update with connections
      n2 = n3;
      handler.run();
      TESTER_ASSERT( e1.getResult() == 8 );
   }

   void testUpdate2()
   {
      int n[] = { 1, 2, 3, 4, 5 };
      DummyEngineHandler handler;

      DummyResource n0( n + 0, false );
      DummyResource n1( n + 1, false );
      DummyResource n2( n + 2, false );
      DummyResource n3( n + 3, false );
      DummyResource n4( n + 4, false );

      n4.connect( n3 );

      // basic test
      DummyEngine e1( handler, n0, n1 );
      DummyEngine e2( handler, n2, n3 );
      handler.run();
      TESTER_ASSERT( e1.getResult() == 3 );
      TESTER_ASSERT( e2.getResult() == 7 );

      n0 = n1;
      n2 = n3;
      handler.run();
      TESTER_ASSERT( e1.getResult() == 4 );
      TESTER_ASSERT( e2.getResult() == 8 );

      n0 = n3;
      handler.run();
      TESTER_ASSERT( e1.getResult() == 8 );
      TESTER_ASSERT( e2.getResult() == 8 );

      TESTER_ASSERT( !e1.needToRecompute() );
      TESTER_ASSERT( !e2.needToRecompute() );
      n4.notify();
      TESTER_ASSERT( e1.needToRecompute() );
      TESTER_ASSERT( e2.needToRecompute() );
   }

   void testLifetime()
   {
      int n[] = { 1, 2, 3, 4, 5 };
      DummyEngineHandler handler;

      
      {
         DummyResource n1( n + 1, false );
         {
            DummyResource n0( n + 0, false );
            n0.connect( n1 );
         }
      }

      {
         DummyResource n1( n + 1, false );
         {
            DummyResource n0( n + 0, false );
            n1.connect( n0 );
         }
      }

      {
         DummyResource n0( n + 1, false );
         DummyResource n1( n + 1, false );
         {
            DummyEngine e1( handler, n0, n1 );
         }
      }

      {
         DummyResource n0( n + 1, false );
         DummyResource n1( n + 1, false );
         DummyEngine e1( handler, n0, n1 );
         {
            DummyResource n2( n + 1, false );
            e1.connect( &n2 );
         }
      }
   }
};

TESTER_TEST_SUITE(TestResource);
TESTER_TEST(testResourceVolumes);
TESTER_TEST(testResourceMap);
TESTER_TEST(testResourceScopedBarrier);
TESTER_TEST(testResourceSimpleCount);
TESTER_TEST(testUpdate);
TESTER_TEST(testUpdate2);
TESTER_TEST(testLifetime);
TESTER_TEST_SUITE_END();