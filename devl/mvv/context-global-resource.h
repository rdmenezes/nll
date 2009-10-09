#ifndef MVV_CONTEXT_GLOBAL_RESOURCE_H_
# define MVV_CONTEXT_GLOBAL_RESOURCE_H_

namespace mvv
{
   /**
    @brief Holds all the shared resources

    All order creators should be referenced here so there is only 1 access point to create/receive all orders.
    */
   class ContextGlobalResource : public ContextInstance, public OrderCreator
   {
      typedef std::map<Symbol, MedicalVolume*>  Volumes;
      typedef std::set<OrderCreator*>           OrderCreators;
      typedef std::vector<Order*>              Orders;

   public:
      ~ContextGlobalResource()
      {
         for ( Volumes::iterator it = _volumes.begin(); it != _volumes.end(); ++it )
            removeVolume( it->first );
      }

      /**
       @brief Add a volume
       @param name must be unique
       @param vol must be an allocated pointer. It will be freed with the destruction of this context.
       */
      void addVolume( const Symbol& name, MedicalVolume* vol )
      {
         ensure( vol, "can't be null" );
         _volumes[ name ] = vol;
      }

      /**
       @brief Remove a volume. All other contexts should have this volume removed too.
       */
      void removeVolume( const Symbol& name )
      {
         Volumes::iterator it = _volumes.find( name );
         if ( it != _volumes.end() )
         {
            // get the pointer
            MedicalVolume* vol = it->second;
            _volumes.erase( it );

            // clear the other contexts
            ContextMpr* contextMpr = Context::instance().get<ContextMpr>();
            if ( contextMpr )
            {
               for ( ContextMpr::Mprs::iterator it = contextMpr->begin(); it != contextMpr->end(); ++it )
               {
                  it->second->removeVolume( vol );
               }
            }
         }
      }

      /**
       @brief Add an object that handle asynchronous orders
       */
      void addOrderCreator( OrderCreator* o )
      {
         ensure( o, "should not be null" );
         _orderCreators.insert( o );
      }

      /**
       @brief Remove an order creator.
       */
      void removeOrderCreator( OrderCreator* o )
      {
         ensure( o, "should not be null" );
         OrderCreators::iterator it = _orderCreators.find( o );
         if ( it != _orderCreators.end() )
            _orderCreators.erase( o );
      }

      virtual void consume( const Orders& o )
      {
         for ( Orders::const_iterator it = o.begin(); it != o.end(); ++it )
            consume( *it );
      }

      /**
       @brief Consume an order. This order will be consumed by all order creators
       */
      virtual void consume( Order* o )
      {
         for ( OrderCreators::iterator it = _orderCreators.begin(); it != _orderCreators.end(); ++it )
            ( *it )->consume( o );
      }

      /**
       @brief Run cycle.
       */
      virtual void run()
      {
         for ( OrderCreators::iterator it = _orderCreators.begin(); it != _orderCreators.end(); ++it )
            ( *it )->run();
      }

   private:
      Volumes        _volumes;
      OrderCreators  _orderCreators;
   };
}

#endif