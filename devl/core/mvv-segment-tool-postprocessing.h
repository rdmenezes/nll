#ifndef CORE_MVV_SEGMENT_TOOL_POSTPROCESSING_H_
# define CORE_MVV_SEGMENT_TOOL_POSTPROCESSING_H_

# include "core.h"

# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

# include <mvvPlatform/context-global.h>

# include <mvvMprPlugin/context-segments.h>
# include <mvvMprPlugin/segment-tool-pointer.h>
# include <mvvMprPlugin/segment-tool-camera.h>
# include <mvvMprPlugin/segment-tool-annotations.h>
# include <mvvMprPlugin/segment-tool-autocenter.h>
# include <mvvMprPlugin/segment-tool-postprocessing.h>
# include <mvvMprPlugin/annotation-point.h>
# include <mvvMprPlugin/annotation-line.h>
# include <mvvMprPlugin/annotation-colors.h>
# include <mvvMprPlugin/mip-tool-annotations.h>

using namespace mvv::parser;
using namespace mvv;

namespace impl
{
   class PostProcessing : public ToolPostProcessingInterface
   {
   public:
      PostProcessing( RuntimeValue fptr, CompilerFrontEnd& e ) : _fptr( fptr ), _e( e )
      {
      }

      virtual void run( Sliceuc& s )
      {
         // we need to construct the arguments and call the function pointer

         // create the arguments
         mvv::platform::ResourceImageuc im;
         im.setValue( s.getStorage() );

         RuntimeValue image( RuntimeValue::TYPE );
         RuntimeValues* vals = new RuntimeValues( 1 );
         (*vals)[ 0 ] = RuntimeValue( RuntimeValue::PTR );
         RuntimeValue* ptr = reinterpret_cast<RuntimeValue*>( &im );
         (*vals)[ 0 ].ref = ptr;
         image.vals = RuntimeValue::RefcountedValues( 0, 0, vals );

         RuntimeValue spacing;
         createVector2f( spacing, s.getSpacing()[ 0 ], s.getSpacing()[ 1 ] );

         RuntimeValue origin;
         createVector3f( origin, s.getOrigin()[ 0 ], s.getOrigin()[ 1 ], s.getOrigin()[ 2 ] );

         // call the callback
         std::vector<RuntimeValue> values( 3 );
         values[ 0 ] = image;
         values[ 1 ] = spacing;
         values[ 2 ] = origin;

         try
         {
            _e.evaluateCallback( _fptr, values );
         } catch ( std::runtime_error e )
         {
            std::cout << "error in postprocessing callback=" << e.what() << std::endl;
         }
      }

   private:
      // copy disabled
      PostProcessing& operator=( const PostProcessing& );
      PostProcessing( const PostProcessing& );

   private:
      RuntimeValue         _fptr;
      CompilerFrontEnd&    _e;
   };
}

namespace impl
{
   struct PostprocessingStorage
   {
      typedef SegmentToolPostProcessing   PointeeSegment;
      typedef MipToolPostProcessing       PointeeMip;

      RefcountedTyped<PointeeSegment>     postprocessingSegment;
      RefcountedTyped<PointeeMip>         postprocessingMip;

      PostprocessingStorage( RefcountedTyped<PointeeSegment> s, RefcountedTyped<PointeeMip> m ) : postprocessingSegment( s ), postprocessingMip( m )
      {}
   };
}

class FunctionToolPostprocessingConstructor: public FunctionRunnable
{
public:
   typedef ::impl::PostprocessingStorage Pointee;
   typedef Pointee::PointeeSegment   PointeeSegment;
   typedef Pointee::PointeeMip       PointeeMip;

public:
   FunctionToolPostprocessingConstructor( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
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


      // construct the type
      ::impl::PostProcessing* postProcessing = new ::impl::PostProcessing( v2, _e );
      RefcountedTyped<ToolPostProcessingInterface> ref( postProcessing );
      PointeeSegment* tool1 = new PointeeSegment( ref );
      PointeeMip* tool2 = new PointeeMip( ref );

      // create the storage
      Pointee* tool = new Pointee( RefcountedTyped<PointeeSegment>( tool1 ), RefcountedTyped<PointeeMip>( tool2 ) );

      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( tool ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
    CompilerFrontEnd&   _e;
};

/*
class FunctionToolPostprocessingConstructor: public FunctionRunnable
{
public:
   typedef SegmentToolPostProcessing Pointee;

public:
   FunctionToolPostprocessingConstructor( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
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


      // construct the type
      ::impl::PostProcessing* postProcessing = new ::impl::PostProcessing( v2, _e );
      Pointee* tool = new Pointee( RefcountedTyped<ToolPostProcessingInterface>( postProcessing ) );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( tool ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }

private:
    CompilerFrontEnd&   _e;
};*/

class FunctionToolPostprocessingDestructor: public FunctionRunnable
{
public:
   typedef FunctionToolPostprocessingConstructor::Pointee Pointee;

public:
   FunctionToolPostprocessingDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionSegmentSetPostprocessing: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;
   typedef FunctionToolPostprocessingConstructor::Pointee PointeeTool;

public:
   FunctionSegmentSetPostprocessing( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
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
      PointeeTool* tool = reinterpret_cast<PointeeTool*>( (*v2.vals)[ 0 ].ref );

      // disconnect previous tools
      std::set<SegmentToolPostProcessing*> tools = pointee->segment.getTools<SegmentToolPostProcessing>();
      for ( std::set<SegmentToolPostProcessing*>::iterator it = tools.begin(); it != tools.end(); ++it )
      {
         pointee->segment.disconnect( *it );
      }

      // connect the current tool
      pointee->segment.connect( &(*tool->postprocessingSegment) );
      pointee->toolPostprocessing = v2;
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }

private:
   CompilerFrontEnd&             _e;
};

#endif