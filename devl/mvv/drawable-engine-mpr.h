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

   /*
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

   class MVV_API MprToolkitMove : public MprToolkit
   {
   public:
      MprToolkitMove( DrawableMprToolkits& toolkits, OrderProvider& orderProvider, const ResourceImageRGB& mpr ) : MprToolkit( toolkits, orderProvider, mpr )
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


   class MVV_API MprToolkitTarget : public MprToolkit
   {
   public:
      MprToolkitTarget( DrawableMprToolkits& toolkits, OrderProvider& orderProvider, const ResourceImageRGB& mpr );

   protected:
      virtual bool _run()
      {
         // we don't do anything is the target is not within the MPR
         if ( _targetPos[ 0 ] > _mpr.image.sizex() || _targetPos[ 1 ] > _mpr.image.sizey() )
            return true;

         _img.image.clone( _mpr.image );
         ResourceImageRGB::Image::DirectionalIterator it = _img.image.getIterator( _targetPos[ 0 ], 0, 1 );
         for ( ui32 y = 0; y < _img.image.sizey(); ++y )
         {
            *it = 255;
            it.addy();
         }
         it = _img.image.getIterator( 0, _targetPos[ 1 ], 1 );
         for ( ui32 x = 0; x < _img.image.sizex(); ++x )
         {
            *it = 255;
            it.addx();
         }
         return true;
      }

      virtual void consume( Order* )
      {
         // we don't want to send asynchronous orders
      }

      void handle( const InteractionEvent& event );

      nll::core::vector2i getTargetPsoition() const
      {
         return _targetPos;
      }

      virtual const ResourceImageRGB& getImage()
      {
         return _img;
      }

   private:
      bool                 _isLeftCurrentlyPressed;
      nll::core::vector3d  _initialOrigin;
      nll::core::vector2i  _initialMousePos;
      nll::core::vector2i  _targetPos;

      ResourceImageRGB     _img;
   };
   */

   class MprToolkit : public OrderCreator
   {
   public:
      typedef std::set<DynamicResource*>     Resources;
      typedef std::set<DrawableMprToolkits*> Mprs;

      virtual ~MprToolkit()
      {
      }

      /**
       @brief Returns the resources that will force an update of the DrawableMprToolkits
              (example: the mouse move the target. If the target is moved, the mpr must be updated,
                        consequently the target position is a resouce of the DrawableMprToolkits).
       */
      Resources& getResources()
      {
         return _resources;
      }

      virtual void consume( Order* )
      {
      }

      virtual void run()
      {
      }

      virtual void handle( const InteractionEvent& event, DrawableMprToolkits& source, ResourceImageRGB* mpr )
      {
      }

      /**
       @brief We need to know what DrawableMprToolkits is attached to a Mpr toolkit.

       Example: toolkit to move the MPR position: event is reived, a new MPR position is calculated,
                the toolkit must update the position of all the other MPR
       */
      void attach( DrawableMprToolkits* r )
      {
         _mprs.insert( r );
      }

      void detach( DrawableMprToolkits* r )
      {
         Mprs::iterator it = _mprs.find( r );
         if ( it != _mprs.end() )
            _mprs.erase( it );
      }

      virtual bool isMprMustBeSaved()
      {
         return false;
      }

   protected:
      Resources   _resources;
      Mprs        _mprs;
   };

   /**
    @brief Handle a MPR rendering and pluggable toolkits

    Toolkits will be handled form first added to last added
    */
   class MVV_API DrawableMprToolkits : public Drawable, public InteractionEventReceiver, public OrderCreator
   {
      class MprState : public InteractionEventReceiver, public EngineRunnable
      {
      public:
         MprState( MprToolkit& mprToolkit, DrawableMprToolkits& toolkits, OrderProvider& orderProvider, ResourceImageRGB& mpr, bool createMprCopy = false ) :
            _mprToolkit( mprToolkit ), _source( toolkits ), _orderProvider( orderProvider ), _previous( mpr ), _isMprSaved( createMprCopy )
         {
            if ( _isMprSaved )
            {
               // if we intend to modify the MPR, we need to create a saving point. Changes are triggered
               // when the previous image has changed
               attach( mpr );
            }
         }

         void handle( const InteractionEvent& event )
         {
            _mprToolkit.handle( event, _source, _isMprSaved ? &_previous : 0 );
         }

         virtual void consume( Order* o )
         {
            _mprToolkit.consume( o );
         }

         virtual bool _run()
         {
            _mprToolkit.run();   // TODO: some limitations here on the kind of orders
            return true;
         }

         virtual ResourceImageRGB& getImage()
         {
            if ( _isMprSaved )
               return _img;
            return _previous;
         }

         MprToolkit& getToolkit()
         {
            return _mprToolkit;
         }

      private:
         ResourceImageRGB        _img;
         DrawableMprToolkits&    _source;
         OrderProvider&          _orderProvider;
         ResourceImageRGB&       _previous;
         bool                    _isMprSaved;
         MprToolkit&             _mprToolkit;
      };
      typedef std::vector<MprState*>  States;

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

         for ( States::iterator it = _states.begin(); it != _states.end(); )
         {
            delete *it;
         }
      }

      /**
       @param toolkit must be an allocated object, will be deallocated by this object
       */
      void addToolkit( MprToolkit& toolkit )
      {
         toolkit.attach( this );
         ResourceImageRGB* outputMpr;
         if ( _states.size() )
            outputMpr = &_mpr->outFusedMpr;
         else
            outputMpr = &(*_states.rbegin())->getImage();

         _states.push_back( new MprState( toolkit, *this, _orderProvider, *outputMpr, toolkit.isMprMustBeSaved() ) );
      }

      /**
       @biref remove a toolkit from this MPR. The toolkit will be deallocated.
       */
      void removeToolkit( MprToolkit& toolkit )
      {
         toolkit.detach( this );

         for ( States::iterator it = _states.begin(); it != _states.end(); )
         {
            States::iterator cur = it;
            ++it;
            if ( &(*cur)->getToolkit() == &toolkit )
            {
               delete *cur;
               _states.erase( cur );
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
         if ( _states.size() )
         {
            const Image& image = (*_states.rbegin())->getImage().image;

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
         for ( States::iterator it = _states.begin(); it != _states.end(); ++it )
            (*it)->handle( event );
      }

      virtual void run()
      {
         _mpr->run();
         for ( States::iterator it = _states.begin(); it != _states.end(); ++it )
            (*it)->run();
      }

      virtual void consume( Order* o )
      {
         _mpr->consume( o );
         for ( States::iterator it = _states.begin(); it != _states.end(); ++it )
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
      States             _states;
      OrderProvider&     _orderProvider;

   public:
      ResourceImageRGB&          slice;
   };
}

#endif