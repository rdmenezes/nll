#ifndef CORE_MVV_SEGMENT_TOOL_H_
# define CORE_MVV_SEGMENT_TOOL_H_

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
# include <mvvMprPlugin/mip-tool-annotations.h>

using namespace mvv::parser;
using namespace mvv;

class FunctionSegmentToolCenteringConstructor: public FunctionRunnable
{
public:
   typedef platform::SegmentToolAutocenter Pointee;

public:
   FunctionSegmentToolCenteringConstructor( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw RuntimeException( "mvv global context has not been initialized" );
      }

      // construct the type
      Pointee* pointee = new Pointee( global->engineHandler );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
   Context& _context;
};

class FunctionSegmentToolCenteringDestructor: public FunctionRunnable
{
public:
   typedef platform::SegmentToolAutocenter Pointee;

public:
   FunctionSegmentToolCenteringDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionSegmentToolPointerConstructor: public FunctionRunnable
{
public:
   typedef platform::SegmentToolPointer Pointee;

public:
   FunctionSegmentToolPointerConstructor( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
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
      Pointee* pointee = new Pointee( global->commonFont, v2.intval,  global->engineHandler );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;
      return v1;  // return the original object!
   }

private:
   Context& _context;
};

class FunctionSegmentToolPointerDestructor: public FunctionRunnable
{
public:
   typedef platform::SegmentToolPointer Pointee;

public:
   FunctionSegmentToolPointerDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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


class FunctionSegmentToolPointerSetPosition: public FunctionRunnable
{
public:
   typedef platform::SegmentToolPointer Pointee;

public:
   FunctionSegmentToolPointerSetPosition( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      if ( v2.type != RuntimeValue::CMP_FLOAT || v3.type != RuntimeValue::CMP_FLOAT || v3.type != RuntimeValue::CMP_FLOAT )
      {
         throw RuntimeException( "wrong argument type: expecting 3 floats" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      pointee->setPosition( nll::core::vector3f( v2.floatval, v3.floatval, v4.floatval ) );
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

namespace impl
{
   struct ToolAnnotationsStorage
   {
      typedef std::map<ui32, RefcountedTyped<Annotation> > AnnotationDictionary;
      ToolAnnotationsStorage( EngineHandler& handler ) : tool( annotations, handler ), toolMip( annotations, handler )
      {}

      ResourceAnnotations     annotations;
      SegmentToolAnnotations  tool;
      MipToolAnnotations      toolMip;
      AnnotationDictionary    dictionary;    // link an ID to a specific annotation
   };
}

class FunctionToolAnnotationsConstructor: public FunctionRunnable
{
public:
   typedef ::impl::ToolAnnotationsStorage Pointee;

public:
   FunctionToolAnnotationsConstructor( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw RuntimeException( "mvv global context has not been initialized" );
      }

      // construct the type
      Pointee* pointee = new Pointee( global->engineHandler );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
   Context& _context;
};

class FunctionToolAnnotationsDestructor: public FunctionRunnable
{
public:
   typedef ::impl::ToolAnnotationsStorage Pointee;

public:
   FunctionToolAnnotationsDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionToolAnnotationsAdd: public FunctionRunnable
{
public:
   typedef ::impl::ToolAnnotationsStorage Pointee;

public:
   FunctionToolAnnotationsAdd( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      if ( v3.type != RuntimeValue::STRING || (*v2.vals).size() != 1 || (*(*v2.vals)[ 0 ].vals).size() != 3
                                           || (*v4.vals).size() != 1 || (*(*v4.vals)[ 0 ].vals).size() != 3 )
      {
         throw RuntimeException( "wrong argument type: expecting Vector3f, string and Vector3f" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw RuntimeException( "mvv global context has not been initialized" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      static ui32 annotationId = 0;
      ++annotationId;

      // create the annotation
      nll::core::vector3f  position( (*(*v2.vals)[ 0 ].vals)[ 0 ].floatval,
                                     (*(*v2.vals)[ 0 ].vals)[ 1 ].floatval,
                                     (*(*v2.vals)[ 0 ].vals)[ 2 ].floatval );
      nll::core::vector3uc color( static_cast<ui8>( (*(*v4.vals)[ 0 ].vals)[ 0 ].intval ),
                                  static_cast<ui8>( (*(*v4.vals)[ 0 ].vals)[ 1 ].intval ),
                                  static_cast<ui8>( (*(*v4.vals)[ 0 ].vals)[ 2 ].intval ) );
      RefcountedTyped<Annotation> annotation( new AnnotationPoint( position, v3.stringval, global->commonFont, 12, color ) );

      // register it
      pointee->annotations.insert( annotation );
      pointee->dictionary[ annotationId ] = annotation;

      // return the annotation ID      
      RuntimeValue rt( RuntimeValue::TYPE );
      rt.vals = RuntimeValue::RefcountedValues( 0, 0, new RuntimeValues( 1 ) ); // no associated destructor
      (*rt.vals)[ 0 ].setType( RuntimeValue::CMP_INT );
      (*rt.vals)[ 0 ].intval = annotationId;
      return rt;
   }

private:
   Context&    _context;
};

class FunctionToolAnnotationsErase: public FunctionRunnable
{
public:
   typedef ::impl::ToolAnnotationsStorage Pointee;

public:
   FunctionToolAnnotationsErase( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( (*v2.vals).size() != 1 && (*v2.vals)[ 0 ].type != RuntimeValue::CMP_INT )
      {
         throw RuntimeException( "wrong argument type: expecting AnnotationID" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      Pointee::AnnotationDictionary::iterator it = pointee->dictionary.find( (*v2.vals)[ 0 ].intval );
      if ( it == pointee->dictionary.end() )
      {
         throw RuntimeException( "the AnnotationID doesn't belong to this tool" );
      }
      pointee->annotations.erase( it->second );
      pointee->dictionary.erase( it );

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};



class FunctionSegmentToolCameraConstructor: public FunctionRunnable
{
public:
   typedef platform::SegmentToolCamera Pointee;

public:
   FunctionSegmentToolCameraConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
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

class FunctionSegmentToolCameraDestructor: public FunctionRunnable
{
public:
   typedef platform::SegmentToolCamera Pointee;

public:
   FunctionSegmentToolCameraDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
#endif