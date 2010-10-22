#ifndef CORE_MVV_LAYOUT_H_
# define CORE_MVV_LAYOUT_H_

# include "core.h"
# include "mvv-segment.h"
# include "mvv-mip-tools.h"
# include "mvv-image.h"
# include <mvvScript/compiler.h>

# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include <mvvPlatform/layout-pane.h>
# include <mvvPlatform/layout-pane-image.h>
# include <mvvPlatform/layout-pane-cmdl.h>
# include <mvvMprPlugin/layout-segment.h>
# include <mvvMprPlugin/layout-mip.h>

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

namespace impl
{
   struct LayoutStorage
   {
      LayoutStorage( Pane* p ) : pane( RefcountedTyped<Pane>( p ) )
      {
      }

      RefcountedTyped<Pane> pane;
      RuntimeValue          segment;
      RuntimeValue          mip;
      RuntimeValue          leftLayout;
      RuntimeValue          rightLayout;
      RuntimeValue          imageHolder;
   };
}

// import Layout( Vertical   tag, Layout left, Layout right, float ratio );
class FunctionLayoutConstructorVertical: public FunctionRunnable
{
public:
   typedef ::impl::LayoutStorage Pointee;

public:
   FunctionLayoutConstructorVertical( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
           v5.type != RuntimeValue::CMP_FLOAT )
      {
         throw std::runtime_error( "wrong arguments: expecting Vertical, Layout, Layout, float" );
      }

