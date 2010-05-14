#ifndef MVV_INIT_H_
# define MVV_INIT_H_

# include <mvvPlatform/types.h>
# include <mvvPlatform/event-mouse.h>
# include <mvvPlatform/layout-pane.h>
# include <mvvPlatform/layout-widget.h>
# include <mvvPlatform/context.h>
# include <mvvPlatform/context-volumes.h>
# include <mvvPlatform/context-tools.h>
# include <mvvPlatform/context-global.h>
# include <mvvPlatform/engine-handler-impl.h>
# include <mvvMprPlugin/context-segments.h>
# include <mvvMprPlugin/layout-segment.h>
# include <mvvMprPlugin/segment-tool-pointer.h>
# include <mvvMprPlugin/segment-tool-camera.h>
# include <mvvMprPlugin/segment-tool-annotations.h>
# include <mvvMprPlugin/segment-tool-autocenter.h>
# include <mvvMprPlugin/layout-mip.h>
# include <mvvMprPlugin/annotation-point.h>
# include <mvvMprPlugin/mip-tool-pointer.h>
# include <mvvScript/compiler.h>
# include <core/mvv-layout.h>


using namespace mvv;
using namespace mvv::platform;
using namespace mvv::parser;

namespace mvv
{
   struct ApplicationVariables
   {
      unsigned int                        screenTextureId;
      Image                               screen;
      EventMouse                          mouseEvent;

      Context                             context;
      EngineHandlerImpl                   engineHandler;
      OrderManagerThreadPool              orderManager;

      RefcountedTyped<Font>               font;
      RefcountedTyped<Pane>               layout;

      CompilerFrontEnd                    compiler;

      ApplicationVariables() : screen( 1280, 1024, 3 ), orderManager( 6 )
      {  
         initFont();
         initContext();
         initScript();

         // event
         mouseEvent.isMouseRightButtonJustReleased = false;
         mouseEvent.isMouseLeftButtonJustReleased = false;
         mouseEvent.isMouseRightButtonJustPressed = false;
         mouseEvent.isMouseLeftButtonJustPressed = false;
         mouseEvent.isMouseRightButtonPressed = false;
         mouseEvent.isMouseLeftButtonPressed = false;
      }

   private:
      void initScript()
      {
         // set the extension, so we can access the context
         compiler.setContextExtension( RefcountedTyped<Context>( &context, false ) );

         //Error::ErrorType state = compiler.run( "D:/Devel/sandbox/mvvLauncher/script/single.ludo" );
         Error::ErrorType state = compiler.run( "include \"../../mvvLauncher/script/single\"" );
         if ( state != Error::SUCCESS )
         {
            throw std::exception( "initialization script couldn't be parsed successfully" );
         }

         // the only thing we absolutely need to find is a handle on the "layout", which MUST be defined in the script
         // the reference must be kept alive until the end of the program
         const RuntimeValue& layoutRef = compiler.getVariable( mvv::Symbol::create( "layout" ) );
         FunctionLayoutConstructorSegment::Pointee* pointee = reinterpret_cast<FunctionLayoutConstructorSegment::Pointee*>( (*layoutRef.vals)[ 0 ].ref );
         layout = pointee->pane;

         (*layout).setSize( nll::core::vector2ui( screen.sizex(), screen.sizey() ) );
         (*layout).updateLayout();
      }

      void initFont()
      {
         std::vector<char> chars;
         std::ifstream i( "../../nllTest/data/font/bitmapfont1_24.txt" );
         std::string line;
         while ( !i.eof() )
         {
            std::getline( i, line );
            ensure( line.size() < 2, "expected 1 character by line" );
            if ( line.size() == 1 )
            {
               chars.push_back( line[ 0 ] );
            } else chars.push_back( static_cast<char>( 0 ) );
         }
         ensure( chars.size() == 256, "we are expecting 256 chars, currently=" + nll::core::val2str( chars.size() ) );
         font = RefcountedTyped<Font>( new FontBitmapMatrix( "../../nllTest/data/font/bitmapfont1_24.bmp", nll::core::vector2ui( 16, 16 ), nll::core::vector2ui( 16, 16 ), chars ) );
      }

      void initContext()
      {
         ContextVolumes* ctxVolumes = new ContextVolumes();
         context.add( ctxVolumes );

         ContextTools* ctxTools = new ContextTools( ctxVolumes->volumes, engineHandler, orderManager, orderManager );
         context.add( ctxTools );

         ContextSegments* ctxSegments = new ContextSegments();
         context.add( ctxSegments );

         ContextGlobal* ctxGlobal = new ContextGlobal( engineHandler, orderManager, *font );
         context.add( ctxGlobal );
      }
   };
}

