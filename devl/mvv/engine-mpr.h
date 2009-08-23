#ifndef MVV_ENGINE_MPR_H_
# define MVV_ENGINE_MPR_H_

# include "dynamic-resource.h"

namespace mvv
{
   /**
    @brief A multiplanar reconstruction object
    */
   class EngineMpr : public Engine
   {
   public:
      /**
       @brief Consume an order
       */
      virtual void consume( Order* )
      {
      }

      /**
       @brief Compute a MPR
       */
      virtual void run()
      {
      }
   };
}

#endif
