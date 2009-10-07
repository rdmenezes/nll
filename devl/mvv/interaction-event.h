#ifndef MVV_INTERACTION_EVENT_H_
# define MVV_INTERACTION_EVENT_H_

namespace mvv
{
   /**
    @brief Event for keyboard/mouse action
    */
   struct InteractionEvent
   {
   };

   /**
    @brief Notifies the derived class an event occured
    */
   class InteractionEventReceiver
   {
   public:
      virtual ~InteractionEventReceiver()
      {}

      virtual void handle( const InteractionEvent& event ) = 0;
   };
}

#endif
