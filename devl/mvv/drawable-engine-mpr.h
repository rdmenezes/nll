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


   /**
    @brief MPR toolkit

    Define specific behaviour & tools for a MPR

    Toolkits are allowed to create & handle orders
    */
   class MVV_API MprToolkit
   {
   public:
      typedef std::set<DynamicResource*>     Resources;
      typedef std::set<DrawableMprToolkits*> Mprs;

      virtual ~MprToolkit();

      /**
       @brief Returns the resources that will force an update of the DrawableMprToolkits
              (example: the mouse move the target. If the target is moved, the mpr must be updated,
                        consequently the target position is a resouce of the DrawableMprToolkits).
       */
      Resources& getResources()
      {
         return _resources;
      }

      /**
       This is used to consume orders. 
       */
      virtual void consume( Order* )
      {
      }

      /**
       This method is called each time the MPR slice has been modified
       @param source the source where the toolkit must be run
       @param mpr the slice. If null, this means this toolkit is not supposed to modify a slice (else override isMprMustBeSaved)
       @return true if the toolkit must be run at the next checkpoint
       */
      virtual bool run( DrawableMprToolkits& , ResourceSliceRGB* )
      {
         return true;
      }

      /**
       @brief If an event have been received by a drawable toolkits and this toolkit has been attached to it,
              it must be handled here. Incoming Source & slice are provided
       @param mpr the slice. If null, this means this toolkit is not supposed to modify a slice (else override isMprMustBeSaved)
       */
      virtual void handle( const InteractionEvent& event, DrawableMprToolkits& source, ResourceSliceRGB* mpr ) = 0;

      /**
       @brief We need to know what DrawableMprToolkits is attached to a Mpr toolkit.

       Example: toolkit to move the MPR position: event is received, a new MPR position is calculated,
                the toolkit must update the position of all the other MPRs attached
       */
      virtual void attach( DrawableMprToolkits* r )
      {
         _mprs.insert( r );
      }

      /**
       @brief detach a DrawableMprToolkits
       */
      virtual void detach( DrawableMprToolkits* r )
      {
         Mprs::iterator it = _mprs.find( r );
         if ( it != _mprs.end() )
            _mprs.erase( it );
      }

      /**
       @brief By default the slice within a DrawableMprToolkits can't be modified by a toolkit (for memory usage)
              Override this method and return true if the toolkit will modify the slice. In addition, the
              correct resource must be provided by <code>getResources</code> to trigger an update on the
              DrawableMprToolkits object (else changes won't update the drawed slice).

       */
      virtual bool isMprMustBeSaved()
      {
         return false;
      }

   protected:
      Resources   _resources;
      Mprs        _mprs;
   };



   /**
    @brief Handle movement & scaling of the MPR
    */
   class MVV_API MprToolkitMove : public MprToolkit
   {
   public:
      MprToolkitMove()
      {
         _isLeftCurrentlyPressed = false;
         _isRightCurrentlyPressed = false;
         _zoom = nll::core::vector2f( 1, 1 );
         _position = nll::core::vector3f( 0, 0, 0 );
      }

      /**
       @brief Handle is called each time an event is received
       */
      virtual void handle( const InteractionEvent& event, DrawableMprToolkits& source, ResourceSliceRGB* mpr );

      /**
       @brief Special attach: when attached for the first time, the MPR is centered on the biggest attached volume
       */
      virtual void attach( DrawableMprToolkits* r );

      /**
       @brief We don't want to save the state of this slice as it doesn't modify it
       */
      virtual bool isMprMustBeSaved()
      {
         return false;
      }

   protected:
      /// update all the attached MPR with new position & zoom
      void _updateMprs();

   protected:
      nll::core::vector3f  _position;
      nll::core::vector2f  _zoom;

      bool                 _isLeftCurrentlyPressed;
      bool                 _isRightCurrentlyPressed;
      nll::core::vector3f  _initialOrigin;
      nll::core::vector2f  _initialZoom;
      nll::core::vector2i  _initialMousePos;
   };

   class MVV_API MprToolkitPoint : public MprToolkit
   {
   public:
      MprToolkitPoint()
      {
         _isLeftCurrentlyPressed = false;
         _initialOrigin = nll::core::vector3f( 0, 0, 0 );
         _resources.insert( &_trigger );
      }

      virtual void handle( const InteractionEvent& , DrawableMprToolkits& , ResourceSliceRGB*  )
      {
      }

      virtual bool isMprMustBeSaved()
      {
         return true;
      }

      virtual bool run( DrawableMprToolkits& , ResourceSliceRGB* );

      virtual void attach( DrawableMprToolkits* r );

   protected:
      bool                 _isLeftCurrentlyPressed;
      nll::core::vector3f  _initialOrigin;
      nll::core::vector2i  _initialMousePos;

      DynamicResource      _trigger;
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
         MprState( MprToolkit& mprToolkit, DrawableMprToolkits& toolkits, OrderProvider& orderProvider, ResourceSliceRGB& mpr, bool createMprCopy = false ) :
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
            _mprToolkit.handle( event, _source, _isMprSaved ? &_img : 0 );
         }

         virtual void consume( Order* o )
         {
            _mprToolkit.consume( o );
         }

         virtual bool _run()
         {
            if ( _isMprSaved )
            {
               _img.slice.clone( _previous.slice );
            }
            return _mprToolkit.run( _source, _isMprSaved ? &_img : 0 );
         }

         virtual ResourceSliceRGB& getImage()
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
         MprState& operator=( const MprState& );
         MprState( const MprState& );

      private:
         ResourceSliceRGB        _img;
         DrawableMprToolkits&    _source;
         OrderProvider&          _orderProvider;
         ResourceSliceRGB&       _previous;
         bool                    _isMprSaved;
         MprToolkit&             _mprToolkit;
      };
      typedef std::vector<MprState*>  States;

   public:
      DrawableMprToolkits( OrderProvider& orderProviderv,
                           ResourceVolumes& volumesv,
                           ResourceVector3f& originv,
                           ResourceVector3f& vector1v,
                           ResourceVector3f& vector2v,
                           ResourceVector2f& zoomv,
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
         ResourceSliceRGB* outputMpr;
         if ( !_states.size() )
            outputMpr = &_mpr->outFusedMpr;
         else
            outputMpr = &(*_states.rbegin())->getImage();

         _states.push_back( new MprState( toolkit, *this, _orderProvider, *outputMpr, toolkit.isMprMustBeSaved() ) );
      }

      /**
       @brief remove a toolkit from this MPR. Its associated state will be destroyed
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
            const ResourceSliceRGB::Slice& slice = (*_states.rbegin())->getImage().slice;

            _tmpImage.clear();
            if ( slice.size()[ 0 ] != renderingSize[ 0 ] || 
                 slice.size()[ 1 ] != renderingSize[ 1 ] )
            {
               std::cout << "Warning: resize" << std::endl;
               _tmpImage.clone( slice.getStorage() );
               nll::core::rescaleBilinear( _tmpImage, renderingSize[ 0 ], renderingSize[ 1 ] );
               return _tmpImage;
            }
            return slice.getStorage();
         }

         // else just pick the raw MPR slice
         if ( _mpr->outFusedMpr.slice.size()[ 0 ] != renderingSize[ 0 ] || _mpr->outFusedMpr.slice.size()[ 0 ] != renderingSize[ 1 ] )
         {
            // we need to rescale for this frame the current MPR as the size is different (asynchrone results)
            // it will be correctly updated later...
            nll::core::rescaleBilinear( _mpr->outFusedMpr.slice.getStorage(), renderingSize[ 0 ], renderingSize[ 1 ] );
         }
         return _mpr->outFusedMpr.slice.getStorage();
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
      ResourceVector3f&          origin;
      ResourceVector3f&          vector1;
      ResourceVector3f&          vector2;
      ResourceVector2f&          zoom;
      ResourceVolumeIntensities& intensities;
      ResourceLuts&              luts;

   protected:
      EngineMprImpl*     _mpr;
      States             _states;
      OrderProvider&     _orderProvider;

      /// this image is a temporary save in case we need to resample it
      Image              _tmpImage;

   public:
      ResourceSliceRGB&          slice;
   };
}

#endif