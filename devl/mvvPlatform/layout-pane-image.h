#ifndef MVV_PLATFORM_LAYOUT_IMAGE_H_
# define MVV_PLATFORM_LAYOUT_IMAGE_H_

# include "layout-pane.h"

namespace mvv
{
namespace platform
{
   class MVVPLATFORM_API PaneImage : public Pane 
   {
   public:
      class EngineMonitorImageChange : public Engine
      {
      public:
         EngineMonitorImageChange( EngineHandler& handler, PaneImage& li ) : Engine( handler ), _li( li )
         {
            li.image.connect( this );
         }

         virtual bool _run()
         {
            _li.notify();
            return true;
         }

      private:
         PaneImage&   _li;
      };

      PaneImage( const nll::core::vector2ui& origin,
                   const nll::core::vector2ui& size,
                   const nll::core::vector3uc background,
                   EngineHandler& handler) : Pane( origin, size, background )
      {
         // create the engine...
         _monitor = RefcountedTyped<EngineMonitorImageChange>( new EngineMonitorImageChange( handler, *this ) );
      }

      virtual void destroy()
      {
      }

      virtual void updateLayout()
      {
         notify();
      }

      virtual Pane* find( const mvv::Symbol& )
      {
         // no hooks are needed...
         return 0;
      }

      virtual void _draw( Image& screen )
      {
         // safety check
         if ( _origin[ 0 ] + _size[ 0 ] > screen.sizex() ||
              _origin[ 1 ] + _size[ 1 ] > screen.sizey() )
         {
            return;
         }

         // resample if necessary
         nll::core::Image<ui8> imageResampled;
         nll::core::Image<ui8>* toDisplay = 0;
         if ( image.getValue().sizex() == _size[ 0 ] && image.getValue().sizey() == _size[ 1 ] )
         {
            toDisplay = &image.getValue();
            std::cout << "pane image not resampled..." << std::endl;
         } else {
            imageResampled.clone( image.getValue() );
            nll::core::rescaleBilinear( imageResampled, _size[ 0 ], _size[ 1 ] );
            toDisplay = &imageResampled;
            std::cout << "pane image resampled..." << std::endl;
         }

         // display
         Image::DirectionalIterator oy = screen.getIterator( _origin[ 0 ], _origin[ 1 ], 0 );
         Image::DirectionalIterator iy = toDisplay->getIterator( 0, 0, 0 );

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

   public:
      // the resource to display, internally it will be bilinearly resampled to match the window size
      ResourceImageuc   image;

   private:
      RefcountedTyped<EngineMonitorImageChange> _monitor;
   };
}
}

#endif