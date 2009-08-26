#ifndef MVV_ENGINE_MPR_H_
# define MVV_ENGINE_MPR_H_

# include "dynamic-resource.h"
# include "drawable.h"
# include "resource.h"

namespace mvv
{
   typedef nll::imaging::VolumeSpatial<double> MedicalVolume;

   /**
    @ingroup mvv
    @brief A multiplanar reconstruction object
    */
   class EngineMpr : public Engine, Drawable
   {
      typedef std::set<MedicalVolume*> Volumes;

   public:
      EngineMpr( ResourceVector3d& origin,
                 ResourceVector3d& vector1,
                 ResourceVector3d& vector2,
                 ResourceVector2d& zoom ) : _sx( 0 ), _sy( 0 ), _origin( origin ),
                 _vector1( vector1 ), _vector2( vector2 ), _zoom( zoom )
      {
         attach( origin );
         attach( vector1 );
         attach( vector2 );
         attach( zoom );
      }

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
            
            // for now just resample the image... we have to wait for the updated asynchronous order
            nll::core::rescaleBilinear( _slice, _sx, _sy );
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

      ResourceVector3d& _origin;
      ResourceVector3d& _vector1;
      ResourceVector3d& _vector2;
      ResourceVector2d& _zoom;
   };
}

#endif
