#ifndef MVV_PLATFORM_LAYOUT_MIP_H_
# define MVV_PLATFORM_LAYOUT_MIP_H_

# include <mvvPlatform/layout-pane.h>
# include "mip.h"

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API PaneMip : public Pane
   {
   public:
      PaneMip( const nll::core::vector2ui& origin,
                   const nll::core::vector2ui& size,
                   RefcountedTyped<Mip> mip ) : Pane( origin, size, nll::core::vector3uc( 0, 0, 0 ) ), _mip( mip )
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
         Image& src = (*_mip).outImage.getValue().getStorage();
         nll::core::vector2ui size( src.sizex(), src.sizey() );
         if ( !src.sizex() || !src.sizey() )
            return;
         if ( size != _size )
         {
            // we need to resample! // this should be a very rare state!!
            nll::core::rescaleBilinear( src, _size[ 0 ], _size[ 1 ] );
         }

         // safety check
         if ( _origin[ 0 ] + _size[ 0 ] > image.sizex() ||
              _origin[ 1 ] + _size[ 1 ] > image.sizey() )
         {
            return;
         }

         // copy the image
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
            oy2.pickcol( 0 ) = 255;
            oy2.pickcol( 1 ) = 255;
            oy2.pickcol( 2 ) = 255;

            oy2.addy();
         }
      }

      virtual Pane* find( const mvv::Symbol& )
      {
         return 0;
      }

      virtual void updateLayout()
      {
         // update the display size
         (*_mip).size.setValue( _size );
      }

   protected:
      virtual void _receive( const EventMouse& e )
      {
         (*_mip).receive( e, _origin );
      }


   private:
      RefcountedTyped<Mip>    _mip;
   };
}
}

#endif