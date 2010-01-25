#ifndef MVV_INIT_H_
# define MVV_INIT_H_

# include <mvvPlatform/types.h>
# include <mvvPlatform/event-mouse.h>
# include <mvvPlatform/layout-pane.h>
# include <mvvPlatform/layout-widget.h>
# include <mvvPlatform/context.h>
# include <mvvPlatform/context-volumes.h>
# include <mvvPlatform/context-tools.h>
# include <mvvPlatform/engine-handler-impl.h>
# include <mvvMprPlugin/context-segments.h>
# include <mvvMprPlugin/layout-segment.h>
# include <mvvMprPlugin/segment-tool-pointer.h>
# include <mvvMprPlugin/segment-tool-camera.h>
# include <mvvMprPlugin/layout-mip.h>


using namespace mvv;
using namespace mvv::platform;

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

      RefcountedTyped<SegmentToolPointer> segmentPointer;
      RefcountedTyped<SegmentToolCamera>  segmentToolCamera;
      RefcountedTyped<Mip>                mip;

      ApplicationVariables() : screen( 1024, 1024, 3 ), orderManager( 6 )
      {  
         initContext();

         // MIP
         mip = RefcountedTyped<Mip>( new Mip( context.get<ContextVolumes>()->volumes, engineHandler, orderManager, orderManager ) );
         (*mip).volumes.insert( SymbolVolume::create( "pt1" ) );

         initLayout();

       // context.get<ContextTools>()->loadVolume( "../../nllTest/data/medical/1_-NAC.mf2", SymbolVolume::create( "pt1" ) );
       // context.get<ContextTools>()->loadVolume( "../../nllTest/data/medical/1_-CT.mf2", SymbolVolume::create( "ct1" ) );

		   context.get<ContextTools>()->loadVolume( "../../nllTest/data/medical/pet.mf2", SymbolVolume::create( "pt1" ) );
        // context.get<ContextTools>()->loadVolume( "../../nllTest/data/medical/ct.mf2", SymbolVolume::create( "ct1" ) );


         // segment 1
         RefcountedTyped<Segment> segment1;
         context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment1"), segment1 );
         (*segment1).volumes.insert( SymbolVolume::create( "ct1" ) );
         (*segment1).volumes.insert( SymbolVolume::create( "pt1" ) );
         (*segment1).intensities.insert( SymbolVolume::create( "ct1" ), 0.5f );
         (*segment1).intensities.insert( SymbolVolume::create( "pt1" ), 0.5f );

         nll::imaging::LookUpTransformWindowingRGB lutPetImpl( 0, 10000, 4500 );
         float red[] = {255, 0, 0};
         lutPetImpl.createColorScale( red );
         ResourceLut lutPet( lutPetImpl );
         ResourceLut lutCt( -200, 1000 );

         (*segment1).luts.insert( SymbolVolume::create( "ct1" ), lutCt );
         (*segment1).luts.insert( SymbolVolume::create( "pt1" ), lutPet );

         segmentPointer = RefcountedTyped<SegmentToolPointer>( new SegmentToolPointer( engineHandler ) );
         (*segment1).connect( segmentPointer.getDataPtr() );
         segmentToolCamera = RefcountedTyped<SegmentToolCamera>( new SegmentToolCamera( context.get<ContextVolumes>()->volumes, engineHandler ) );
         (*segment1).connect( segmentToolCamera.getDataPtr() );

         // segment 2
         RefcountedTyped<Segment> segment2;
         context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment2"), segment2 );
         (*segment2).volumes.insert( SymbolVolume::create( "ct1" ) );
         (*segment2).volumes.insert( SymbolVolume::create( "pt1" ) );
         (*segment2).intensities.insert( SymbolVolume::create( "ct1" ), 0.5f );
         (*segment2).intensities.insert( SymbolVolume::create( "pt1" ), 0.5f );

         (*segment2).luts.insert( SymbolVolume::create( "ct1" ), lutCt );
         (*segment2).luts.insert( SymbolVolume::create( "pt1" ), lutPet );

         (*segment2).connect( segmentToolCamera.getDataPtr() );
         (*segment2).connect( segmentPointer.getDataPtr() );

         // segment 3
         RefcountedTyped<Segment> segment3;
         context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment3"), segment3 );
         (*segment3).volumes.insert( SymbolVolume::create( "ct1" ) );
         (*segment3).volumes.insert( SymbolVolume::create( "pt1" ) );
         (*segment3).intensities.insert( SymbolVolume::create( "ct1" ), 0.5f );
         (*segment3).intensities.insert( SymbolVolume::create( "pt1" ), 0.5f );

         (*segment3).luts.insert( SymbolVolume::create( "ct1" ), lutCt );
         (*segment3).luts.insert( SymbolVolume::create( "pt1" ), lutPet );

         (*segment3).connect( segmentToolCamera.getDataPtr() );
         (*segment3).connect( segmentPointer.getDataPtr() );


         nll::imaging::LookUpTransformWindowingRGB lutPetImpl2( -1000, 15000, 256 );
         for ( int n = 0; n < 255; ++n )
         {
            //float grey[] = {255.01f - n, 255.01f - n, 255.01f - n};
            float grey[] = {255 - n, 255 - n, 255 - n};
            lutPetImpl2.set( n, grey );
         }
         ResourceLut lutPet2( lutPetImpl2 );
         (*mip).lut = lutPet2;

         /*
         // segment 4
         RefcountedTyped<Segment> segment4;
         context.get<ContextSegments>()->segments.find( SymbolSegment::create("segment4"), segment4 );
         //(*segment4).volumes.insert( SymbolVolume::create( "ct1" ) );
         (*segment4).volumes.insert( SymbolVolume::create( "pt1" ) );
         //(*segment4).intensities.insert( SymbolVolume::create( "ct1" ), 0.5f );
         (*segment4).intensities.insert( SymbolVolume::create( "pt1" ), 0.5f );

         //(*segment4).luts.insert( SymbolVolume::create( "ct1" ), lutCt );
         (*segment4).luts.insert( SymbolVolume::create( "pt1" ), lutPet );

         (*segment4).connect( segmentToolCamera.getDataPtr() );
         (*segment4).connect( segmentPointer.getDataPtr() );
         */


         // event
         mouseEvent.isMouseRightButtonJustReleased = false;
         mouseEvent.isMouseLeftButtonJustReleased = false;
         mouseEvent.isMouseRightButtonJustPressed = false;
         mouseEvent.isMouseLeftButtonJustPressed = false;
         mouseEvent.isMouseRightButtonPressed = false;
         mouseEvent.isMouseLeftButtonPressed = false;
      }

   private:
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

         /*
         Segment* segment3 = new Segment( context.get<ContextVolumes>()->volumes, engineHandler, orderManager, orderManager );
         context.get<ContextSegments>()->segments.insert( SymbolSegment::create( "segment4" ), RefcountedTyped<Segment>( segment3 ) );
         segment3->directionx.setValue( nll::core::vector3f( 0, 1, 0 ) );
         segment3->directiony.setValue( nll::core::vector3f( 0, 0, -1 ) );
         segment3->interpolation.setValue( NEAREST );
         */

         PaneSegment* e0 = new PaneSegment(nll::core::vector2ui( 0, 0 ),
                                           nll::core::vector2ui( 0, 0 ),
                                           RefcountedTyped<Segment>( segment0 ) );
         
         PaneSegment* e1 = new PaneSegment(nll::core::vector2ui( 0, 0 ),
                                           nll::core::vector2ui( 0, 0 ),
                                           RefcountedTyped<Segment>( segment1 ) );

         PaneSegment* e2 = new PaneSegment(nll::core::vector2ui( 0, 0 ),
                                           nll::core::vector2ui( 0, 0 ),
                                           RefcountedTyped<Segment>( segment2 ) );

         /*
         PaneSegment* e3 = new PaneSegment(nll::core::vector2ui( 0, 0 ),
                                           nll::core::vector2ui( 0, 0 ),
                                           RefcountedTyped<Segment>( segment3 ) );
         */

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

