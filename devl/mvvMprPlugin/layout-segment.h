#ifndef MVV_PLATFORM_LAYOUT_SEGMENT_H_
# define MVV_PLATFORM_LAYOUT_SEGMENT_H_

# include <mvvPlatform/layout-pane.h>
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
                   RefcountedTyped<Segment> segment ) : Pane( origin, size, nll::core::vector3uc( 0, 0, 0 ) ), _segment( segment )
      {
      }

      virtual void destroy()
      {
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
      }

      virtual void updateLayout()
      {
         // update the display area
         (*_segment).size.setValue( _size );
      }

      virtual bool sendMessage( const std::string&, const nll::core::vector3uc& )
      {
         return false;
      }

   protected:
      virtual void _receive( const EventMouse& e )
      {
         (*_segment).receive( e, _origin );
      }

   protected:
      RefcountedTyped<Segment>   _segment;
   };
}
}

#endif