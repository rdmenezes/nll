# include "layout-pane-textbox.h"

namespace mvv
{
namespace platform
{
   void PaneTextbox::_draw( Image& image )
   {
      if ( _needToBeRefreshed )
      {
         nll::core::Timer timer;
         // clear the textbox, we need to reprint everything
         Image::DirectionalIterator oy = image.getIterator( _origin[ 0 ], _origin[ 1 ], 0 );

         for ( ui32 yy = 0; yy < _size[ 1 ]; ++yy )
         {
            Image::DirectionalIterator lo = oy;
            for ( ui32 xx = 0; xx < _size[ 0 ]; ++xx )
            {
               lo.pickcol( 0 ) = _background[ 0 ];
               lo.pickcol( 1 ) = _background[ 1 ];
               lo.pickcol( 2 ) = _background[ 2 ];

               lo.addx();
            }
            oy.addy();
         }

         _drawText( image );

         // finally decorate the text...
         for ( unsigned n = 0; n < _decorators.size(); ++n )
         {
            (*_decorators[ n ]).draw( image );
         }
         _needToBeRefreshed = false;
      }
   }

   void PaneTextbox::_receive( const EventMouse& e )
   {
      for ( ui32 n = 0; n < _decorators.size(); ++n )
      {
         bool res = (*_decorators[ n ]).receive( e );
         if ( res )
            break;
      }
   }

   void PaneTextbox::_receive( const EventKeyboard& e )
   {
      for ( ui32 n = 0; n < _decorators.size(); ++n )
      {
         bool res = (*_decorators[ n ]).receive( e );
         if ( res )
            break;
      }
   }
}
}