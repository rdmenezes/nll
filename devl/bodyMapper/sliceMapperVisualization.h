#ifndef MVV_MAPPER_SLICE_MAPPER_VISUALIZATION_H_
# define MVV_MAPPER_SLICE_MAPPER_VISUALIZATION_H_

#include "dataset.h"

# pragma warning( push )
# pragma warning( disable:4251 ) // dll interface for STL

namespace mvv
{
namespace mapper
{
   class BODYMAPPER_API SliceMapperVisualization
   {
      typedef LandmarkDataset::Volume                      Volume;
      typedef nll::imaging::LookUpTransformWindowingRGB    Lut;
      typedef nll::imaging::Slice<float>                   Slice;
      typedef nll::core::Image<nll::ui8>                   Image;

   public:
      static void createPreview( const LandmarkDataset& data, const std::string outputDir = "../../bodyMapper/data/sliceMapperVolumePreview/" )
      {
         for ( unsigned n = 0; n < data.size(); ++n )
         {
            std::cout << "case=" << n << " out of=" << data.size() << std::endl;

            std::auto_ptr<Volume> v = data.loadData( n );
            ensure( v.get(), "can't load volume:" + data[ n ].fileId );

            Image i = _extractXZ( *v );
            nll::core::extend( i, 3 );
            nll::core::writeBmp( i, outputDir + data[ n ].fileId + ".bmp" );
         }
      }

   private:
      static Image _extractXZ( const Volume& v )
      {
         typedef nll::imaging::Mpr< Volume, nll::imaging::InterpolatorTriLinear<Volume> >  Mpr;

         // extract a centered slice, should have the same size than number of voxels in x/y
         Lut lut( -320, 220, 256 );
         lut.createGreyscale();

         nll::core::vector3f vx( v.getPst()( 0, 0 ), 
                                 v.getPst()( 1, 0 ),
                                 v.getPst()( 2, 0 ) );
         nll::core::vector3f vy( v.getPst()( 0, 2 ), 
                                 v.getPst()( 1, 2 ),
                                 v.getPst()( 2, 2 ) );
         assert( v.getSpacing()[ 0 ] == v.getSpacing()[ 1 ] ); // we are expecting same spacing

         nll::core::vector3f barycentre = nll::imaging::computeBarycentre( v, lut );
         nll::core::vector3f position = v.indexToPosition( nll::core::vector3f( static_cast<float>( v.size()[ 0 ] ) / 2,
                                                                                static_cast<float>( v.size()[ 1 ] ) / 2,
                                                                                static_cast<float>( v.size()[ 2 ] ) / 2 ) );
         nll::ui32 sx1 = static_cast<nll::ui32>( v.size()[ 0 ] * v.getSpacing()[ 0 ] / v.getSpacing()[ 2 ] );
         position[ 1 ] = barycentre[ 1 ];
         Slice slice( nll::core::vector3ui( sx1, v.size()[ 2 ], 3 ),   // we extract a smaller slice => rounding errors, which can create one full row of non zero voxel, depending on the lut...
                      vx,
                      vy,
                      position,
                      nll::core::vector2f( v.getSpacing()[ 2 ], v.getSpacing()[ 2 ] ) );
         Mpr mpr( v );
         mpr.getSlice( slice );

         // convert to greyscale image
         nll::core::Image<nll::ui8> sliceTfm( sx1, v.size()[ 2 ], 1 );
         for ( nll::ui32 y = 0; y < slice.size()[ 1 ]; ++y )
         {
            for ( nll::ui32 x = 0; x < slice.size()[ 0 ]; ++x )
            {
               const float* value = lut.transform( slice( x, y, 0 ) );

               nll::ui8* pos = sliceTfm.point( x, y );
               pos[ 0 ] = static_cast<nll::ui8>( NLL_BOUND( value[ 0 ], 0, 255 ) );
            }
         }
         return sliceTfm;
      }
   };
}
}

# pragma warning( pop )

#endif