#ifndef CORE_MVV_MOUSE_H_
# define CORE_MVV_MOUSE_H_

# include "core.h"

# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include <mvvPlatform/mouse-pointer.h>
# include <mvvPlatform/context-global.h>

using namespace mvv::parser;
using namespace mvv;

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

      createVector2i( mousePosition, (int)event.mousePosition[ 0 ] - (int)windowOrigin[ 0 ], (int)event.mousePosition[ 1 ] - (int)windowOrigin[ 1 ] );
      createVector3f( segmentOrigin, s.getOrigin()[ 0 ], s.getOrigin()[ 1 ], s.getOrigin()[ 2 ] );
      createVector2f( segmentSpacing, s.getSpacing()[ 0 ], s.getSpacing()[ 1 ] );
      createVector2i( segmentSize, s.size()[ 0 ], s.size()[ 1 ] );

      // run the callback
      std::vector<RuntimeValue> values( 4 );
      values[ 0 ] = mousePosition;
      values[ 1 ] = segmentOrigin;
      values[ 2 ] = segmentSpacing;
      values[ 3 ] = segmentSize;

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

      global->onSegmentLeftMouseClick = RefcountedTyped<MouseSegmentCallback>( new MouseSegmentCallbackImpl( unref( *args[ 0 ] ), _e ) );
      
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

      global->onSegmentRightMouseClick = RefcountedTyped<MouseSegmentCallback>( new MouseSegmentCallbackImpl( unref( *args[ 0 ] ), _e ) );
      
      RuntimeValue res( RuntimeValue::EMPTY );
      return res;
   }

private:
   CompilerFrontEnd&    _e;
   Context&             _context;
};


#endif