#ifndef MVV_PLATFORM_TYPES2_H_
# define MVV_PLATFORM_TYPES2_H_

# include <nll/nll.h>

namespace mvv
{
namespace platform
{
   typedef nll::imaging::Slice<float>     Slice;
   typedef nll::imaging::Slice<nll::ui8>  Sliceuc;

   /// hold a RGB image
   typedef ResourceValue<Sliceuc>   ResourceSliceuc;
}
}

#endif