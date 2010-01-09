#ifndef MVV_PLATFORM_TYPES2_H_
# define MVV_PLATFORM_TYPES2_H_

# include <nll/nll.h>
# include <mvvPlatform/resource-value.h>

namespace mvv
{
namespace platform
{
   typedef nll::imaging::Slice<float>     Slice;
   typedef nll::imaging::Slice<nll::ui8>  Sliceuc;

   /// specicalization to avoid equality testing
   template <>
   class ResourceValue<Sliceuc> : public Resource<Sliceuc>
   {
   public:
      ResourceValue() : Resource( new Sliceuc(), true )
      {
      }

      void setValue( Sliceuc val )
      {
         // we don't check for equality as potentially, it can be very long, so instead we prefere to notify
         Resource<Sliceuc>::getValue() = val;
         notify();
      }

      Sliceuc getValue() const
      {
         return Resource<Sliceuc>::getValue();
      }
   };

   /// hold a RGB slice
   typedef ResourceValue<Sliceuc>   ResourceSliceuc;
}
}

#endif