#ifndef MVV_PLATFORM_LAYOUT_TOOL_H_
# define MVV_PLATFORM_LAYOUT_TOOL_H_

# include <mvvPlatform/event-mouse-receiver.h>
# include <mvvPlatform/linkable.h>
# include <mvvPlatform/engine-order.h>
# include "mvvMprPlugin.h"
# include "types.h"

namespace mvv
{
namespace platform
{
   // forward declaration
   class Mip;

   /**
    @ingroup platform
    @brief Tools that can be plugged on a Mip
    */
   class MVVMPRPLUGIN_API MipTool : public LinkableDouble< Mip*, MipTool* >
   {
   public:
      MipTool( bool canModifyImage ) : _canModify( canModifyImage )
      {
      }

      virtual ~MipTool();

      /**
       @brief Returns true is this tool is modifying the segment (add layouts...). Else modification
              on the MPR are not allowed and <code>updateSegment</code> won't be called
       */
      bool isModifyingMprImage() const
      {
         return _canModify;
      }

      /**
       @brief Returns true is this tool need to save the current segment: when the tool is updated,
              and if no other resources have change, then we don't need to recompute all the segment,
              just start at the point we saved previously
       */
      virtual bool isSavingMprImage() const
      {
         return false;
      }

      /**
       @brief Tools are sorted according to their priority from highest to lowest
              they are displayed from lowest->highest
              received events from highest->lowest
       */
      virtual f32 priority() const
      {
         return 0;
      }

      /**
       @brief action need to be run when the input slice changed.
       */
      virtual void updateMip( ResourceSliceuc mip, Mip& holder ) = 0;

      /**
       @param sender the mip where the signal originate's from
       @param event the event
       @paran windowOrigin the origin of the layout so we can compute the actual position of the mouse relative to the layout
       */
      virtual void receive( Mip& sender, const EventMouse& event, const nll::core::vector2ui& windowOrigin ) = 0;

      /**
       @brief if true, the event will not be propagated to other tools
       */
      virtual bool interceptEvent() const
      {
         return false;
      }

      virtual void connect( Mip* m );

      virtual void disconnect( Mip* m );

   private:
      // disabled copy
      MipTool& operator=( const MipTool& );
      MipTool( const MipTool& );

   protected:
      bool                    _canModify;
   };
}
}

#endif
