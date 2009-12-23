#include <tester/register.h>
#include <mvvPlatform/types.h>
#include <mvvPlatform/resource.h>
#include <mvvPlatform/resource-volumes.h>
#include <mvvPlatform/resource-storage-volumes.h>
#include <mvvPlatform/resource-map.h>


using namespace mvv;
using namespace mvv::platform;

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
   
};

TESTER_TEST_SUITE(TestResource);
TESTER_TEST(testResourceVolumes);
TESTER_TEST(testResourceMap);
TESTER_TEST_SUITE_END();