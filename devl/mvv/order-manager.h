#ifndef MVV_ORDER_MANAGER_H_
# define MVV_ORDER_MANAGER_H_

# include <set>
# include "order.h"

namespace mvv
{

   /**
    @ingroup mvv
    @brief Manage and digest orders
    */
   class OrderManagerInterface
   {
   public:
      /**
       @brief Construct an order manager
       @param interstedInOrders a set of order in which the manager is able to handle
       */
      OrderManagerInterface( const std::set<OrderId>& interstedInOrders ) : _interested( interstedInOrders )
      {}

      virtual ~OrderManagerInterface()
      {}

      /**
       @brief Consume a command.
       @return true if the manager has correctly handled this order
       */
      virtual bool consume( const OrderInterface* order ) const = 0;

   protected:
      std::set<OrderId>    _interested;
   };

   /**
    @ingroup mvv
    @brief Hold a set of <code>OrderManagerInterface</code>
    */
   class OrderManagers
   {
      typedef std::set<const OrderManagerInterface*>  Managers;

   public:
      /**
       @brief Register a manager. It has to be an allocated pointer as it will be destroyed in the destructor
       */
      void registerManager( const OrderManagerInterface* manager )
      {
         _manager.insert( manager );
      }

      virtual ~OrderManagers()
      {
         for ( Managers::const_iterator it = _manager.begin(); it != _manager.end(); ++it )
            delete *it;
      }

      /**
       @brief Consume the order. All the managers will be run on this command.
       */
      virtual void consume( const OrderInterface* order )
      {
         if ( !order )
            throw ExceptionBadOrder("null order");

         bool consumed = false;
         for ( Managers::const_iterator it = _manager.begin(); it != _manager.end(); ++it )
         {
            const OrderManagerInterface* manager = *it;
            consumed |= manager->consume( order );
         }

         if ( !consumed )
            throw ExceptionOrderNotConsumed( std::string( "the order:" ) + nll::core::val2str( order->getId() ) + std::string( " has not been consumed" ) );
      }

   protected:
      Managers    _manager;
   };
}

#endif
