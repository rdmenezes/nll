#ifndef MVV_INTERACTION_EVENT_H_
# define MVV_INTERACTION_EVENT_H_

namespace mvv
{
   /**
    @brief Event for keyboard/mouse action
    */
   struct InteractionEvent
   {
      /// the current mouse position
      nll::core::vector2ui    mousePosition;

      /// the position where the first left unreleased click occured
      nll::core::vector2ui    mouseLeftClickedPosition;

      /// the position where first unreleased
      nll::core::vector2ui    mouseLeftReleasedPosition;

      /// set if left button is pressed
      bool                    isMouseLeftButtonPressed;

      /// set if right button is pressed
      bool                    isMouseRightButtonPressed;
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
