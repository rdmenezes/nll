#ifndef MVV_PLATFORM_TRANSFER_FUNCTION_H_
# define MVV_PLATFORM_TRANSFER_FUNCTION_H_

# include "mvvPlatform.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Transform a real value to a RGB{A} value
    */
   class MVVPLATFORM_API TransferFunction
   {
   public:
      typedef f32    value_type;

      virtual ~TransferFunction()
      {}

      /**
       @brief Transform a real value to a RGB value
       @param inValue the input value
       
       */
      virtual const value_type* transform( float inValue ) const = 0;

      /**
       @brief returns the number of components returned by the transfer function
       */
      virtual ui32 nbComponents() const = 0;
   };
}
}

#endif