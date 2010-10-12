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
   class PostProcessing : public SegmentToolPostProcessingInterface
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
         createFields( image, 1, RuntimeValue::PTR );
         (*image.vals)[ 0 ].ref = reinterpret_cast<RuntimeValue*>( &im );

         RuntimeValue spacing;
         createVector2f( spacing, s.getSpacing()[ 0 ], s.getSpacing()[ 1 ] );

         RuntimeValue origin;
         createVector3f( origin, s.getOrigin()[ 0 ], s.getOrigin()[ 1 ], s.getOrigin()[ 2 ] );

         // call the callback
         std::vector<RuntimeValue> values( 3 );
         values[ 0 ] = image;
         values[ 1 ] = spacing;
         values[ 2 ] = origin;

         _e.evaluateCallback( _fptr, values );
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

class FunctionSegmentSetPostprocessing: public FunctionRunnable
{
public:
   typedef ::impl::SegmentStorage Pointee;

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

      if ( v1.type != RuntimeValue::TYPE || v2.type != RuntimeValue::FUN_PTR )
      {
         throw std::runtime_error( "wrong arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // disconnect previous tools
      std::set<SegmentToolPostProcessing*> tools = pointee->segment.getTools<SegmentToolPostProcessing>();
      for ( std::set<SegmentToolPostProcessing*>::iterator it = tools.begin(); it != tools.end(); ++it )
      {
         pointee->segment.disconnect( *it );
      }

      // connect the current tool
      RefcountedTyped<SegmentToolPostProcessingInterface> postProcessing( new ::impl::PostProcessing( v2, _e ) );
      _tool = RefcountedTyped<SegmentTool>( new SegmentToolPostProcessing( postProcessing ) );
      pointee->segment.connect( &(*_tool) );
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }

private:
   RefcountedTyped<SegmentTool>  _tool;
   CompilerFrontEnd&             _e;
};

#endif