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
         connect( r1 );
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
}

struct TestResource
{
   void testResourceVolumes()
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
};

TESTER_TEST_SUITE(TestResource);
TESTER_TEST(testResourceVolumes);
TESTER_TEST(testResourceMap);
TESTER_TEST(testResourceScopedBarrier);
TESTER_TEST_SUITE_END();