      // read the storage value
      assert( (*v3.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* left = reinterpret_cast<Pointee*>( (*v3.vals)[ 0 ].ref );
      assert( (*v4.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* right = reinterpret_cast<Pointee*>( (*v4.vals)[ 0 ].ref );

      // construct the type
      const float ratio = v5.floatval;
      if ( ratio < 0 || ratio > 1 )
      {
         throw std::runtime_error( "ratio must be in range [0..1]" );
      }
      PaneListVertical* pane = new PaneListVertical( nll::core::vector2ui(), nll::core::vector2ui() );   // undefined size
      pane->addChild( left->pane, ratio );
      pane->addChild( right->pane, 1 - ratio );

      // fill the storage
      Pointee* pointee = new Pointee( pane );
      pointee->leftLayout = v3;
      pointee->rightLayout = v4;
      
      // update the object
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionLayoutConstructorHorizontal: public FunctionRunnable
{
public:
   typedef ::impl::LayoutStorage Pointee;

public:
   FunctionLayoutConstructorHorizontal( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
           v5.type != RuntimeValue::CMP_FLOAT )
      {
         throw std::runtime_error( "wrong arguments: expecting Vertical, Layout, Layout, float" );
      }

      // read the storage value
      assert( (*v3.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* left = reinterpret_cast<Pointee*>( (*v3.vals)[ 0 ].ref );
      assert( (*v4.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* right = reinterpret_cast<Pointee*>( (*v4.vals)[ 0 ].ref );

      // construct the type
      const float ratio = v5.floatval;
      if ( ratio < 0 || ratio > 1 )
      {
         throw std::runtime_error( "ratio must be in range [0..1]" );
      }
      PaneListHorizontal* pane = new PaneListHorizontal( nll::core::vector2ui(), nll::core::vector2ui() );   // undefined size
      pane->addChild( left->pane, ratio );
      pane->addChild( right->pane, 1 - ratio );

      // fill the storage
      Pointee* pointee = new Pointee( pane );
      pointee->leftLayout = v3;
      pointee->rightLayout = v4;
      
      // update the object
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionLayoutDestructor: public FunctionRunnable
{
public:
   typedef ::impl::LayoutStorage Pointee;

public:
   FunctionLayoutDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionLayoutConstructorSegment: public FunctionRunnable
{
public:
   typedef ::impl::LayoutStorage Pointee;

public:
   FunctionLayoutConstructorSegment( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
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

      if ( v2.type != RuntimeValue::TYPE )
      {
         throw std::runtime_error( "wrong arguments: expecting Segment" );
      }

      // read the storage value
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionSegmentConstructor::Pointee* segment = reinterpret_cast<FunctionSegmentConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // it is safe not to have a real refcount here, as we are saving the layout (which is refcounted!)
      Pane* pane = new PaneSegment( nll::core::vector2ui( 0, 0 ),
                                    nll::core::vector2ui( 0, 0 ),
                                    RefcountedTyped<Segment>( &segment->segment, false ),
                                    _context );

      // fill the storage
      Pointee* pointee = new Pointee( pane );
      pointee->segment = v2;

      // update the object
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
   Context&    _context;
};

class FunctionLayoutConstructorMip: public FunctionRunnable
{
public:
   typedef ::impl::LayoutStorage Pointee;

public:
   FunctionLayoutConstructorMip( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v2.type != RuntimeValue::TYPE )
      {
         throw std::runtime_error( "wrong arguments: expecting Segment" );
      }

      // read the storage value
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionMipConstructor::Pointee* mip = reinterpret_cast<FunctionMipConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // it is safe not to have a real refcount here, as we are saving the layout (which is refcounted!)
      Pane* pane = new PaneMip( nll::core::vector2ui( 0, 0 ),
                                nll::core::vector2ui( 0, 0 ),
                                RefcountedTyped<Mip>( &mip->mip, false ) );

      // fill the storage
      Pointee* pointee = new Pointee( pane );
      pointee->mip = v2;

      // update the object
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

/*
class FunctionLayoutConstructorConsole: public FunctionRunnable
{
public:
   typedef LayoutCommandLine Pointee;

public:
   FunctionLayoutConstructorConsole( const AstDeclFun* fun, mvv::platform::Context& context, mvv::parser::CompilerFrontEnd& compiler ) : FunctionRunnable( fun ), _context( context ), _compiler( compiler )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      // fill the storage
      Pointee* pointee = new Pointee( nll::core::vector2ui( 0, 0 ), nll::core::vector2ui( 0, 0 ), mvv::platform::RefcountedTyped<mvv::platform::Font>( &global->commonFont, false ), _compiler );
      
      // update the object
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
   mvv::platform::Context&          _context;
   mvv::parser::CompilerFrontEnd&   _compiler;
};

class FunctionLayoutConsoleDestructor: public FunctionRunnable
{
public:
   typedef LayoutCommandLine Pointee;

public:
   FunctionLayoutConsoleDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
*/

class FunctionLayoutConstructorLayout: public FunctionRunnable
{
public:
   typedef ::impl::LayoutStorage Pointee;

public:
   FunctionLayoutConstructorLayout( const AstDeclFun* fun, mvv::platform::Context& context, mvv::parser::CompilerFrontEnd& compiler ) : FunctionRunnable( fun ), _context( context ), _compiler( compiler )
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

      if ( v2.type != RuntimeValue::TYPE )
      {
         throw std::runtime_error( "wrong arguments: expecting Console" );
      }

      // it is safe not to have a real refcount here, as we are saving the layout (which is refcounted!)
      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      // fill the storage
      Pane* pane = new LayoutCommandLine( nll::core::vector2ui( 0, 0 ), nll::core::vector2ui( 0, 0 ), mvv::platform::RefcountedTyped<mvv::platform::Font>( &global->commonFont, false ), _compiler, *global->completion );

      // fill the storage
      Pointee* pointee = new Pointee( pane );

      // update the object
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
   mvv::platform::Context&          _context;
   mvv::parser::CompilerFrontEnd&   _compiler;
};

class FunctionLayoutConstructorImage: public FunctionRunnable
{
public:
   typedef ::impl::LayoutStorage Pointee;

public:
   FunctionLayoutConstructorImage( const AstDeclFun* fun, mvv::platform::Context& context ) : FunctionRunnable( fun ), _context( context )
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
      FunctionImageHolderConstructor::Pointee* image = reinterpret_cast<FunctionImageHolderConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // find global context
      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      // it is safe not to have a real refcount here, as we are saving the layout (which is refcounted!)
      PaneImage* pane = new PaneImage( nll::core::vector2ui( 0, 0 ),
                                       nll::core::vector2ui( 0, 0 ),
                                       nll::core::vector3uc(),
                                       global->engineHandler );

      // connect the image
      pane->image = *image;


      // fill the storage
      Pointee* pointee = new Pointee( pane );
      pointee->imageHolder = v2;

      // update the object
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
   mvv::platform::Context&          _context;
};



#endif