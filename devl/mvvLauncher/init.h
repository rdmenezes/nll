#ifndef MVV_INIT_H_
# define MVV_INIT_H_

# include <mvvPlatform/types.h>
# include <mvvPlatform/event-mouse.h>
# include <mvvPlatform/layout-pane.h>
# include <mvvPlatform/layout-widget.h>


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

      ApplicationVariables()
      {
         screen = Image( 512, 512, 3 );

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
            }
        
            Pane::PaneRef dropDown( new WidgetDropDown( nll::core::vector2ui( 0, 256 ), nll::core::vector2ui( 40, 20 ), entry, droplist ) );
            (*layout).insert( dropDown );
         
         Pane::PaneRef  test( new PaneEmpty( nll::core::vector2ui( 200, 200 ),
                                             nll::core::vector2ui( 40, 40 ),
                                             nll::core::vector3uc( 0, 0, 255 ) ) );
         (*layout).insert( test );
      }
   };
}

#endif