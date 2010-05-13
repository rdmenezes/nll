#ifndef CORE_MVV_SEGMENT_H_
# define CORE_MVV_SEGMENT_H_

# include "core.h"

# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

# include <mvvPlatform/context-global.h>

# include <mvvMprPlugin/context-segments.h>
# include <mvvMprPlugin/segment-tool-pointer.h>
# include <mvvMprPlugin/segment-tool-camera.h>
# include <mvvMprPlugin/segment-tool-annotations.h>
# include <mvvMprPlugin/segment-tool-autocenter.h>
# include <mvvMprPlugin/annotation-point.h>

using namespace mvv::parser;
using namespace mvv;

namespace impl
{
   struct SegmentStorage
   {
      SegmentStorage( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) :
         segment( storage, handler, provider, dispatcher )
      {}

      platform::Segment segment;
   };
}

class FunctionSegmentConstructor: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

public:
   FunctionSegmentConstructor( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 5 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      RuntimeValue& v5 = unref( *args[ 4 ] );

      if ( v2.type != RuntimeValue::TYPE ||
           v3.type != RuntimeValue::TYPE ||
           v4.type != RuntimeValue::TYPE ||
           v5.type != RuntimeValue::TYPE )
      {
         throw RuntimeException( "wrong arguments: expecting VolumeContainer, Vector3f, Vector3f, Vector3f" );
      }

      // read the storage value
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionVolumeContainerConstructor::Pointee* storage = reinterpret_cast<FunctionVolumeContainerConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // read the other parameters
      nll::core::vector3f axisx;
      getVector3fValues( v3, axisx );
      nll::core::vector3f axisy;
      getVector3fValues( v4, axisy );
      nll::core::vector3f position;
      getVector3fValues( v5, position );

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw RuntimeException( "mvv global context has not been initialized" );
      }



      // construct the type
      Pointee* pointee = new Pointee( storage->volumes.getStorage(), global->engineHandler, global->orderManager, global->orderManager );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
   Context& _context;
};

class FunctionSegmentDestructor: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

public:
   FunctionSegmentDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // deallocate data
      delete pointee;
      (*v1.vals)[ 0 ].ref = 0;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionSegmentRefresh: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

public:
   FunctionSegmentRefresh( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // refresh the segment (force the tools to redraw...)
      pointee->segment.refreshTools();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

#endif