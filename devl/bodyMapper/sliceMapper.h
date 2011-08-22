#ifndef MVV_SLICE_MAPPER_H_
# define MVV_SLICE_MAPPER_H_

# include "def.h"
# include "dataset.h"

namespace mvv
{
namespace mapper
{
   struct BODYMAPPER_API SliceMapperParameters
   {
      SliceMapperParameters()
      {
         preprocessSizeX = 66;
         preprocessSizeY = 44;

         lutMin = -100;
         lutMax = 250;
      }

      void write( std::ostream& o ) const
      {
         nll::core::write<unsigned>( preprocessSizeX, o );
         nll::core::write<unsigned>( preprocessSizeY, o );
         nll::core::write<float>( lutMin, o );
         nll::core::write<float>( lutMax, o );
      }

      void read( std::istream& i )
      {
         nll::core::read( preprocessSizeX, i );
         nll::core::read( preprocessSizeY, i );
         nll::core::read( lutMin, i );
         nll::core::read( lutMax, i );
      }

      unsigned preprocessSizeX;     // the number of pixels the slice will be resampled to
      unsigned preprocessSizeY;     // the number of pixels the slice will be resampled to
      float lutMin;                 // the LUT min value
      float lutMax;                 // the LUT max value
   };

   /**
    @brief Find specific CT slices
    */
   class BODYMAPPER_API SliceMapper
   {
   public:
      typedef nll::core::Buffer1D<double>                      Point;
      typedef nll::core::ClassificationSample<Point, unsigned> Sample;
      typedef nll::core::Database<Sample>                      Database;
      typedef nll::core::Image<unsigned char>                  Image;
      typedef nll::core::Image<float>                          Imagef;
      typedef nll::imaging::LookUpTransformWindowingRGB        Lut;
      typedef LandmarkDataset::Volume                          Volume;

      SliceMapper( const SliceMapperParameters params = SliceMapperParameters() ) : _params( params ), _lut( params.lutMin, params.lutMax, 256, 1 ),
                                                                                                       _lutMask( -440, 320, 256, 1 )
      {
         _lut.createGreyscale();
         _lutMask.createGreyscale();
      }

      // preprocess the slice (i.e. LUT transform the image, extract biggest CC, cropping, center, resample...)
      Image preprocessSlice( const Volume& volume, unsigned slice );
      
      // create a database with all the volumes already preprocessed and export it
      void createPreprocessedDatabase( const std::string& exportPath, bool exportImages = false ) const;

   private:
      SliceMapperParameters   _params;
      Lut                     _lut;
      Lut                     _lutMask;
   };
}
}

#endif