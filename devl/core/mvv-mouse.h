#ifndef CORE_MVV_MOUSE_H_
# define CORE_MVV_MOUSE_H_

# include "core.h"

# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include <mvvPlatform/mouse-pointer.h>
# include <mvvPlatform/context-global.h>

using namespace mvv::parser;
using namespace mvv;

inline static nll::core::vector3f computeSliceOrigin( const Sliceuc& s )
{
   nll::core::vector3f originSlice( s.getOrigin()[ 0 ] - 0.5 * s.size()[ 0 ] * s.getSpacing()[ 0 ] * s.getAxisX()[ 0 ] - 0.5 * s.size()[ 1 ] * s.getSpacing()[ 1 ] * s.getAxisY()[ 0 ],
                                    s.getOrigin()[ 1 ] - 0.5 * s.size()[ 0 ] * s.getSpacing()[ 0 ] * s.getAxisX()[ 1 ] - 0.5 * s.size()[ 1 ] * s.getSpacing()[ 1 ] * s.getAxisY()[ 1 ],
                                    s.getOrigin()[ 2 ] - 0.5 * s.size()[ 0 ] * s.getSpacing()[ 0 ] * s.getAxisX()[ 2 ] - 0.5 * s.size()[ 1 ] * s.getSpacing()[ 1 ] * s.getAxisY()[ 2 ] );

   return originSlice;
}

class FunctionSetMousePointer: public FunctionRunnable
{
public:
   FunctionSetMousePointer( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      if ( v1.type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "arguments: expected 1 int" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      (*global->mousePointer).setMousePointer( (MousePointer::MouseGraphic)v1.intval );
      
      RuntimeValue res( RuntimeValue::EMPTY );
      return res;
   }

private:
   Context&    _context;
};

class FunctionGetMousePointer: public FunctionRunnable
{
public:
   FunctionGetMousePointer( const AstDeclFun* fun, Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 0 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }
      
      RuntimeValue res( RuntimeValue::CMP_INT );
      res.intval = (*global->mousePointer).getMousePointer();
      return res;
   }

private:
   Context&    _context;
};


class MouseSegmentCallbackImpl : platform::MouseSegmentCallback
{
public:
   MouseSegmentCallbackImpl( RuntimeValue fptr, CompilerFrontEnd& e ) : _fptr( fptr ), _e( e )
   {
   }

   virtual void run( const nll::imaging::Slice<nll::ui8>& s, const EventMouse& event, const nll::core::vector2ui& windowOrigin )
   {
      // create the callback arguments
      RuntimeValue mousePosition;
      RuntimeValue segmentOrigin;
      RuntimeValue segmentSpacing;
      RuntimeValue segmentSize;
      RuntimeValue axisx;
      RuntimeValue axisy;

      nll::core::vector3f blOrig = computeSliceOrigin( s );

      createVector2i( mousePosition, (int)event.mousePosition[ 0 ] - (int)windowOrigin[ 0 ], (int)event.mousePosition[ 1 ] - (int)windowOrigin[ 1 ] );
      createVector3f( segmentOrigin, blOrig[ 0 ], blOrig[ 1 ], blOrig[ 2 ] );
      createVector2f( segmentSpacing, s.getSpacing()[ 0 ], s.getSpacing()[ 1 ] );
      createVector2i( segmentSize, s.size()[ 0 ], s.size()[ 1 ] );
      createVector3f( axisx, s.getAxisX()[ 0 ], s.getAxisX()[ 1 ], s.getAxisX()[ 2 ] );
      createVector3f( axisy, s.getAxisY()[ 0 ], s.getAxisY()[ 1 ], s.getAxisY()[ 2 ] );


      // run the callback
      std::vector<RuntimeValue> values( 6 );
      values[ 0 ] = mousePosition;
      values[ 1 ] = segmentOrigin;
      values[ 2 ] = segmentSpacing;
      values[ 3 ] = segmentSize;
      values[ 4 ] = axisx;
      values[ 5 ] = axisy;

      try
      {
         _e.evaluateCallback( _fptr, values );
      } catch ( std::runtime_error e )
      {
         std::cout << "error in event mouse callback=" << e.what() << std::endl;
      }
   }

private:
   // copy disabled
   MouseSegmentCallbackImpl& operator=( const MouseSegmentCallbackImpl& );
   MouseSegmentCallbackImpl( const MouseSegmentCallbackImpl& );

private:
   RuntimeValue         _fptr;
   CompilerFrontEnd&    _e;
};

