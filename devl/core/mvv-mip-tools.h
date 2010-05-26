#ifndef CORE_MVV_MIP_TOOLS_H_
# define CORE_MVV_MIP_TOOLS_H_

# include "core.h"

# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

# include <mvvPlatform/context-global.h>

# include <mvvMprPlugin/context-segments.h>
# include <mvvMprPlugin/mip-tool-pointer.h>
# include <mvvMprPlugin/mip.h>

using namespace mvv::parser;
using namespace mvv;

class FunctionMipToolPointerConstructor: public FunctionRunnable
{
public:
   typedef platform::MipToolPointer Pointee;

public:
   FunctionMipToolPointerConstructor( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v2.type != RuntimeValue::CMP_INT )
      {
         throw RuntimeException( "expected int as argument" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw RuntimeException( "mvv global context has not been initialized" );
      }

      // construct the type
      Pointee* pointee = new Pointee( v2.intval );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;
      return v1;  // return the original object!
   }

private:
   Context& _context;
};

class FunctionMipToolPointerDestructor: public FunctionRunnable
{
public:
   typedef platform::MipToolPointer Pointee;

public:
   FunctionMipToolPointerDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

namespace impl
{
   struct MipStorage
   {
      MipStorage( mvv::SymbolVolume& volume, ResourceStorageVolumes& volumeStore, ResourceLut& lut,  EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher, ui32 nbFrames ) :
         mip( volumeStore, handler, provider, dispatcher, nbFrames )
      {
         mip.volumes.insert( volume );
         mip.lut = lut;
      }

      platform::Mip mip;

      // hold a reference on other objects
      RuntimeValue      toolPointer;
      RuntimeValue      toolAnnotations;
   };
}

class FunctionMipConstructor: public FunctionRunnable
{
public:
   typedef ::impl::MipStorage Pointee;

public:
   FunctionMipConstructor( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw RuntimeException( "unexpected number of arguments: VolumeID, Lut, int" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      if ( v4.type != RuntimeValue::CMP_INT || v4.intval <= 0 )
      {
         throw RuntimeException( "expected int > 0" );
      }
      if ( v2.type != RuntimeValue::TYPE && (*v2.vals).size() == 1 && (*v2.vals)[ 0 ].type == RuntimeValue::STRING )
      {
         throw RuntimeException( "invalid argument" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      platform::ContextVolumes* volumes = _context.get<platform::ContextVolumes>();
      if ( !volumes )
      {
         throw RuntimeException( "mvv global || volumes context have not been initialized" );
      }


      mvv::SymbolVolume volume = mvv::SymbolVolume::create( (*v1.vals)[ 0 ].stringval );
      FunctionLutConstructor::Pointee* lutPointee = reinterpret_cast<FunctionLutConstructor::Pointee*>( v3.ref );
      Pointee* pointee = new Pointee( volume, volumes->volumes, *lutPointee, global->engineHandler, global->orderManager, global->orderManager, v3.intval );

      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
   Context& _context;
};

class FunctionMipDestructor: public FunctionRunnable
{
public:
   typedef ::impl::MipStorage Pointee;

public:
   FunctionMipDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionMipSetToolPointer: public FunctionRunnable
{
public:
   typedef ::impl::MipStorage Pointee;

public:
   FunctionMipSetToolPointer( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::TYPE || v2.type != RuntimeValue::TYPE )
      {
         throw RuntimeException( "wrong arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionMipToolPointerConstructor::Pointee* tool = reinterpret_cast<FunctionMipToolPointerConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // we first need to remove all the tools of this category to ensure there is no inconsistencies...
      std::set<MipToolPointer*> tools = pointee->mip.getTools<MipToolPointer>();
      for ( std::set<MipToolPointer*>::iterator it = tools.begin(); it != tools.end(); ++it )
      {
         pointee->mip.disconnect( *it );
      }

      // finally add the tool
      pointee->mip.connect( tool );
      pointee->toolPointer = v2;

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionMipSetToolAnnotations: public FunctionRunnable
{
public:
   typedef ::impl::MipStorage Pointee;

public:
   FunctionMipSetToolAnnotations( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::TYPE || v2.type != RuntimeValue::TYPE )
      {
         throw RuntimeException( "wrong arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionToolAnnotationsConstructor::Pointee* tool = reinterpret_cast<FunctionToolAnnotationsConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // we first need to remove all the tools of this category to ensure there is no inconsistencies...
      std::set<MipToolAnnotations*> tools = pointee->mip.getTools<MipToolAnnotations>();
      for ( std::set<MipToolAnnotations*>::iterator it = tools.begin(); it != tools.end(); ++it )
      {
         pointee->mip.disconnect( *it );
      }

      // finally add the tool
      pointee->mip.connect( &tool->toolMip );
      pointee->toolAnnotations = v2;

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

#endif