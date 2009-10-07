#ifndef MVV_DRAWABLE_ENGINE_MPR_H_
# define MVV_DRAWABLE_ENGINE_MPR_H_

# include "engine-mpr.h"
# include "symbol.h"
# include "context.h"
# include "interaction-event.h"

namespace mvv
{
public:
   class MprToolkit : public InteractionEventReceiver, public EngineRunnable
   {
   public:
   };

   class DrawableMprToolkits : public Drawable, public InteractionEventReceiver
   {
   public:
      DrawableMprToolkits( const Symbol& mprName )
      {
         _mpr = 0;
         init( mprName );
      }

      ~DrawableMprToolkits()
      {
         delete _mpr;
      }

      virtual const Image& draw()
      {
         if ( outFusedMpr.image.sizex() != _renderingSize[ 0 ] || outFusedMpr.image.sizey() != _renderingSize[ 1 ] )
         {
            // we need to rescale for this frame the current MPR as the size is different (asynchrone results)
            // it will be correctly updated later...
            nll::core::rescaleBilinear( outFusedMpr.image, _renderingSize[ 0 ], _renderingSize[ 1 ] );
         }
         return outFusedMpr.image;
      }

      virtual void setImageSize( ui32 sx, ui32 sy )
      {
         _renderingSize[ 0 ] = sx;
         _renderingSize[ 1 ] = sy;
      }

      virtual void handle( const InteractionEvent& event )
      {
      }

   protected:
      void init( const Symbol& mprName )
      {
         _mprName = mprName;

         // get the mpr
         ContextMpr* mprs = Context::instance().get<ContextMpr>();
         ensure( mprs, "first init correctly the context!" );

         ContextMpr::ContextMprInstance* context = mprs->getMpr( mprName );
         ensure( mpr, "the MPR doesn't exist in the context!" );

         _mpr = new EngineMprImpl( ResourceManager::instance(),
                                   context->volumes,
                                   context->origin,
                                   context->vector1,
                                   context->vector2,
                                   context->zoom,
                                   _renderingSize );
      }

   private:
      // non copiable
      DrawableMprToolkits& operator=( const DrawableMprToolkits );
      DrawableMprToolkits( const DrawableMprToolkits& );


   protected:
      Symbol             _mprName;
      EngineMprImpl*     _mpr;
      ResourceVector2ui  _renderingSize;
   };
}

#endif