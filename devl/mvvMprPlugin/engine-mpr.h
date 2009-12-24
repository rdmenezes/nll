#ifndef MVV_MPR_ENGINE_MPR_H_
# define MVV_MPR_ENGINE_MPR_H_

#include <mvvPlatform/engine-order.h>
#include <mvvPlatform/resource-vector.h>
#include <mvvPlatform/resource-volumes.h>
#include <mvvPlatform/resource-typedef.h>

# define MVV_PLATFORM_ORDER_CREATE_SLICE  "MVV_PLATFORM_ORDER_CREATE_SLICE"

namespace mvv
{
namespace platform
{
   enum InterpolationMode
   {
      LINEAR,
      NEAREST
   };

   typedef ResourceValue<InterpolationMode>  ResourceInterpolationMode;

   namespace impl
   {
      class OrderSliceCreatorResult : public OrderResult
      {
      public:
         OrderSliceCreatorResult()
         {
         }
      };

      /**
       @brief Create a slice from position, orientation...

       @note We are copying the data so that the values are not changing while computing the MPR...
       */
      class OrderSliceCreator : public Order
      {
      public:
         OrderSliceCreator( nll::core::vector3f position,
                            nll::core::vector3f dirx,
                            nll::core::vector3f diry,
                            nll::core::vector3f panning,
                            nll::core::vector2f zoom,
                            nll::core::vector2ui size,
                            InterpolationMode interpolation,
                            RefcountedTyped<Volume> volume ) : Order( Order::OrderClassId::create( MVV_PLATFORM_ORDER_CREATE_SLICE ), Order::Predecessors(), true ), _volume( volume ), _position( position ), _dirx( dirx ), _diry( diry ), _panning( panning ), _zoom( zoom ), _size( size ), _interpolation( interpolation )
         {
         }

      protected:
         virtual OrderResult* _compute()
         {
            return new OrderResult();
         }

      protected:
         nll::core::vector3f _position;
         nll::core::vector3f _dirx;
         nll::core::vector3f _diry;
         nll::core::vector3f _panning;
         nll::core::vector2f _zoom;
         nll::core::vector2ui _size;
         InterpolationMode   _interpolation;
         RefcountedTyped<Volume> _volume;
      };

      /**
       @brief computes a MPR for each volume in the list
       */
      class EngineMprSlice : EngineOrder
      {
      public:
         ResourceVolumes      volumes;       /// list of volume to display
         ResourceVector3f     position;      /// position in mm of the camera
         ResourceVector3f     directionx;    /// x-axis of the MPR
         ResourceVector3f     directiony;    /// y-axis of the MPR
         ResourceVector3f     panning;       /// panning position in mm
         ResourceVector2f     zoom;          /// zoom
         ResourceVector2ui    size;          /// size in pixel of the slice to render

         ResourceBool         isInteracting;       /// set to true if MPR is being modified, so we can use less accurate algorithms to speed up the process
         ResourceInterpolationMode interpolation;  /// Interpolation used to produce the slices

      public:
         EngineMprSlice( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher, bool fasterDisplayWhenInteracting ) : EngineOrder( handler, provider, dispatcher ), volumes( storage ), _fasterDisplayWhenInteracting( fasterDisplayWhenInteracting )
         {
            position.connect( this );
            directionx.connect( this );
            directiony.connect( this );
            panning.connect( this );
            zoom.connect( this );
            size.connect( this );
            volumes.connect( this );

            if ( fasterDisplayWhenInteracting )
            {
               isInteracting.connect( this );
            }
            interpolation.connect( this );

            _construct();
         }

         virtual void consume( RefcountedTyped<Order> order )
         {
         }

         virtual const std::set<OrderClassId>& interestedOrder() const
         {
            return _interested;
         }

      protected:
         virtual bool _run()
         {
            std::vector< RefcountedTyped<Order> > orders;
            for ( ResourceVolumes::Iterator it = volumes.begin(); it != volumes.end(); ++it )
            {
               RefcountedTyped<Order> order( new OrderSliceCreator( position.getValue(),
                                                                    directionx.getValue(),
                                                                    directiony.getValue(),
                                                                    panning.getValue(),
                                                                    zoom.getValue(),
                                                                    size.getValue(),
                                                                    interpolation.getValue(),
                                                                    *it ) );
               orders.push_back( order );
               _orderProvider.pushOrder( order );
            }
            _ordersSend = orders;
            return true;
         }

         void _construct()
         {
            _interested.insert( OrderClassId::create( MVV_PLATFORM_ORDER_CREATE_SLICE ) );
         }

      protected:
         bool                    _fasterDisplayWhenInteracting;
         std::set<OrderClassId>  _interested;
         std::vector< RefcountedTyped<Order> >  _ordersSend;
      };
   }

   /**
    @ingroup platform
    @brief Compute a fused MPR of a set of volume
    */
   class EngineMpr : EngineOrder
   {
   public:

   public:
      // public input slots, any change in a slot and the engine needs to be recomputed
      ResourceVector3f     position;      /// position in mm of the camera
      ResourceVector3f     directionx;    /// x-axis of the MPR
      ResourceVector3f     directiony;    /// y-axis of the MPR
      ResourceVector3f     panning;       /// panning position in mm
      ResourceVector2f     zoom;          /// zoom
      ResourceVector2ui    size;          /// size of the slice
      ResourceVolumes      volumes;       /// list of volume to display
      ResourceTransferFunction lut;       /// list of LUTs associated to the volume
      ResourceFloats       intensities;   /// list of intensities associated to the volume

      ResourceBool         isInteracting;       /// set to true if MPR is being modified, so we can use less accurate algorithms to speed up the process
      ResourceInterpolationMode interpolation;  /// Interpolation used to produce the slices
   protected:


   public:
      EngineMpr( ResourceStorageVolumes storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher, bool fasterDisplayWhenInteracting = false ) : EngineOrder( handler, provider, dispatcher ), volumes( storage ), _fasterDisplayWhenInteracting( fasterDisplayWhenInteracting )
      {
         position.connect( this );
         directionx.connect( this );
         directiony.connect( this );
         panning.connect( this );
         zoom.connect( this );
         size.connect( this );
         volumes.connect( this );
         lut.connect( this );
         intensities.connect( this );

         if ( fasterDisplayWhenInteracting )
         {
            // register the change only if activated
            isInteracting.connect( this );
         }
      }

   private:
      bool  _fasterDisplayWhenInteracting;
   };
}
}

#endif