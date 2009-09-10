#ifndef MVV_ORDER_H_
# define MVV_ORDER_H_

# include "mvv.h"
# include "types.h"
# include "exception.h"
# include <nll/nll.h>

# pragma warning( push )
# pragma warning( disable:4290 ) // conversion from 'const double' to XXX, possible loss of data

namespace mvv
{
   /**
    @ingroup mvv
    @brief base class for order results. All results must inherit this class.
    */
   class OrderResult
   {
   public:
      virtual ~OrderResult()
      {}
   };

   /**
    @ingroup mvv
    @brief class that binds and OrderId to a derived OrderResult class

    This default implementation will create an errors. It means the orderId given is not currently
    handled use the macro <code>MVV_BIND_ORDER_RESULT</code> to generate the binding.
    */
   template <int>
   struct BindOrderResultType
   {
      typedef void   value_type;
   };

/**
 @ingroup mvv
 @brief register on OrderId to a redived class of OrderResult
 */
# define MVV_BIND_ORDER_RESULT( OrderId1, OrderResultDerived ) \
   template <>                                                 \
   struct BindOrderResultType<(int)OrderId1>                   \
   {                                                           \
      enum                                                     \
      {                                                        \
         value = nll::core::IsDerivedFrom<OrderResultDerived,  \
                                          OrderResult>::value  \
      };                                                       \
      STATIC_ASSERT( value );                                  \
      typedef OrderResultDerived value_type;                   \
   }




   /**
    @ingroup mvv
    @brief defines an order type that will be used to recognize this type
    */
   enum OrderClassId
   {
      ORDER_NULL,                   /// no operation, nothing is done
      ORDER_MPR_RENDERING,          /// this order will render a reformated slice of a volume, see OrderMprRendering class
      ORDER_MPR_RENDERING_COMBINE   /// combine the slices of a MPR
   };

   /**
    @ingroup mvv
    @brief Contain orders and their result.
    
    @note Orders in the QueueOrders will be stored by order of creation
          and not by insertion time!
    */
   class Order
   {
   public:
      typedef std::set<ui32>  Predecessors;

   public:
      /**
       @brief Construct an empty order
       @param orderClassId the id of the type of order
       @param toBeMultithreaded true if the order must be run on another thread. It should be so, only if
              the order is computationally intensive so that the OrderThread doesn't have to wait for the
              end of this order. Example: rendering of a volume...
       @param predecessors all the predecessor ids to be run and completed before starting this order. Example
              in to render a volume, it must have already been in memory!
       */
      Order( OrderClassId orderClassId,
             bool toBeMultiThreaded,
             const Predecessors& predecessors ) 
         : _id( nll::core::IdMaker::instance().generateId() ), _orderClassId( orderClassId ),
           _toBeMultiThreaded( toBeMultiThreaded ), _predecessors( predecessors ), _result( 0 )
      {}

      virtual ~Order()
      {}

      /**
       @brief Run the order and return a result. (_result will be automatically set externally)
       */
      virtual OrderResult* run() = 0;

      /**
       @return the order instance id
       */
      ui32 getId() const
      {
         return _id;
      }

      /**
       @return the id of the kind of order
       */
      OrderClassId getOrderClassId() const
      {
         return _orderClassId;
      }

      /**
       @brief Set the result of an order. There may not already have a result and it may not be null.
              When it is not used anymore, <code>result</code> will be _deallocated_!
       */
      void setResult( OrderResult* result ) throw (ExceptionBadOrder)
      {
         if ( !result || _result )
            throw ExceptionBadOrder( "Error in setting an order result" );
         _result = result;
      }

      /**
       @brief returns the result og the current order. Null if not results yet.
       */
      const OrderResult* getResult() const
      {
         return _result;
      }

      /**
       @brief returns the predecessor of an order. The order cannot be executed if the predecessors have not
              been computed before
       */
      const Predecessors& getPredecessors() const
      {
         return _predecessors;
      }

      /**
       @return true if this order must be launched on a different thread.
       */
      bool toBeMultithreaded() const
      {
         return _toBeMultiThreaded;
      }

   protected:
      ui32           _id;
      OrderClassId   _orderClassId;
      OrderResult*   _result;
      bool           _toBeMultiThreaded;
      Predecessors   _predecessors;
   };
}

# pragma warning( pop )

#endif
