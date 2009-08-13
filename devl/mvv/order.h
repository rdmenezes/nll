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
   class MVV_API OrderResult
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
   enum MVV_API OrderId
   {
      NOOP            /// no operation, nothing is done
   };

   /**
    @ingroup mvv
    @brief Contain orders and their result.
    
    @note Orders in the QueueOrders will be stored by order of creation
          and not by insertion time!
    */
   class MVV_API OrderInterface
   {
   public:
      /**
       @brief Construct an empty order
       @param orderId the id of the type of order
       */
      OrderInterface( OrderId orderId ) : _id( nll::core::IdMaker::instance().generateId() ), _orderId( orderId ), _result( 0 )
      {}

      ~OrderInterface()
      {}

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
      OrderId getOrderId() const
      {
         return _orderId;
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
       @biref returns the result og the current order. Null if not results yet.
       */
      const OrderResult* getResult() const
      {
         return _result;
      }

   protected:
      ui32        _id;
      OrderId     _orderId;
      OrderResult*_result;
   };
}

# pragma warning( pop )

#endif
