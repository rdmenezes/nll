#ifndef NLL_IMAGING_SLICE_FUSION_H_
# define NLL_IMAGING_SLICE_FUSION_H_

namespace nll
{
namespace imaging
{
   template <class T, class Mapper, class OUT, class LutMapper>
   struct BlendSliceInfo
   {
      typedef core::Image<T, Mapper>            Slice;
      typedef LookUpTransform<OUT, LutMapper>   Lut;

      core::Image<T, Mapper>  slice;
      float                   blendFactor;
      Lut                     lut;
   };


   template <class T, class Mapper, class OUT, class OUTMapper>
   void blend( const BlendSliceInfo< T, Mapper, >& sliceInfos,
               core::image<OUT, OUTMapper>& out )
   {
   }
}
}

#endif