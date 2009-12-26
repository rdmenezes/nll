#ifndef MVV_PLATFORM_RESOURCE_LUT_H_
# define MVV_PLATFORM_RESOURCE_LUT_H_

# include "resource-value.h"
# include "transfer-function.h"
# include <nll/nll.h>

namespace mvv
{
namespace platform
{
   namespace impl
   {
      struct ResourceLutImpl
      {
         ResourceLutImpl( float minIntensity, float maxIntensity ) : lut( minIntensity, maxIntensity, 256, 3 )
         {}

         nll::imaging::LookUpTransformWindowingRGB    lut;
      };
   }

   class MVVPLATFORM_API ResourceLut : public Resource<impl::ResourceLutImpl>, public TransferFunction
   {
   public:
      ResourceLut() : Resource( 0 )
      {
      }

      ResourceLut( float minIntensity, float maxIntensity ) : Resource( new impl::ResourceLutImpl( minIntensity, maxIntensity ), true )
      {
      }

      virtual const TransferFunction::value_type* transform( float inValue ) const
      {
         return getValue().lut.transform( inValue );
      }

      virtual ui32 nbComponents() const
      {
         return 3;
      }
   };
}
}

#endif
