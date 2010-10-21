#ifndef CORE_MVV_SEGMENT_H_
# define CORE_MVV_SEGMENT_H_

# include "core.h"
# include "mvv-volume-container.h"
# include "mvv-segment-tool.h"
# include "mvv-manipulators.h"

# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

# include <mvvPlatform/context-global.h>
# include <mvvPlatform/resource-storage-volumes.h>

# include <mvvMprPlugin/context-segments.h>
# include <mvvMprPlugin/segment-tool-pointer.h>
# include <mvvMprPlugin/segment-tool-camera.h>
# include <mvvMprPlugin/segment-tool-annotations.h>
# include <mvvMprPlugin/segment-tool-autocenter.h>
# include <mvvMprPlugin/annotation-point.h>

# include "mvv-image.h"
# include "mvv-imagef.h"

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

namespace impl
{
   struct SegmentStorage
   {
      SegmentStorage( ResourceVolumes& volumes, ResourceMapTransferFunction& luts, ResourceFloats& intensities, ResourceMapRegistrations& regs, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) :
         segment( volumes.getStorage(), handler, provider, dispatcher )
      {
         
         segment.volumes = volumes;
         segment.intensities = intensities;
         segment.luts = luts;
         segment.registrations = regs;   
      }

      platform::Segment segment;

      // hold a reference on other objects
      RuntimeValue      volumeContainer;
      RuntimeValue      toolPointer;
      RuntimeValue      toolCamera;
      RuntimeValue      toolAnnotations;
      RuntimeValue      toolAutocenter;
      RuntimeValue      toolPostprocessing;
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
         throw std::runtime_error( "unexpected number of arguments" );
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
         throw std::runtime_error( "wrong arguments: expecting VolumeContainer, Vector3f, Vector3f, Vector3f" );
      }

      // read the storage value
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionVolumeContainerConstructor::Pointee* storage = reinterpret_cast<FunctionVolumeContainerConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );
      std::cout << "storage @=" << storage->volumes.getStorage().getDataPtr() << std::endl;

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }



      // construct the type
      Pointee* pointee = new Pointee( storage->volumes, storage->luts, storage->intensities, storage->registrations, global->engineHandler, global->orderManager, global->orderManager );
      pointee->volumeContainer = v2;

      // read the other parameters
      nll::core::vector3f axisx;
      getVector3fValues( v3, axisx );
      nll::core::vector3f axisy;
      getVector3fValues( v4, axisy );
      nll::core::vector3f position;
      getVector3fValues( v5, position );

      pointee->segment.directionx.setValue( axisx );
      pointee->segment.directiony.setValue( axisy );
      pointee->segment.position.setValue( position );
      pointee->segment.zoom.setValue( nll::core::vector2f( 1.0f, 1.0f ) );
     // pointee->segment.updateResourceSource();

      std::cout << "nb volumes in light store from segment=" << pointee->segment.volumes.size() << std::endl;


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
         throw std::runtime_error( "unexpected number of arguments" );
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
         throw std::runtime_error( "unexpected number of arguments" );
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

/*
class FunctionSegmentSetToolPointer: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

public:
   FunctionSegmentSetToolPointer( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::TYPE || v2.type != RuntimeValue::TYPE )
      {
         throw std::runtime_error( "wrong arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionSegmentToolPointerConstructor::Pointee* tool = reinterpret_cast<FunctionSegmentToolPointerConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // we first need to remove all the tools of this category to ensure there is no inconsistencies...
      std::set<SegmentToolPointer*> tools = pointee->segment.getTools<SegmentToolPointer>();
      for ( std::set<SegmentToolPointer*>::iterator it = tools.begin(); it != tools.end(); ++it )
      {
         pointee->segment.disconnect( *it );
      }

      // finally add the tool
      pointee->segment.connect( tool );
      pointee->toolPointer = v2;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};
*/

class FunctionSegmentSetToolAutocenter: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

