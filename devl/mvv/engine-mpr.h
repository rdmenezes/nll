#ifndef MVV_ENGINE_MPR_H_
# define MVV_ENGINE_MPR_H_

# include "dynamic-resource.h"
# include "drawable.h"

namespace mvv
{
   /**
    @ingroup mvv
    @brief A multiplanar reconstruction object
    */
   class EngineMpr : public Engine, Drawable
   {
   public:
      EngineMpr() : _sx( 0 ), _sy( 0 )
      {}

      /**
       @brief Consume an order
       */
      virtual void consume( Order* )
      {
      }

      /**
       @brief Compute a MPR
       */
      virtual void _run()
      {
      }

      /**
       @brief Return a MPR fully processed and ready to be drawed
       */
      virtual const Image& draw()
      {
         if ( _slice.sizex() != _sx || _slice.sizey() != _sy )
         {
            // run another time in case it wasn't correctly notified
            notify();
            run();
            
            // now just resample the image... we have to wait for the updated asynchronous order...
            // TODO
         }
         return _slice;
      }

      /**
       @brief Specifies the size of the MPR in pixel
       */
      virtual void setImageSize( ui32 sx, ui32 sy )
      {
         _sx = sx;
         _sy = sy;
      }

   protected:
      Image    _slice;
      ui32     _sx;
      ui32     _sy;
   };
}

#endif
