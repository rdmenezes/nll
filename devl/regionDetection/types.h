#ifndef RD_TYPES_H_
# define RD_TYPES_H_

# include <nll/nll.h>

namespace nll
{
   namespace detect
   {
      typedef imaging::VolumeSpatial<float>         Volume;
      typedef imaging::LookUpTransformWindowingRGB  Lut;
      typedef imaging::Slice<float>                 Slice;
   }
}

#endif