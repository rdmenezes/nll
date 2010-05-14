#ifndef CORE_MVV_LAYOUT_H_
# define CORE_MVV_LAYOUT_H_

# include "core.h"

# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include <mvvPlatform/layout-pane.h>
# include <mvvMprPlugin/layout-segment.h>

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
      RuntimeValue          leftLayout;
      RuntimeValue          rightLayout;
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
           v5.type != RuntimeValue::FLOAT )
      {
         throw RuntimeException( "wrong arguments: expecting Vertical, Layout, Layout, float" );
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
         throw RuntimeException( "ratio must be in range [0..1]" );
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
           v5.type != RuntimeValue::FLOAT )
      {
         throw RuntimeException( "wrong arguments: expecting Vertical, Layout, Layout, float" );
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
         throw RuntimeException( "ratio must be in range [0..1]" );
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

class FunctionLayoutConstructorSegment: public FunctionRunnable
{
public:
   typedef ::impl::LayoutStorage Pointee;

public:
   FunctionLayoutConstructorSegment( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v2.type != RuntimeValue::TYPE )
      {
         throw RuntimeException( "wrong arguments: expecting Segment" );
      }

      // read the storage value
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      FunctionSegmentConstructor::Pointee* segment = reinterpret_cast<FunctionSegmentConstructor::Pointee*>( (*v2.vals)[ 0 ].ref );

      // it is safe not to have a real refcount here, as we are saving the layout (which is refcounted!)
      Pane* pane = new PaneSegment( nll::core::vector2ui( 0, 0 ),
                                    nll::core::vector2ui( 0, 0 ),
                                    RefcountedTyped<Segment>( &segment->segment, false ) );

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
};

#endif