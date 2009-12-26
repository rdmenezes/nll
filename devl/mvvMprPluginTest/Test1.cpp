#include <tester/register.h>
#include <mvvPlatform/resource-volumes.h>
#include <mvvPlatform/resource-lut.h>
#include <mvvPlatform/resource-typedef.h>
#include <mvvMprPlugin/engine-mpr.h>
#include <mvvPlatform/order-manager-thread-pool.h>


using namespace mvv;
using namespace mvv::platform;

void wait( float seconds )
{
  clock_t endwait;
  endwait = (clock_t)(clock () + seconds * CLOCKS_PER_SEC);
  while (clock() < endwait) {}
}

class EngineOrderTest : public EngineOrder
{
public:
   EngineOrderTest( EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : 
      EngineOrder( handler, provider, dispatcher )
   {
      dispatcher.connect( *this );
   }

   ~EngineOrderTest()
   {
      _dispatcher.disconnect( *this );
   }

   virtual void consume( RefcountedTyped<Order> order )
   {
   }

   virtual const std::set<OrderClassId>& interestedOrder() const
   {
      static std::set<OrderClassId> tmp;
      return tmp;
   }

   virtual bool _run()
   {
      return true;
   }
};

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


struct Test1
{
   void test1()
   {
      Volume ct, pet;
      bool res =  nll::imaging::loadSimpleFlatFile( "../../nllTest/data/medical/1_-CT.mf2", ct );
           res |= nll::imaging::loadSimpleFlatFile( "../../nllTest/data/medical/1_-NAC.mf2", pet );
      TESTER_ASSERT( res ); // error, cant find file

      ResourceStorageVolumes volumesStorage;
      ResourceVolumes   volumes( volumesStorage );
      ResourceVector3f  position;
      ResourceVector3f  directionx;
      ResourceVector3f  directiony;
      ResourceVector3f  panning;
      ResourceVector2f  zoom;
      ResourceVector2ui size;
      ResourceMapTransferFunction lut;
      ResourceFloats    intensities;
      ResourceBool      isInteracting;
      ResourceInterpolationMode interpolation;

      // configure:
      SymbolVolume cts = SymbolVolume::create( "ct" );
      SymbolVolume pets = SymbolVolume::create( "pet" );
      volumesStorage.insert( cts,  RefcountedTyped<Volume>( &ct, false ) );
      volumesStorage.insert( pets, RefcountedTyped<Volume>( &pet, false ) );
      volumes.insert( cts );
      volumes.insert( pets );

      position.setValue( ct.indexToPosition( nll::core::vector3f( ct.getSize()[ 0 ] / 2.0f,
                                                                  ct.getSize()[ 1 ] / 2.0f,
                                                                  ct.getSize()[ 2 ] / 2.0f ) ) );


      directionx.setValue( nll::core::vector3f( 1, 0, 0 ) );
      directiony.setValue( nll::core::vector3f( 0, 1, 0 ) );
      zoom.setValue( nll::core::vector2f( 10.0f, 10.0f ) );
      size.setValue( nll::core::vector2ui( 1024, 1024 ) );

      ResourceLut lutPet( 0, 5000 );
      ResourceLut lutCt( 100, 1100 );
      lut.insert( cts, lutCt );
      lut.insert( pets, lutPet );

      intensities.insert( cts, 0.5f );
      intensities.insert( pets, 0.5f );

      isInteracting.setValue( false );

      // run
      DummyEngineHandler handler;
      OrderManagerThreadPool manager( 4 );

      /*
      EngineOrderTest test( handler,
                            manager,
                            manager );*/
     // manager.connect( test );
      
      EngineMpr mpr( volumes,
                     position,
                     directionx,
                     directiony,
                     panning,
                     zoom,
                     size,
                     lut,
                     intensities,
                     isInteracting,
                     interpolation,
                     handler,
                     manager,
                     manager,
                     false );

      handler.run();
      manager.run();
      wait( 4.0f );
      handler.run();
      manager.run();
      wait( 4.0f );
      handler.run();
      manager.run();
      wait( 4.0f );
      handler.run();
      manager.run();
   }   
};

TESTER_TEST_SUITE(Test1);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();