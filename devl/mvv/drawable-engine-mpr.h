#ifndef MVV_DRAWABLE_ENGINE_MPR_H_
# define MVV_DRAWABLE_ENGINE_MPR_H_

# include "engine-mpr.h"
# include "symbol.h"
# include "context-mpr.h"
# include "interaction-event.h"

namespace mvv
{
   /// forward declaration
   class DrawableMprToolkits;

   class MVV_API MprToolkit : public InteractionEventReceiver, public EngineRunnable
   {
   public:
      MprToolkit( DrawableMprToolkits& toolkits, OrderProvider& orderProvider, const ResourceImageRGB& mpr ) : _toolkits( toolkits ), _orderProvider( orderProvider ), _mpr( mpr )
      {
      }

      virtual ~MprToolkit()
      {
      }

      virtual const ResourceImageRGB& getImage()
      {
         return _mpr;
      }

   protected:
      OrderProvider&          _orderProvider;
      const ResourceImageRGB& _mpr;
      DrawableMprToolkits&    _toolkits;
   };

   class MVV_API MprToolkitTranslation : public MprToolkit
   {
   public:
      MprToolkitTranslation( DrawableMprToolkits& toolkits, OrderProvider& orderProvider, const ResourceImageRGB& mpr ) : MprToolkit( toolkits, orderProvider, mpr )
      {
         _isLeftCurrentlyPressed = false;
         _isRightCurrentlyPressed = false;
      }

   protected:
      virtual bool _run()
      {
         return true;
      }

      virtual void consume( Order* )
      {
         // we don't want to send asynchronous orders
      }

      void handle( const InteractionEvent& event );

   private:
      bool                 _isLeftCurrentlyPressed;
      bool                 _isRightCurrentlyPressed;
      nll::core::vector3d  _initialOrigin;
      nll::core::vector2d  _initialZoom;
      nll::core::vector2i  _initialMousePos;
   };

   /**
    @brief Handle a MPR rendering and pluggable toolkits

    Toolkits will be handled form first added to last added
    */
   class MVV_API DrawableMprToolkits : public Drawable, public InteractionEventReceiver, public OrderCreator
   {
      typedef std::vector<MprToolkit*>  Toolkits;

   public:
      DrawableMprToolkits( OrderProvider& orderProviderv,
                           ResourceVolumes& volumesv,
                           ResourceVector3d& originv,
                           ResourceVector3d& vector1v,
                           ResourceVector3d& vector2v,
                           ResourceVector2d& zoomv,
                           ResourceVolumeIntensities& intensitiesv,
                           ResourceLuts& lutsv ) : _orderProvider( orderProviderv ),
                                                   volumes( volumesv ),
                                                   origin( originv ),
                                                   vector1( vector1v ),
                                                   vector2( vector2v ),
                                                   zoom( zoomv ),
                                                   intensities( intensitiesv ),
                                                   luts( lutsv ),
                                                   _mpr( new EngineMprImpl( orderProviderv,
                                                                            volumesv,
                                                                            originv,
                                                                            vector1v,
                                                                            vector2v,
                                                                            zoomv,
                                                                            renderingSize,
                                                                            intensitiesv,
                                                                            lutsv ) ),
                                                      slice( _mpr->outFusedMpr )
                                                   

      {
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

      /**
       @brief Retutns the rendered & decorated MPR

       If toolkits are, the last one is the one supposed to hold the result
       */
      virtual const Image& draw()
      {
         // if we have toolkits, pick the last one, which is supposed to old the final frame to be rendered
         if ( _toolkits.size() )
         {
            const Image& image = (*_toolkits.rbegin())->getImage().image;

            if ( image.sizex() != renderingSize[ 0 ] || 
                 image.sizey() != renderingSize[ 1 ] )
            {
               std::cout << "Warning: resize" << std::endl;
               Image tmp;
               tmp.clone( image );
               return tmp;
            }
            return image;
         }

         // else just pick the raw MPR slice
         if ( _mpr->outFusedMpr.image.sizex() != renderingSize[ 0 ] || _mpr->outFusedMpr.image.sizey() != renderingSize[ 1 ] )
         {
            // we need to rescale for this frame the current MPR as the size is different (asynchrone results)
            // it will be correctly updated later...
            nll::core::rescaleBilinear( _mpr->outFusedMpr.image, renderingSize[ 0 ], renderingSize[ 1 ] );
         }
         return _mpr->outFusedMpr.image;
      }

      /**
       @brief Set the new size if the layouts have to resize the window, we need to recompute a MPR
       */
      virtual void setImageSize( ui32 sx, ui32 sy )
      {
         renderingSize.setValue( 0, sx );
         renderingSize.setValue( 1, sy );
      }

      /**
       @brief Handle all the mouse & keyboard event throught the toolkits
       */
      virtual void handle( const InteractionEvent& event )
      {
         for ( Toolkits::iterator it = _toolkits.begin(); it != _toolkits.end(); ++it )
            (*it)->handle( event );
      }

      virtual void run()
      {
         _mpr->run();
         for ( Toolkits::iterator it = _toolkits.begin(); it != _toolkits.end(); ++it )
            (*it)->run();
      }

      virtual void consume( Order* o )
      {
         _mpr->consume( o );
         for ( Toolkits::iterator it = _toolkits.begin(); it != _toolkits.end(); ++it )
            (*it)->consume( o );
      }

   private:
      // non copiable
      DrawableMprToolkits& operator=( const DrawableMprToolkits );
      DrawableMprToolkits( const DrawableMprToolkits& );


   public:
      // all resources are published
      ResourceVector2ui          renderingSize;
      ResourceVolumes&           volumes;
      ResourceVector3d&          origin;
      ResourceVector3d&          vector1;
      ResourceVector3d&          vector2;
      ResourceVector2d&          zoom;
      ResourceVolumeIntensities& intensities;
      ResourceLuts&              luts;

   protected:
      EngineMprImpl*     _mpr;
      Toolkits           _toolkits;
      OrderProvider&     _orderProvider;

   public:
      ResourceImageRGB&          slice;
   };
}

#endif