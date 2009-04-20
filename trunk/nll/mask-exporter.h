#ifndef NLL_MASK_EXPORTER_H_
# define NLL_MASK_EXPORTER_H_

namespace nll
{
namespace utility
{
   /**
    @ingroup utility
    @brief export a mask using random colors for each ID
    */
   template <class T, class Mapper> void maskExport( const core::ImageMask& mask, core::Image<T, Mapper>& out )
   {
      ui32 randColor[ 3 ];
      ui32 randDev[ 3 ];

      randColor[ 0 ] = ( rand() + 1) % 256;
      randColor[ 1 ] = ( rand() + 1) % 256;
      randColor[ 2 ] = ( rand() + 1) % 256;

      randDev[ 0 ] = ( rand() + 1) % 256;
      randDev[ 1 ] = ( rand() + 1) % 256;
      randDev[ 2 ] = ( rand() + 1) % 256;

      core::Image<T, Mapper> i( mask.sizex(), mask.sizey(), 3 );
      for ( ui32 y = 0; y < mask.sizey(); ++y )
         for ( ui32 x = 0; x < mask.sizex(); ++x )
            for ( ui32 c = 0; c < 3; ++c )
               i( x, y, c ) = static_cast<T>( ( randColor[ c ] * mask( x, y, 0 ) + randDev[ c ] ) % 256 );
      out = i;
   }
}
}

#endif
