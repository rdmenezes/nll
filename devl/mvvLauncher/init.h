#ifndef MVV_INIT_H_
# define MVV_INIT_H_

# include <mvvPlatform/types.h>
# include <mvvPlatform/event-mouse.h>
# include <mvvPlatform/layout-pane.h>


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
         screen = Image( 1024, 1024, 3 );

         layout = RefcountedTyped<Pane>( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                        nll::core::vector2ui( screen.sizex(), screen.sizey() ),
                                                        nll::core::vector3uc( 255, 0, 0 ) ) );

         (*layout).insert( RefcountedTyped<Pane>( new PaneEmpty( nll::core::vector2ui( 0, 0 ),
                                                      nll::core::vector2ui( 50, 50 ),
                                                      nll::core::vector3uc( 128, 0, 0 ) ) ) );
      }
   };
}

#endif