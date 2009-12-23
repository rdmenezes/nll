#ifndef MVV_PLATFORM_TYPES_H_
# define MVV_PLATFORM_TYPES_H_

# include <nll/nll.h>
# include "symbol-typed.h"

namespace mvv
{
   typedef     unsigned char     ui8;
   typedef     char              i8;
   typedef     unsigned short    ui16;
   typedef     short             i16;
   typedef     unsigned long     ui32;
   typedef     long              i32;
   typedef     float             f32;
   typedef     double            f64;

   typedef     nll::core::Image<ui8>   Image;
   typedef     nll::core::Image<f32>   Imagef;

   typedef     nll::imaging::VolumeSpatial<f32> Volume;
   typedef     platform::SymbolTyped<Volume>    SymbolVolume;
}

#endif