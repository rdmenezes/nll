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
# include <mvvMprPlugin/annotation-line.h>
# include <mvvMprPlugin/annotation-colors.h>
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v2.type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "expected int as argument" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
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

/*
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      if ( v2.type != RuntimeValue::CMP_FLOAT || v3.type != RuntimeValue::CMP_FLOAT || v3.type != RuntimeValue::CMP_FLOAT )
      {
         throw std::runtime_error( "wrong argument type: expecting 3 floats" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      pointee->setPosition( nll::core::vector3f( v2.floatval, v3.floatval, v4.floatval ) );
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionSegmentToolPointerSetPositionV: public FunctionRunnable
{
public:
   typedef platform::SegmentToolPointer Pointee;

public:
   FunctionSegmentToolPointerSetPositionV( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      nll::core::vector3f pos;
      mvv::parser::getVector3fValues( v2, pos );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      pointee->setPosition( pos );
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};
*/

class FunctionSegmentToolPointerGetPosition: public FunctionRunnable
{
public:
   typedef platform::SegmentToolPointer Pointee;

public:
   FunctionSegmentToolPointerGetPosition( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      RuntimeValue rt( RuntimeValue::EMPTY );
      nll::core::vector3f pos = pointee->getPosition();
      mvv::parser::createVector3f( rt, pos[ 0 ], pos[ 1 ], pos[ 2 ] );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
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

static ui32 annotationId = 0;

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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      if ( v3.type != RuntimeValue::STRING || (*v2.vals).size() != 1 || (*(*v2.vals)[ 0 ].vals).size() != 3
                                           || (*v4.vals).size() != 1 || (*(*v4.vals)[ 0 ].vals).size() != 3 )
      {
         throw std::runtime_error( "wrong argument type: expecting Vector3f, string and Vector3f" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

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

class FunctionToolAnnotationsAddLine: public FunctionRunnable
{
public:
   typedef ::impl::ToolAnnotationsStorage Pointee;

public:
   FunctionToolAnnotationsAddLine( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 6 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );

      RuntimeValue& v4 = unref( *args[ 3 ] );
      RuntimeValue& v5 = unref( *args[ 4 ] );
      RuntimeValue& v6 = unref( *args[ 5 ] );
      if ( v4.type != RuntimeValue::STRING || (*v2.vals).size() != 1 || (*(*v2.vals)[ 0 ].vals).size() != 3
                                           || (*v3.vals).size() != 1 || (*(*v3.vals)[ 0 ].vals).size() != 3
                                           || (*v5.vals).size() != 1 || (*(*v5.vals)[ 0 ].vals).size() != 3 )
      {
         throw std::runtime_error( "wrong argument type: expecting Vector3f, Vector3f, string, Vector3f, float" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      ++annotationId;

      // create the annotation
      nll::core::vector3f  position( (*(*v2.vals)[ 0 ].vals)[ 0 ].floatval,
                                     (*(*v2.vals)[ 0 ].vals)[ 1 ].floatval,
                                     (*(*v2.vals)[ 0 ].vals)[ 2 ].floatval );
      nll::core::vector3f  orientation( (*(*v3.vals)[ 0 ].vals)[ 0 ].floatval,
                                        (*(*v3.vals)[ 0 ].vals)[ 1 ].floatval,
                                        (*(*v3.vals)[ 0 ].vals)[ 2 ].floatval );
      nll::core::vector3uc color( static_cast<ui8>( (*(*v5.vals)[ 0 ].vals)[ 0 ].intval ),
                                  static_cast<ui8>( (*(*v5.vals)[ 0 ].vals)[ 1 ].intval ),
                                  static_cast<ui8>( (*(*v5.vals)[ 0 ].vals)[ 2 ].intval ) );
      RefcountedTyped<Annotation> annotation( new AnnotationLine( position, orientation, v6.floatval, v4.stringval, global->commonFont, 12, color ) );

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

class FunctionToolAnnotationsAddColors: public FunctionRunnable
{
public:
   typedef ::impl::ToolAnnotationsStorage Pointee;

public:
   FunctionToolAnnotationsAddColors( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 5 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      RuntimeValue& v5 = unref( *args[ 4 ] );

      
      if ( v4.type != RuntimeValue::CMP_INT || (*v2.vals).size() != 1 || (*(*v2.vals)[ 0 ].vals).size() != 3
                                            || (*v3.vals).size() != 1 || (*(*v3.vals)[ 0 ].vals).size() != 3
                                             )
      {
         throw std::runtime_error( "wrong argument type: expecting Vector3f, Vector3f, int, Vector3i[]" );
      }

      /*
      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }*/

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      ++annotationId;

      // create the annotation
      nll::core::vector3f  position( (*(*v2.vals)[ 0 ].vals)[ 0 ].floatval,
                                     (*(*v2.vals)[ 0 ].vals)[ 1 ].floatval,
                                     (*(*v2.vals)[ 0 ].vals)[ 2 ].floatval );
      nll::core::vector3f  orientation( (*(*v3.vals)[ 0 ].vals)[ 0 ].floatval,
                                        (*(*v3.vals)[ 0 ].vals)[ 1 ].floatval,
                                        (*(*v3.vals)[ 0 ].vals)[ 2 ].floatval );
      int size = v4.intval;
      if ( size <= 0 )
         throw std::runtime_error( "Annotation::add expects a width > 0" );

      std::vector<nll::core::vector3uc> colors( (*v5.vals).size() );
      for ( ui32 n = 0; n < colors.size(); ++n )
      {
         RuntimeValue& v = (*v5.vals)[ n ];
         if ( (*v.vals).size() != 1 || (*(*v.vals)[ 0 ].vals).size() != 3 )
            throw std::runtime_error( "Annotation::add colors should be an array of Vector3i" );

         nll::core::vector3uc& c = colors[ n ];
         c[ 0 ] = (*(*v.vals)[ 0 ].vals)[ 0 ].intval;
         c[ 1 ] = (*(*v.vals)[ 0 ].vals)[ 1 ].intval;
         c[ 2 ] = (*(*v.vals)[ 0 ].vals)[ 2 ].intval;
      }
      RefcountedTyped<Annotation> annotation( new AnnotationColors( position, orientation, size, colors ) );

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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( (*v2.vals).size() != 1 && (*v2.vals)[ 0 ].type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "wrong argument type: expecting AnnotationID" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      Pointee::AnnotationDictionary::iterator it = pointee->dictionary.find( (*v2.vals)[ 0 ].intval );
      if ( it == pointee->dictionary.end() )
      {
         throw std::runtime_error( "the AnnotationID doesn't belong to this tool" );
      }
      pointee->annotations.erase( it->second );
      pointee->dictionary.erase( it );


      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionToolAnnotationsClear: public FunctionRunnable
{
public:
   typedef ::impl::ToolAnnotationsStorage Pointee;

public:
   FunctionToolAnnotationsClear( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      pointee->annotations.clear();
      pointee->dictionary.clear();

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionToolAnnotationsSetPosition: public FunctionRunnable
{
public:
   typedef ::impl::ToolAnnotationsStorage Pointee;

public:
   FunctionToolAnnotationsSetPosition( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( (*v2.vals).size() != 1 && (*v2.vals)[ 0 ].type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "wrong argument type: expecting AnnotationID" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      Pointee::AnnotationDictionary::iterator it = pointee->dictionary.find( (*v2.vals)[ 0 ].intval );
      if ( it == pointee->dictionary.end() )
      {
         throw std::runtime_error( "the AnnotationID doesn't belong to this tool" );
      }
      nll::core::vector3f pos;
      mvv::parser::getVector3fValues( v3, pos );
      (*it->second).setPosition( pos );
      pointee->annotations.notify();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionToolAnnotationsGetPosition: public FunctionRunnable
{
public:
   typedef ::impl::ToolAnnotationsStorage Pointee;

public:
   FunctionToolAnnotationsGetPosition( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( (*v2.vals).size() != 1 && (*v2.vals)[ 0 ].type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "wrong argument type: expecting AnnotationID" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      Pointee::AnnotationDictionary::iterator it = pointee->dictionary.find( (*v2.vals)[ 0 ].intval );
      if ( it == pointee->dictionary.end() )
      {
         throw std::runtime_error( "the AnnotationID doesn't belong to this tool" );
      }

      nll::core::vector3f pos = (*it->second).getPosition();
      RuntimeValue rt( RuntimeValue::EMPTY );
      mvv::parser::createVector3f( rt, pos[ 0 ], pos[ 1 ], pos[ 2 ] );      
      return rt;
   }
};


class FunctionSegmentToolCameraSetPositionV: public FunctionRunnable
{
public:
   typedef platform::SegmentToolCamera Pointee;

public:
   FunctionSegmentToolCameraSetPositionV( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      nll::core::vector3f pos;
      mvv::parser::getVector3fValues( v2, pos );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      pointee->setPosition( pos );
      
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
#endif