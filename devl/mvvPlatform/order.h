#ifndef MVV_PLATFORM_ORDER_H_
# define MVV_PLATFORM_ORDER_H_

# include "symbol-typed.h"
# include "mvvPlatform.h"
# include "refcounted.h"
# include "types.h"

# pragma warning( push )
# pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief A tag to specify the derived class will be a result computed by an Order.
    */
   class MVVPLATFORM_API OrderResult
   {
   public:
      virtual ~OrderResult()
      {}
   };

   /**
    @ingroup platform
    @brief Returns a mean to wait for an operation to be completed
    @note the future must be created in the current thread, and unlock() called when the order result is to be dispatched
    */
   class MVVPLATFORM_API Future
   {
   public:
      void wait()
      {
         // just try to aquire the lock, if succeed, then the order has finished
         boost::mutex::scoped_lock lock( _mutex );
      }

      Future()
      {
         _mutex.lock();
      }

      /**
       @brief should not be used user
       */
      void _unlock()
      {
         _mutex.unlock();
      }



   private:
      // disabled copy
      Future( const Future& );
      Future& operator=( Future& );

   private:
      boost::mutex _mutex;
   };

   /**
    @ingroup platform
    @brief Defines an order that can be run synchronously/asynchronously
    @note internally an order should not use resources as they are refcounted
          and can cause some problems if the orders are multithreaded...
    */
   class MVVPLATFORM_API Order
   {
   public:
      typedef SymbolTyped<Order>                       OrderClassId;  /// specify what category of order it is
      typedef std::set< Order* >                       Predecessors;  /// predecessors that must be run and finished before this order starts

      /**
       @brief Construct an order
       @param classId a class ID necessary to dispatch the orders
       @param predecessors all orders that must be completed before this ordr is started. All predecessors must remain
              alive until this order is executed
       */
      Order( OrderClassId classId, const Predecessors& predecessors, bool willBeMultithreaded = true ) : _classId( classId ), _predecessors( predecessors ), _multithreaded( willBeMultithreaded ), _result( 0 )
      {
         static ui32 orderId = 0;
         _orderId = ++orderId;
      }

      ui32 getId() const
      {
         return _orderId;
      }

      virtual ~Order()
      {
         delete _result;
      }   

      /**
       @brief Update the result of the order
       @param r the result
       @param own if true, this result will be deleted when this order is destroyed
       */
      void setResult( OrderResult* r )
      {
         _result = r;
      }

      /**
       @brief returns the list of predecessors
       */
      const Predecessors& getPredecessors() const
      {
         return _predecessors;
      }

      /**
       @brief returns the class ID of this order
       */
      OrderClassId getClassId() const
      {
         return _classId;
      }

      /**
       @brief launch the computation
       */
      void compute()
      {
         setResult( _compute() );
      }

      /**
       @brief return the result. Null if not yet computed
       */
      OrderResult* getResult() const
      {
         return _result;
      }

      /**
       @brief return true if it needs to be multithreaded
       */
      bool toBeMultithreaded() const
      {
         return _multithreaded;
      }

      /**
       @brief returns a reference on the furture of this order
       */
      RefcountedTyped<Future>& getFuture()
      {
         while ( !_future.getDataPtr() )
            ;
         return _future;
      }

      // should not be used by user
      void _setFuture( RefcountedTyped<Future> f )
      {
         _future = f;
      }

   protected:
      /**
       @brief Compute the order. It must return an OrderResult that has been allocated
       */
      virtual OrderResult* _compute() = 0;

   protected:
      OrderClassId   _classId;
      Predecessors   _predecessors;
      bool           _multithreaded;
      OrderResult*   _result;
      ui32           _orderId;
      RefcountedTyped<Future> _future;
   };
}
}

# pragma warning( pop )
#endif
