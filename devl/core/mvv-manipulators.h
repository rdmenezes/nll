#ifndef CORE_MVV_SEGMENT_TOOL_MANIPULATORS_H_
# define CORE_MVV_SEGMENT_TOOL_MANIPULATORS_H_

# include "core.h"

# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include <mvvPlatform/context-global.h>

# include <mvvMprPlugin/context-segments.h>
# include <mvvMprPlugin/segment-tool-manipulators.h>

using namespace mvv::parser;
using namespace mvv;

typedef int ToolManipulatorPoint;

class FunctionManipulatorPointConstructor: public FunctionRunnable
{
public:
   typedef RefcountedTyped<ToolManipulatorsInterface> Pointee;

public:
   FunctionManipulatorPointConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      nll::core::vector3f position;
      getVector3fValues( v2, position );
      
      // construct the type
      Pointee* pointee = new Pointee( new ToolManipulatorsPoint( position, nll::core::vector3uc( 255, 255, 255 ) ) );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;
      return v1;  // return the original object!
   }
};

class FunctionManipulatorPointDestructor: public FunctionRunnable
{
public:
   typedef FunctionManipulatorPointConstructor Pointee;

public:
   FunctionManipulatorPointDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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


struct ToolManipulatorStorage
{
   SegmentToolManipulators    segmentManipulators;
};

class FunctionToolManipulatorsConstructor: public FunctionRunnable
{
public:
   typedef ToolManipulatorStorage Pointee;

public:
   FunctionToolManipulatorsConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      
      // construct the type
      Pointee* pointee = new Pointee();
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;
      return v1;  // return the original object!
   }
};

class FunctionToolManipulatorsDestructor: public FunctionRunnable
{
public:
   typedef FunctionToolManipulatorsConstructor::Pointee Pointee;

public:
   FunctionToolManipulatorsDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionToolManipulatorsAddPoint : public FunctionRunnable
{
public:
   typedef FunctionToolManipulatorsConstructor::Pointee Pointee;
   typedef FunctionManipulatorPointConstructor::Pointee PointeePoint;

public:
   FunctionToolManipulatorsAddPoint( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
            
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      PointeePoint* point = reinterpret_cast<PointeePoint*>( (*v2.vals)[ 0 ].ref );

      p->segmentManipulators.add( *point );
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};


#endif