#endif

/*
layout = RefcountedTyped<Pane>( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                        nll::core::vector2ui( screen.sizex(), screen.sizey() ),
                                                        nll::core::vector3uc( 0, 0, 0 ) ) );

        
            Pane::PaneRef entry( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                nll::core::vector2ui( 0, 0 ),
                                                nll::core::vector3uc( 0, 0, 100 ) ) );
            Pane::Panes droplist;
            Pane::PaneRef drop1( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                nll::core::vector2ui( screen.sizex(), screen.sizey() ),
                                                nll::core::vector3uc( 0, 255, 0 ) ) );
            Pane::PaneRef drop2( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                nll::core::vector2ui( screen.sizex(), screen.sizey() ),
                                                nll::core::vector3uc( 0, 200, 0 ) ) );
            Pane::PaneRef drop3( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                nll::core::vector2ui( screen.sizex(), screen.sizey() ),
                                                nll::core::vector3uc( 0, 150, 0 ) ) );
            droplist.push_back( drop1 );
            droplist.push_back( drop2 );
            droplist.push_back( drop3 );

            {
               Pane::PaneRef entry2( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                    nll::core::vector2ui( 0, 0 ),
                                                    nll::core::vector3uc( 0, 0, 100 ) ) );
               Pane::Panes droplist2;
               Pane::PaneRef drop21( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                    nll::core::vector2ui( screen.sizex(), screen.sizey() ),
                                                    nll::core::vector3uc( 0, 255, 0 ) ) );
               Pane::PaneRef drop22( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                    nll::core::vector2ui( screen.sizex(), screen.sizey() ),
                                                    nll::core::vector3uc( 0, 200, 0 ) ) );
               Pane::PaneRef drop23( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                    nll::core::vector2ui( screen.sizex(), screen.sizey() ),
                                                    nll::core::vector3uc( 0, 150, 0 ) ) );
               droplist2.push_back( drop21 );
               droplist2.push_back( drop22 );
               droplist2.push_back( drop23 );


               Pane::PaneRef dropDown2( new WidgetDropDown( nll::core::vector2ui( 100, 256 ), nll::core::vector2ui( 40, 20 ), entry2, droplist2 ) );

               droplist.push_back( dropDown2 );
               //(*layout).insert( dropDown2 );
*/