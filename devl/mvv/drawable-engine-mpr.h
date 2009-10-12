#ifndef MVV_DRAWABLE_ENGINE_MPR_H_
# define MVV_DRAWABLE_ENGINE_MPR_H_

# include "engine-mpr.h"
# include "symbol.h"
# include "context-mpr.h"
# include "interaction-event.h"

namespace mvv
{
   class MprToolkit : public InteractionEventReceiver, public EngineRunnable
   {
   public:
   };

   /**
    @brief Handle a MPR rendering and pluggable toolkits

    Toolkits will be handled form first added to last added
    */
   class DrawableMprToolkits : public Drawable, public InteractionEventReceiver, public OrderCreator
   {
      typedef std::vector<MprToolkit*>  Toolkits;

   public:
      DrawableMprToolkits( OrderProvider& orderProvider,
                           ResourceVolumes& volumes,
                           ResourceVector3d& origin,
                           ResourceVector3d& vector1,
                           ResourceVector3d& vector2,
                           ResourceVector2d& zoom,
                           ResourceVolumeIntensities& intensities,
                           ResourceLuts& luts )
      {
         _mpr = new EngineMprImpl( orderProvider,
                                   volumes,
                                   origin,
                                   vector1,
                                   vector2,
                                   zoom,
                                   _renderingSize,
                                   intensities,
                                   luts );
      }

      ~DrawableMprToolkits()
      {
         delete _mpr;

         for ( Toolkits::iterator it = _toolkits.begin(); it != _toolkits.end(); )
         {
            delete *it;
         }
      }

      /**
       @param toolkit must be an allocated object, will be deallocated by this object
       */
      void addToolkit( MprToolkit* toolkit )
      {
         _toolkits.push_back( toolkit );
      }

      /**
       @biref remove a toolkit from this MPR. The toolkit will be deallocated.
       */
      void removeToolkit( MprToolkit* toolkit )
      {
         for ( Toolkits::iterator it = _toolkits.begin(); it != _toolkits.end(); )
         {
            Toolkits::iterator cur = it;
            ++it;
            if ( *cur == toolkit )
            {
               delete *cur;
               _toolkits.erase( cur );
            }
         }
      }

      /**
       @brief Set zoom, orientation, position of the bigggest volume to be the center of the MPR
       */
      void autoFindPosition( EngineMprImpl::Orientation orientation )
      {
         _mpr->autoFindPosition( orientation );
      }

      virtual const Image& draw()
      {
         // TODO update with toolkits
         if ( _mpr->outFusedMpr.image.sizex() != _renderingSize[ 0 ] || _mpr->outFusedMpr.image.sizey() != _renderingSize[ 1 ] )
         {
            // we need to rescale for this frame the current MPR as the size is different (asynchrone results)
            // it will be correctly updated later...
            nll::core::rescaleBilinear( _mpr->outFusedMpr.image, _renderingSize[ 0 ], _renderingSize[ 1 ] );
         }
         return _mpr->outFusedMpr.image;
      }

      virtual void setImageSize( ui32 sx, ui32 sy )
      {
         _renderingSize.setValue( 0, sx );
         _renderingSize.setValue( 1, sy );
      }

      virtual void handle( const InteractionEvent& event )
      {
         // TODO
      }

      virtual void run()
      {
         _mpr->run();
      }

      virtual void consume( Order* o )
      {
         _mpr->consume( o );
      }

   private:
      // non copiable
      DrawableMprToolkits& operator=( const DrawableMprToolkits );
      DrawableMprToolkits( const DrawableMprToolkits& );


   protected:
      EngineMprImpl*     _mpr;
      ResourceVector2ui  _renderingSize;
      Toolkits           _toolkits;
   };
}

#endif