public:
   FunctionSegmentSetToolAutocenter( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::TYPE || v2.type != RuntimeValue::TYPE )
      {
         throw std::runtime_error( "wrong arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionSegmentToolCenteringConstructor::Pointee* tool = reinterpret_cast<FunctionSegmentToolCenteringConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // we first need to remove all the tools of this category to ensure there is no inconsistencies...
      std::set<SegmentToolAutocenter*> tools = pointee->segment.getTools<SegmentToolAutocenter>();
      for ( std::set<SegmentToolAutocenter*>::iterator it = tools.begin(); it != tools.end(); ++it )
      {
         pointee->segment.disconnect( *it );
      }

      // finally add the tool
      pointee->segment.connect( tool );
      pointee->toolAutocenter = v2;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

/*
class FunctionSegmentSetToolCamera: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

public:
   FunctionSegmentSetToolCamera( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::TYPE || v2.type != RuntimeValue::TYPE )
      {
         throw std::runtime_error( "wrong arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionSegmentToolCameraConstructor::Pointee* tool = reinterpret_cast<FunctionSegmentToolCameraConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // we first need to remove all the tools of this category to ensure there is no inconsistencies...
      std::set<SegmentToolCamera*> tools = pointee->segment.getTools<SegmentToolCamera>();
      for ( std::set<SegmentToolCamera*>::iterator it = tools.begin(); it != tools.end(); ++it )
      {
         pointee->segment.disconnect( *it );
      }

      // finally add the tool
      pointee->segment.connect( tool );
      pointee->toolCamera = v2;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};*/

class FunctionSegmentSetToolAnnotations: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

public:
   FunctionSegmentSetToolAnnotations( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::TYPE || v2.type != RuntimeValue::TYPE )
      {
         throw std::runtime_error( "wrong arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionToolAnnotationsConstructor::Pointee* tool = reinterpret_cast<FunctionToolAnnotationsConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // we first need to remove all the tools of this category to ensure there is no inconsistencies...
      std::set<SegmentToolAnnotations*> tools = pointee->segment.getTools<SegmentToolAnnotations>();
      for ( std::set<SegmentToolAnnotations*>::iterator it = tools.begin(); it != tools.end(); ++it )
      {
         pointee->segment.disconnect( *it );
      }

      // finally add the tool
      pointee->segment.connect( &tool->tool );
      pointee->toolAnnotations = v2;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionSegmentSetToolManipulators: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

public:
   FunctionSegmentSetToolManipulators( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::TYPE || v2.type != RuntimeValue::TYPE )
      {
         throw std::runtime_error( "wrong arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionToolManipulatorsConstructor::Pointee* tool = reinterpret_cast<FunctionToolManipulatorsConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // we first need to remove all the tools of this category to ensure there is no inconsistencies...
      std::set<SegmentToolAnnotations*> tools = pointee->segment.getTools<SegmentToolAnnotations>();
      for ( std::set<SegmentToolAnnotations*>::iterator it = tools.begin(); it != tools.end(); ++it )
      {
         pointee->segment.disconnect( *it );
      }

      // finally add the tool
      pointee->segment.connect( &tool->segmentManipulators );
      pointee->toolAnnotations = v2;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionSegmentSetInterpolationNN: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

public:
   FunctionSegmentSetInterpolationNN( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      // discard the second arg: just a flag!

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // refresh the segment (force the tools to redraw...)
      pointee->segment.interpolation.setValue( mvv::platform::NEAREST );
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionSegmentSetInterpolationL: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

public:
   FunctionSegmentSetInterpolationL( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      // discard the second arg: just a flag!

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // refresh the segment (force the tools to redraw...)
      pointee->segment.interpolation.setValue( mvv::platform::LINEAR );
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

void waitUntilSegmentIsReadyToDisplayAllVolumes( platform::Segment& segment, platform::ContextGlobal* global, CompilerFrontEnd& e );


class FunctionSegmentGetRawImage: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;
   typedef FunctionImageConstructor::Pointee PointeeImage;

public:
   FunctionSegmentGetRawImage( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args );

private:
   CompilerFrontEnd& _e;
};

class FunctionSegmentGetImage: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;
   typedef FunctionImageConstructor::Pointee PointeeImage;

public:
   FunctionSegmentGetImage( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
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

      p->getValue().clone( pointee->segment.segment.getValue().getStorage() );

      

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

private:
   CompilerFrontEnd& _e;
};

class FunctionSegmentGetRawImagef: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;
   typedef FunctionImagefConstructor::Pointee PointeeImage;

public:
   FunctionSegmentGetRawImagef( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v2.type != RuntimeValue::TYPE && (*v2.vals).size() == 1 && (*v2.vals)[ 0 ].type == RuntimeValue::STRING )
      {
         throw std::runtime_error( "invalid argument" );
      }

      mvv::SymbolVolume v = mvv::SymbolVolume::create( (*v2.vals)[ 0 ].stringval );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // wait for all orders to complete...
      platform::ContextGlobal* global = (*_e.getContextExtension()).get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      waitUntilSegmentIsReadyToDisplayAllVolumes( pointee->segment, global, _e );


      PointeeImage* p = new PointeeImage();
      ResourceMapImage map = pointee->segment.getRawSlices();
      ResourceImagef rawSlice;
      bool result = map.find( v, rawSlice );
      if ( result )
      {
         p->getValue().clone( rawSlice.getValue() );
      } else {
         throw std::runtime_error( "cannot find volume ID:" + std::string( v.getName() ) );
      }

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

private:
   CompilerFrontEnd& _e;
};


#endif