class FunctionSegmentMouseOnLeftClick: public FunctionRunnable
{
public:
   FunctionSegmentMouseOnLeftClick( const AstDeclFun* fun, CompilerFrontEnd& e, Context& context ) : FunctionRunnable( fun ), _e( e ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type == RuntimeValue::NIL )
      {
         global->onSegmentLeftMouseClick.unref();
      } else {
         global->onSegmentLeftMouseClick = RefcountedTyped<MouseSegmentCallback>( new MouseSegmentCallbackImpl( v1, _e ) );
      }
      
      RuntimeValue res( RuntimeValue::EMPTY );
      return res;
   }

private:
   CompilerFrontEnd&    _e;
   Context&             _context;
};

class FunctionSegmentMouseOnRightClick: public FunctionRunnable
{
public:
   FunctionSegmentMouseOnRightClick( const AstDeclFun* fun, CompilerFrontEnd& e, Context& context ) : FunctionRunnable( fun ), _e( e ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type == RuntimeValue::NIL )
      {
         global->onSegmentRightMouseClick.unref();
      } else {
         global->onSegmentRightMouseClick = RefcountedTyped<MouseSegmentCallback>( new MouseSegmentCallbackImpl( v1, _e ) );
      }
      
      RuntimeValue res( RuntimeValue::EMPTY );
      return res;
   }

private:
   CompilerFrontEnd&    _e;
   Context&             _context;
};


class FunctionSegmentMouseOnRightRelease: public FunctionRunnable
{
public:
   FunctionSegmentMouseOnRightRelease( const AstDeclFun* fun, CompilerFrontEnd& e, Context& context ) : FunctionRunnable( fun ), _e( e ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type == RuntimeValue::NIL )
      {
         global->onSegmentRightMouseRelease.unref();
      } else {
         global->onSegmentRightMouseRelease = RefcountedTyped<MouseSegmentCallback>( new MouseSegmentCallbackImpl( v1, _e ) );
      }
      
      RuntimeValue res( RuntimeValue::EMPTY );
      return res;
   }

private:
   CompilerFrontEnd&    _e;
   Context&             _context;
};

class FunctionSegmentMouseOnLeftRelease: public FunctionRunnable
{
public:
   FunctionSegmentMouseOnLeftRelease( const AstDeclFun* fun, CompilerFrontEnd& e, Context& context ) : FunctionRunnable( fun ), _e( e ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type == RuntimeValue::NIL )
      {
         global->onSegmentLeftMouseRelease.unref();
      } else {
         global->onSegmentLeftMouseRelease = RefcountedTyped<MouseSegmentCallback>( new MouseSegmentCallbackImpl( v1, _e ) );
      }
      
      RuntimeValue res( RuntimeValue::EMPTY );
      return res;
   }

private:
   CompilerFrontEnd&    _e;
   Context&             _context;
};

class FunctionSegmentMouseOnLeftPressed: public FunctionRunnable
{
public:
   FunctionSegmentMouseOnLeftPressed( const AstDeclFun* fun, CompilerFrontEnd& e, Context& context ) : FunctionRunnable( fun ), _e( e ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type == RuntimeValue::NIL )
      {
         global->onSegmentLeftMousePressed.unref();
      } else {
         global->onSegmentLeftMousePressed = RefcountedTyped<MouseSegmentCallback>( new MouseSegmentCallbackImpl( v1, _e ) );
      }
      
      RuntimeValue res( RuntimeValue::EMPTY );
      return res;
   }

private:
   CompilerFrontEnd&    _e;
   Context&             _context;
};

class FunctionSegmentMouseOnRightPressed: public FunctionRunnable
{
public:
   FunctionSegmentMouseOnRightPressed( const AstDeclFun* fun, CompilerFrontEnd& e, Context& context ) : FunctionRunnable( fun ), _e( e ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type == RuntimeValue::NIL )
      {
         global->onSegmentRightMousePressed.unref();
      } else {
         global->onSegmentRightMousePressed = RefcountedTyped<MouseSegmentCallback>( new MouseSegmentCallbackImpl( v1, _e ) );
      }
      
      RuntimeValue res( RuntimeValue::EMPTY );
      return res;
   }

private:
   CompilerFrontEnd&    _e;
   Context&             _context;
};


#endif