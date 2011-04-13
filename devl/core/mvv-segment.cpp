#include "stdafx.h"
#include <mvvPlatform/engine.h>
#include <mvvPlatform/context-volumes.h>
#include "mvv-layout.h"
#include "mvv-segment.h"

#include <mvvPlatform/context-global.h>
#include <mvvScript/function-runnable.h>
#include <mvvScript/compiler-helper.h>

RuntimeValue FunctionSegmentGetRawImage::run( const std::vector<RuntimeValue*>& args )
{
   if ( args.size() != 1 )
   {
      throw std::runtime_error( "unexpected number of arguments" );
   }

   // discard the second arg: just a flag!

   RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!

   // check we have the data
   assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
   Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
   PointeeImage* p = new PointeeImage();

   // wait for all orders to complete...
   platform::ContextGlobal* global = (*_e.getContextExtension()).get<platform::ContextGlobal>();
   if ( !global )
   {
      throw std::runtime_error( "mvv global context has not been initialized" );
   }

   waitUntilSegmentIsReadyToDisplayAllVolumes( pointee->segment, global, _e );
   p->getValue().clone( pointee->segment.getRawMpr().getValue().getStorage() );

   

   // create a runtime value with a destructor
   RuntimeValue rt( RuntimeValue::TYPE );
   Type* t = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
   if ( !t )
   {
      throw std::runtime_error( "internal error: cannot instanciate Image type" );
   }

   RuntimeValues* vals = new RuntimeValues( 1 );
   (*vals)[ 0 ] = RuntimeValue( RuntimeValue::PTR );
   (*vals)[ 0 ].ref = reinterpret_cast<RuntimeValue*>( p );
   rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), t, vals );
   return rt;
}

void waitUntilSegmentIsReadyToDisplayAllVolumes( platform::Segment& segment, platform::ContextGlobal* global, CompilerFrontEnd& e )
{
   // first if a layout is defined, we must updated it (else the segment will have a display size = 0)
   try
   {
      const RuntimeValue& layoutRef = e.getVariable( mvv::Symbol::create( "layout" ) );
      FunctionLayoutConstructorSegment::Pointee* pointeeS = reinterpret_cast<FunctionLayoutConstructorSegment::Pointee*>( (*layoutRef.vals)[ 0 ].ref );
      (*pointeeS->pane).setSize( nll::core::vector2ui( global->screen.sizex(), global->screen.sizey() ) );
      (*pointeeS->pane).updateLayout();
      std::cout << "layout update size=" << (*pointeeS->pane).getSize()[ 0 ] << ":" << (*pointeeS->pane).getSize()[ 1 ] << std::endl;
   } catch (...)
   {
      // no layout, it is fine if the segment is not in a layout...
   }

   global->engineHandler.run();
   global->orderManager.run();
   global->engineHandler.run();
   global->orderManager.run();
   global->engineHandler.run();
   global->orderManager.run();
   std::cout << "blocking call: synchronize segment display" << std::endl;
   while ( segment.getRemainingOrderToComplete() )
   {
      global->orderManager.run();
      global->engineHandler.run();
   }
}