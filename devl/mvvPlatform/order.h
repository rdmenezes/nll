#ifndef MVV_PLATFORM_ORDER_H_
# define MVV_PLATFORM_ORDER_H_

# include "symbol-typed.h"
# include "mvvPlatform.h"
# include "refcounted.h"

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
   };

   /**
    @ingroup platform
    @brief Defines an order that can be run synchronously/asynchronously
    */
   class MVVPLATFORM_API Order : public RefcountedTyped<OrderResult*>
   {
      typedef RefcountedTyped<OrderResult*> Base;
   public:
      typedef SymbolTyped<Order>    OrderClassId;  /// specify what category of order it is
      typedef std::set<Order>       Predecessors;  /// predecessors that must be run and finished before this order starts

      /**
       @brief Construct an order
       @param classId a class ID necessary to dispatch the orders
       @param predecessors all orders that must be completed before this ordr is started. All predecessors must remain
              alive until this order is executed
       */
      Order( OrderClassId classId, const Predecessors& predecessors, bool willBeMultithreaded = true ) : _classId( classId ), _predecessors( predecessors ), _multithreaded( willBeMultithreaded )
      {}

      virtual ~Order()
      {}

      /**
       @brief Update the result of the order
       @param r the result
       @param own if true, this result will be deleted when this order is destroyed
       */
      void setResult( OrderResult* r, bool own )
      {
         _data->own = own;
         getData() = r;
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

   protected:
      OrderClassId   _classId;
      Predecessors   _predecessors;
      bool           _multithreaded;
   };
}
}

#endif
