#ifndef MVV_PLATFORM_NOTIFIABLE_H_
# define MVV_PLATFORM_NOTIFIABLE_H_

# include "mvvPlatform.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Simple interface to make a class notifiable (i.e. needs to update its internal state)
    */
   class MVVPLATFORM_API Notifiable
   {
   public:
      virtual void notify() = 0;
   };
}
}

#endif