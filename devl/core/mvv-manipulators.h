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

class FunctionManipulatorCuboidConstructor: public FunctionRunnable
{
public:
   typedef RefcountedTyped<ToolManipulatorsInterface> Pointee;

public:
   FunctionManipulatorCuboidConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );

      nll::core::vector3f min;
      getVector3fValues( v2, min );

      nll::core::vector3f max;
      getVector3fValues( v3, max );

      nll::core::vector3i color;
      getVector3iValues( v4, color );
      if ( color[ 0 ] < 0 || color[ 1 ] < 0 || color[ 2 ] < 0 ||
           color[ 0 ] > 255 || color[ 1 ] > 255 || color[ 2 ] > 255 )
      {
         throw std::runtime_error( "color is out of range [0..255]" );
      }
      
      // construct the type
      Pointee* pointee = new Pointee( new ToolManipulatorsCuboid( min, max, nll::core::vector3uc( (ui8)color[ 0 ], (ui8)color[ 1 ], (ui8)color[ 2 ] ) ) );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;
      return v1;  // return the original object!
   }
};

class FunctionManipulatorCuboidDestructor: public FunctionRunnable
{
public:
   typedef FunctionManipulatorCuboidConstructor Pointee;

public:
   FunctionManipulatorCuboidDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionToolManipulatorPointGetPosition : public FunctionRunnable
{
public:
   typedef FunctionManipulatorPointConstructor::Pointee Pointee;

public:
   FunctionToolManipulatorPointGetPosition( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
            
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      ToolManipulatorsPoint* pp = dynamic_cast<ToolManipulatorsPoint*>( &( **p ) );
      if ( !pp )
         throw std::runtime_error( "ToolManipulatorsPoint is not a ToolManipulatorsPoint!" );
      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, pp->getPosition()[ 0 ], pp->getPosition()[ 1 ], pp->getPosition()[ 2 ] );
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
      p->segmentManipulators.notify();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionManipulatorPointerConstructor: public FunctionRunnable
{
public:
   typedef RefcountedTyped<ToolManipulatorsInterface> Pointee;

public:
   FunctionManipulatorPointerConstructor( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
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


      
      // construct the type
      Pointee* pointee = new Pointee( new ToolManipulatorsPointer( global->commonFont ) );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;
      return v1;  // return the original object!
   }

private:
   Context&    _context;
};

class FunctionManipulatorPointerDestructor: public FunctionRunnable
{
public:
   typedef FunctionManipulatorPointerConstructor Pointee;

public:
   FunctionManipulatorPointerDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionToolManipulatorsAddCuboid : public FunctionRunnable
{
public:
   typedef FunctionToolManipulatorsConstructor::Pointee Pointee;
   typedef FunctionManipulatorCuboidConstructor::Pointee PointeePoint;

public:
   FunctionToolManipulatorsAddCuboid( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      p->segmentManipulators.notify();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionToolManipulatorsAddPointer : public FunctionRunnable
{
public:
   typedef FunctionToolManipulatorsConstructor::Pointee Pointee;
   typedef FunctionManipulatorPointerConstructor::Pointee PointeePoint;

public:
   FunctionToolManipulatorsAddPointer( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      p->segmentManipulators.notify();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionToolManipulatorsErasePointer : public FunctionRunnable
{
public:
   typedef FunctionToolManipulatorsConstructor::Pointee Pointee;
   typedef FunctionManipulatorPointerConstructor::Pointee PointeePoint;

public:
   FunctionToolManipulatorsErasePointer( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      p->segmentManipulators.erase( *point );
      p->segmentManipulators.notify();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionManipulatorSetPosition : public FunctionRunnable
{
public:
   typedef FunctionToolManipulatorsConstructor::Pointee Pointee;

public:
   FunctionManipulatorSetPosition( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      nll::core::vector3f pos;
      getVector3fValues( v2, pos );

      ToolManipulatorsPointer* pp = p->segmentManipulators.get<ToolManipulatorsPointer>();
      if (!pp )
      {
         // there is no manipulator attached, so just set the segment position
         std::set<Segment*> segments = p->segmentManipulators.getConnectedSegments();
         for ( std::set<Segment*>::iterator it = segments.begin(); it != segments.end(); ++it )
         {
            (**it).position.setValue( pos );
         }
      } else {
         //if ( !pp )
         //   throw std::runtime_error( "ToolManipulatorsPointer manipulator found in the container" );
         pp->setPosition( pos );
         p->segmentManipulators.notify();
      }
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionManipulatorGetPosition : public FunctionRunnable
{
public:
   typedef FunctionToolManipulatorsConstructor::Pointee Pointee;

public:
   FunctionManipulatorGetPosition( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
            
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      

      ToolManipulatorsPointer* pp = p->segmentManipulators.get<ToolManipulatorsPointer>();
      if ( !pp )
         throw std::runtime_error( "ToolManipulatorsPointer manipulator found in the container" );

      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, pp->getPosition()[ 0 ], pp->getPosition()[ 1 ], pp->getPosition()[ 2 ] );
      return rt;
   }
};

class FunctionToolManipulatorPointSetPosition : public FunctionRunnable
{
public:
   typedef FunctionManipulatorPointConstructor::Pointee Pointee;
   typedef FunctionToolManipulatorsConstructor::Pointee PointeeManip;

public:
   FunctionToolManipulatorPointSetPosition( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 3 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
            
      nll::core::vector3f pos;
      getVector3fValues( v2, pos );

      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      PointeeManip* manip = reinterpret_cast<PointeeManip*>( (*v3.vals)[ 0 ].ref );

      ToolManipulatorsPoint* pp = dynamic_cast<ToolManipulatorsPoint*>( &( **p ) );
      if ( !pp )
         throw std::runtime_error( "ToolManipulatorsPoint is not a ToolManipulatorsPoint!" );
      pp->setPosition( pos );
      manip->segmentManipulators.notify();   // force the display
      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, pp->getPosition()[ 0 ], pp->getPosition()[ 1 ], pp->getPosition()[ 2 ] );
      return rt;
   }
};


#endif