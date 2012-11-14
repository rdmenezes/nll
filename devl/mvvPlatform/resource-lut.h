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
      typedef TransferFunction::value_type               value_type;
      typedef nll::imaging::LookUpTransformWindowingRGB  lut_type;

      ResourceLut() : Resource( new impl::ResourceLutImpl( 0, 255 ), true )
      {
         getValue().lut.createGreyscale();
      }

      template <class Volume>
      void detectRange( const Volume& v, double ratioSelection, ui32 nbBins = 256 )
      {
         getValue().lut.detectRange( v, ratioSelection, nbBins );
         notify();
      }

      ResourceLut( const nll::imaging::LookUpTransformWindowingRGB& lut ) : Resource( new impl::ResourceLutImpl( 0, 1 ), true )
      {
         getValue().lut = lut;
      }

      ResourceLut( float minIntensity, float maxIntensity ) : Resource( new impl::ResourceLutImpl( minIntensity, maxIntensity ), true )
      {
         getValue().lut.createGreyscale();
      }

      virtual const TransferFunction::value_type* transform( float inValue ) const
      {
         return getValue().lut.transform( inValue );
      }

      const TransferFunction::value_type* getIndex( ui32 index ) const
      {
         return getValue().lut.get( index );
      }

      void setIndex( ui32 index, const TransferFunction::value_type* val )
      {
         return getValue().lut.set( index, val );
      }

      virtual ui32 nbComponents() const
      {
         return 3;
      }

      void setValue( const nll::imaging::LookUpTransformWindowingRGB& lut )
      {
         getValue().lut = lut;
      }

      ResourceLut* operator&()
      {
         return this;
      }

      ui32 getSize() const
      {
         return getValue().lut.getSize();
      }

      /**
       @brief Create a gaussian spike, useful to display the edges
       @param stdDevRatio the stddev to be used, sepficied relative to the lut size (e.g., 0.2 means the stddev will be 0.2 * lutSize)
       @param meanRatio the mean to be used, sepficied relative to the lut size (e.g., with 0.5, the means will be 0.2 * lutSize)
       */
      void createColorVolcano( const value_type* baseColor, double stdDevRatio = 0.2, double meanRatio = 0.5 )
      {
         getValue().lut.createColorVolcano( baseColor, stdDevRatio, meanRatio );
      }
   };
}
}

#endif
