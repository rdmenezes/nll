#ifndef MVV_PLATFORM_LAYOUT_SEGMENT_H_
# define MVV_PLATFORM_LAYOUT_SEGMENT_H_

# include <mvvPlatform/layout-pane.h>
# include <mvvPlatform/context-global.h>
# include "segment.h"

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API PaneSegment : public Pane
   {
   public:
      PaneSegment( const nll::core::vector2ui& origin,
                   const nll::core::vector2ui& size,
                   RefcountedTyped<Segment> segment,
                   Context& context ) : Pane( origin, size, nll::core::vector3uc( 0, 0, 0 ) ), _segment( segment ), _context( context )
      {
      }

      virtual void destroy()
      {
      }

      void draw( Image& image )
      {
         if ( !_visible )
            return;

         // always display the segment
         _draw( image );

         for ( Panes::iterator it = _widgets.begin(); it != _widgets.end(); ++it )
         {
            (**it).draw( image );
         }
      }

      virtual void _draw( Image& image )
      {
         // source image
         Image& src = (*_segment).segment.getValue().getStorage();
         if ( !src.getBuf() )
         {
            //std::cout << "--noimage--" << std::endl;
            // empty image, don't do anything...
            return;
         }

         //nll::core::writeBmp( src, "c:/tmp/test--1.bmp" );
         //std::cout << "export image" << std::endl;

         nll::core::vector2ui size( src.sizex(), src.sizey() );
         if ( size != _size )
         {
            std::cout << "resample!" << std::endl;
            // we need to resample! // this should be a very rare state!!
            nll::core::rescaleBilinear( src, _size[ 0 ], _size[ 1 ] );
         }

         // safety check
         if ( _origin[ 0 ] + _size[ 0 ] > image.sizex() ||
              _origin[ 1 ] + _size[ 1 ] > image.sizey() )
         {
            return;
         }

         // print image
         Image::DirectionalIterator oy = image.getIterator( _origin[ 0 ], _origin[ 1 ], 0 );
         Image::DirectionalIterator iy = src.getIterator( 0, 0, 0 );

         for ( ui32 yy = 0; yy < _size[ 1 ]; ++yy )
         {
            Image::DirectionalIterator lo = oy;
            Image::DirectionalIterator li = iy;
            for ( ui32 xx = 0; xx < _size[ 0 ]; ++xx )
            {
               lo.pickcol( 0 ) = li.pickcol( 0 );
               lo.pickcol( 1 ) = li.pickcol( 1 );
               lo.pickcol( 2 ) = li.pickcol( 2 );

               lo.addx();
               li.addx();
            }
            oy.addy();
            iy.addy();
         }

         // draw canvas
         Image::DirectionalIterator oy2 = image.getIterator( _origin[ 0 ], _origin[ 1 ] + _size[ 1 ] - 1, 0 );
         for ( ui32 n = 0; n < _size[ 0 ]; ++n )
         {
            oy2.pickcol( 0 ) = 180;
            oy2.pickcol( 1 ) = 180;
            oy2.pickcol( 2 ) = 180;

            oy2.addx();
         }

         oy2 = image.getIterator( _origin[ 0 ] + _size[ 0 ] - 1, _origin[ 1 ], 0 );
         for ( ui32 n = 0; n < _size[ 1 ]; ++n )
         {
            oy2.pickcol( 0 ) = 180;
            oy2.pickcol( 1 ) = 180;
            oy2.pickcol( 2 ) = 180;

            oy2.addy();
         }
      }

      virtual void updateLayout()
      {
         // update the display area
         (*_segment).size.setValue( _size );
      }

      virtual Pane* find( const mvv::Symbol& )
      {
         return 0;
      }

   protected:
      virtual void _receive( const EventMouse& e )
      {
         ContextGlobal* global = _context.get<platform::ContextGlobal>();
         if ( global )
         {
            // run specifc mouse event callbacks
            if ( e.isMouseLeftButtonJustPressed && !e.isMouseRightButtonPressed && !global->onSegmentLeftMouseClick.isEmpty() && global->onSegmentLeftMouseClick.getDataPtr() )
            {
               (*global->onSegmentLeftMouseClick).run( (*_segment).segment.getValue(), e, _origin );
               return;
            }

            if ( e.isMouseLeftButtonJustReleased && !e.isMouseRightButtonPressed && !global->onSegmentLeftMouseRelease.isEmpty() && global->onSegmentLeftMouseRelease.getDataPtr() )
            {
               (*global->onSegmentLeftMouseRelease).run( (*_segment).segment.getValue(), e, _origin );
               return;
            }

            if ( e.isMouseRightButtonJustPressed && !e.isMouseLeftButtonPressed && !global->onSegmentRightMouseClick.isEmpty() && global->onSegmentRightMouseClick.getDataPtr() )
            {
               (*global->onSegmentRightMouseClick).run( (*_segment).segment.getValue(), e, _origin );
               return;
            }

            if ( e.isMouseRightButtonJustReleased && !e.isMouseLeftButtonPressed && !global->onSegmentRightMouseRelease.isEmpty() && global->onSegmentRightMouseRelease.getDataPtr() )
            {
               (*global->onSegmentRightMouseRelease).run( (*_segment).segment.getValue(), e, _origin );
               return;
            }

            if ( e.isMouseLeftButtonPressed && !e.isMouseRightButtonPressed && !global->onSegmentLeftMousePressed.isEmpty() && global->onSegmentLeftMousePressed.getDataPtr() )
            {
               (*global->onSegmentLeftMousePressed).run( (*_segment).segment.getValue(), e, _origin );
               return;
            }

            if ( e.isMouseRightButtonPressed && !e.isMouseLeftButtonPressed && !global->onSegmentRightMousePressed.isEmpty() && global->onSegmentRightMousePressed.getDataPtr() )
            {
               (*global->onSegmentRightMousePressed).run( (*_segment).segment.getValue(), e, _origin );
               return;
            }
         }
         (*_segment).receive( e, _origin );
      }

   protected:
      RefcountedTyped<Segment>   _segment;
      Context&                   _context;
   };
}
}

#endif