#endif


/*
namespace mvv
{
   struct ApplicationVariables
   {
      unsigned int                        screenTextureId;
      Image                               screen;
      EventMouse                          mouseEvent;
      RefcountedTyped<Pane>               layout;

      Context                             context;
      EngineHandlerImpl                   engineHandler;
      OrderManagerThreadPool              orderManager;

      RefcountedTyped<SegmentToolAutocenter> segmentToolAutocenter;
      RefcountedTyped<SegmentToolPointer> segmentPointer;
      RefcountedTyped<SegmentToolCamera>  segmentToolCamera;
      RefcountedTyped<SegmentToolAnnotations>  segmentToolAnnotations;
      RefcountedTyped<MipToolAnnotations>  mipToolAnnotations;
      RefcountedTyped<MipToolPointer>     mipToolPointer;
      ResourceAnnotations                 annotations;

      RefcountedTyped<Mip>                mip;
      RefcountedTyped<Font>               font;

      ApplicationVariables() : screen( 1280, 1024, 3 ), orderManager( 6 )
      {  
         initFont();
         initContext();

         // MIP
         
         mip = RefcountedTyped<Mip>( new Mip( context.get<ContextVolumes>()->volumes, engineHandler, orderManager, orderManager ) );
         (*mip).volumes.insert( SymbolVolume::create( "pt1" ) );

         annotations.insert( RefcountedTyped<Annotation>( new AnnotationPoint( nll::core::vector3f( 0, 0, -30 ), "point1", *font, 10, nll::core::vector3uc( 255, 255, 255 )) ) );
         annotations.insert( RefcountedTyped<Annotation>( new AnnotationPoint( nll::core::vector3f( 60, 0, 0 ), "point2", *font, 10, nll::core::vector3uc( 255, 255, 255 ) ) ) );
         annotations.insert( RefcountedTyped<Annotation>( new AnnotationPoint( nll::core::vector3f( 0, 20, 0 ), "point3", *font, 10, nll::core::vector3uc( 255, 255, 255 ) ) ) );


         mipToolAnnotations = RefcountedTyped<MipToolAnnotations>( new MipToolAnnotations( annotations, engineHandler ) );
         (*mip).connect( mipToolAnnotations.getDataPtr() );
         mipToolPointer = RefcountedTyped<MipToolPointer>( new MipToolPointer() );
         (*mip).connect( mipToolPointer.getDataPtr() );
         

         initLayout();

         context.get<ContextTools>()->loadVolume( "c:/tmp/hrezct.mf2", SymbolVolume::create( "ct1" ) );
         context.get<ContextTools>()->loadVolume( "c:/tmp/hrezpet.mf2", SymbolVolume::create( "pt1" ) );

 //        context.get<ContextTools>()->loadVolume( "c:/tmp/cc69_ct.mf2", SymbolVolume::create( "ct1" ) );
 //        context.get<ContextTools>()->loadVolume( "c:/tmp/cc69_pet.mf2", SymbolVolume::create( "pt1" ) );

         //context.get<ContextTools>()->loadVolume( "c:/tmp/ctvol.mf2", SymbolVolume::create( "ct1" ) );
         //context.get<ContextTools>()->loadVolume( "c:/tmp/petvol.mf2", SymbolVolume::create( "pt1" ) );

    //    context.get<ContextTools>()->loadVolume( "../../nllTest/data/medical/1_-NAC.mf2", SymbolVolume::create( "pt1" ) );
    //    context.get<ContextTools>()->loadVolume( "../../nllTest/data/medical/1_-CT.mf2", SymbolVolume::create( "ct1" ) );

	//	   context.get<ContextTools>()->loadVolume( "../../nllTest/data/medical/pet.mf2", SymbolVolume::create( "pt1" ) );
   //      context.get<ContextTools>()->loadVolume( "../../nllTest/data/medical/ct.mf2", SymbolVolume::create( "ct1" ) );


         // segment 1
         RefcountedTyped<Segment> segment1;
         context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment1"), segment1 );
         (*segment1).volumes.insert( SymbolVolume::create( "ct1" ) );
         (*segment1).volumes.insert( SymbolVolume::create( "pt1" ) );
         (*segment1).intensities.insert( SymbolVolume::create( "ct1" ), 0.5f );
         (*segment1).intensities.insert( SymbolVolume::create( "pt1" ), 0.5f );

         nll::imaging::LookUpTransformWindowingRGB lutPetImpl( 0, 10000, 255 );
         float red[] = {255, 0, 0};
         lutPetImpl.createColorScale( red );
         ResourceLut lutPet( lutPetImpl );
         ResourceLut lutCt( -200, 200 );

         (*segment1).luts.insert( SymbolVolume::create( "ct1" ), lutCt );
         (*segment1).luts.insert( SymbolVolume::create( "pt1" ), lutPet );

         segmentToolAutocenter = RefcountedTyped<SegmentToolAutocenter>( new SegmentToolAutocenter( engineHandler ) );
         (*segment1).connect( segmentToolAutocenter.getDataPtr() );
         segmentPointer = RefcountedTyped<SegmentToolPointer>( new SegmentToolPointer( *font, 12, engineHandler ) );
         (*segment1).connect( segmentPointer.getDataPtr() );
         segmentToolCamera = RefcountedTyped<SegmentToolCamera>( new SegmentToolCamera() );
         (*segment1).connect( segmentToolCamera.getDataPtr() );
         segmentToolAnnotations = RefcountedTyped<SegmentToolAnnotations>( new SegmentToolAnnotations( annotations, engineHandler ) );
         (*segment1).connect( segmentToolAnnotations.getDataPtr() );

         // segment 2
         RefcountedTyped<Segment> segment2;
         context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment2"), segment2 );
         (*segment2).volumes.insert( SymbolVolume::create( "ct1" ) );
         (*segment2).volumes.insert( SymbolVolume::create( "pt1" ) );
         (*segment2).intensities.insert( SymbolVolume::create( "ct1" ), 0.5f );
         (*segment2).intensities.insert( SymbolVolume::create( "pt1" ), 0.5f );

         (*segment2).luts.insert( SymbolVolume::create( "ct1" ), lutCt );
         (*segment2).luts.insert( SymbolVolume::create( "pt1" ), lutPet );

         (*segment2).connect( segmentToolAutocenter.getDataPtr() );
         (*segment2).connect( segmentToolCamera.getDataPtr() );
         (*segment2).connect( segmentPointer.getDataPtr() );
         (*segment2).connect( segmentToolAnnotations.getDataPtr() );

         // segment 3
         RefcountedTyped<Segment> segment3;
         context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment3"), segment3 );
         (*segment3).volumes.insert( SymbolVolume::create( "ct1" ) );
         (*segment3).volumes.insert( SymbolVolume::create( "pt1" ) );
         (*segment3).intensities.insert( SymbolVolume::create( "ct1" ), 0.5f );
         (*segment3).intensities.insert( SymbolVolume::create( "pt1" ), 0.5f );

         (*segment3).luts.insert( SymbolVolume::create( "ct1" ), lutCt );
         (*segment3).luts.insert( SymbolVolume::create( "pt1" ), lutPet );

         (*segment3).connect( segmentToolAutocenter.getDataPtr() );
         (*segment3).connect( segmentToolCamera.getDataPtr() );
         (*segment3).connect( segmentPointer.getDataPtr() );
         (*segment3).connect( segmentToolAnnotations.getDataPtr() );


         nll::imaging::LookUpTransformWindowingRGB lutPetImpl2( -1000, 15000, 256 );
         for ( int n = 0; n < 255; ++n )
         {
            //float grey[] = {255.01f - n, 255.01f - n, 255.01f - n};
            float grey[] = {255.0f - n, 255.0f - n, 255.0f - n};
            lutPetImpl2.set( n, grey );
         }
         ResourceLut lutPet2( lutPetImpl2 );
         (*mip).lut = lutPet2;


         // event
         mouseEvent.isMouseRightButtonJustReleased = false;
         mouseEvent.isMouseLeftButtonJustReleased = false;
         mouseEvent.isMouseRightButtonJustPressed = false;
         mouseEvent.isMouseLeftButtonJustPressed = false;
         mouseEvent.isMouseRightButtonPressed = false;
         mouseEvent.isMouseLeftButtonPressed = false;
      }

   private:
      void initFont()
      {
         std::vector<char> chars;
         std::ifstream i( "../../nllTest/data/font/bitmapfont1_24.txt" );
         std::string line;
         while ( !i.eof() )
         {
            std::getline( i, line );
            ensure( line.size() < 2, "expected 1 character by line" );
            if ( line.size() == 1 )
            {
               chars.push_back( line[ 0 ] );
            } else chars.push_back( static_cast<char>( 0 ) );
         }
         ensure( chars.size() == 256, "we are expecting 256 chars, currently=" + nll::core::val2str( chars.size() ) );
         font = RefcountedTyped<Font>( new FontBitmapMatrix( "../../nllTest/data/font/bitmapfont1_24.bmp", nll::core::vector2ui( 16, 16 ), nll::core::vector2ui( 16, 16 ), chars ) );
      }

      void initContext()
      {
         ContextVolumes* ctxVolumes = new ContextVolumes();
         context.add( ctxVolumes );

         ContextTools* ctxTools = new ContextTools( ctxVolumes->volumes, engineHandler, orderManager, orderManager );
         context.add( ctxTools );

         ContextSegments* ctxSegments = new ContextSegments();
         context.add( ctxSegments );
      }

      void initLayout()
      {
         Segment* segment0 = new Segment( context.get<ContextVolumes>()->volumes, engineHandler, orderManager, orderManager );
         context.get<ContextSegments>()->segments.insert( SymbolSegment::create( "segment1" ), RefcountedTyped<Segment>( segment0 ) );
         segment0->directionx.setValue( nll::core::vector3f( 1, 0, 0 ) );
         segment0->directiony.setValue( nll::core::vector3f( 0, 1, 0 ) );

         Segment* segment1 = new Segment( context.get<ContextVolumes>()->volumes, engineHandler, orderManager, orderManager );
         context.get<ContextSegments>()->segments.insert( SymbolSegment::create( "segment2" ), RefcountedTyped<Segment>( segment1 ) );
         segment1->directionx.setValue( nll::core::vector3f( 0, 1, 0 ) );
         segment1->directiony.setValue( nll::core::vector3f( 0, 0, 1 ) );

         Segment* segment2 = new Segment( context.get<ContextVolumes>()->volumes, engineHandler, orderManager, orderManager );
         context.get<ContextSegments>()->segments.insert( SymbolSegment::create( "segment3" ), RefcountedTyped<Segment>( segment2 ) );
         segment2->directionx.setValue( nll::core::vector3f( 1, 0, 0 ) );
         segment2->directiony.setValue( nll::core::vector3f( 0, 0, 1 ) );

         
         Segment* segment3 = new Segment( context.get<ContextVolumes>()->volumes, engineHandler, orderManager, orderManager );
         context.get<ContextSegments>()->segments.insert( SymbolSegment::create( "segment4" ), RefcountedTyped<Segment>( segment3 ) );
         segment3->directionx.setValue( nll::core::vector3f( 0, 1, 0 ) );
         segment3->directiony.setValue( nll::core::vector3f( 0, 0, -1 ) );
         //segment3->interpolation.setValue( NEAREST );
         

         PaneSegment* e0 = new PaneSegment(nll::core::vector2ui( 0, 0 ),
                                           nll::core::vector2ui( 0, 0 ),
                                           RefcountedTyped<Segment>( segment0 ) );
         
         PaneSegment* e1 = new PaneSegment(nll::core::vector2ui( 0, 0 ),
                                           nll::core::vector2ui( 0, 0 ),
                                           RefcountedTyped<Segment>( segment1 ) );

         PaneSegment* e2 = new PaneSegment(nll::core::vector2ui( 0, 0 ),
                                           nll::core::vector2ui( 0, 0 ),
                                           RefcountedTyped<Segment>( segment2 ) );


         PaneMip* e3 = new PaneMip( nll::core::vector2ui( 0, 0 ),
                                    nll::core::vector2ui( 0, 0 ),
                                    mip );
         
         PaneListVertical* vlist = new PaneListVertical( nll::core::vector2ui( 0, 0 ),
                                                         nll::core::vector2ui( screen.sizex(), screen.sizey() ) );

         PaneListHorizontal* list = new PaneListHorizontal( nll::core::vector2ui( 0, 0 ),
                                                            nll::core::vector2ui( screen.sizex(), screen.sizey() ) );
         PaneListHorizontal* list2 = new PaneListHorizontal( nll::core::vector2ui( 0, 0 ),
                                                            nll::core::vector2ui( screen.sizex(), screen.sizey() ) );

         vlist->addChild( RefcountedTyped<Pane>( list ), 0.51 );
         vlist->addChild( RefcountedTyped<Pane>( list2 ), 0.49 );

         list->addChild( RefcountedTyped<Pane>( e0 ), 0.5f );
         list->addChild( RefcountedTyped<Pane>( e1 ), 0.5f );

         list2->addChild( RefcountedTyped<Pane>( e2 ), 0.5f );
         list2->addChild( RefcountedTyped<Pane>( e3 ), 0.5f );
         layout = RefcountedTyped<Pane>( vlist );                                                    
      }
   };
}
*/