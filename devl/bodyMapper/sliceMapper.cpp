#include "sliceMapper.h"

using namespace nll;

namespace mvv
{
namespace mapper
{
   nll::core::vector2i getExtent( const SliceMapper::Image& slice, const nll::core::vector2i& center, double ratiox, double ratioy, const std::vector<unsigned>& linex, const std::vector<unsigned>& liney, double totalPixels )
   {
      nll::core::vector2i extent;
      double nbPixels = 0;
      int n = 0;
      for ( ; n < slice.sizex() / 2; ++n )
      {
         int x1 = (int)center[ 0 ] + n + 1;
         int x2 = (int)center[ 0 ] - n;
         if ( x1 + 1 >= slice.sizex() || x2 <= 0 )
            break;
         nbPixels += liney[ x1 ] + liney[ x2 ];
         const double ratio = nbPixels / totalPixels;
         if ( ratio >= ratiox )
            break;
      }
      extent[ 0 ] = n;

      nbPixels = 0;
      n = 0;
      for ( ; n < slice.sizey() / 2; ++n )
      {
         int y1 = (int)center[ 1 ] + n + 1;
         int y2 = (int)center[ 1 ] - n;
         if ( y1 + 1 >= slice.sizey() || y2 <= 0 )
            break;
         nbPixels += linex[ y1 ] + linex[ y2 ];
         const double ratio = nbPixels / totalPixels;
         if ( ratio >= ratiox )
            break;
      }
      extent[ 1 ] = n;
      return extent;
   }

   SliceMapper::Image SliceMapper::preprocessSlice( const SliceMapper::Volume& volume, unsigned sliceNumber )
   {
      // get the LUT transformed slice and compute a barycenter, compute the sums for each vertical/horizontal line
      typedef nll::imaging::Slice<unsigned char> Slice;
      Slice sliceReal( nll::core::vector3ui( volume.getSize()[ 0 ], volume.getSize()[ 1 ], 1 ),
                       nll::core::vector3f( 1, 0, 0 ),
                       nll::core::vector3f( 0, 1, 0 ),
                       nll::core::vector3f( volume.getSize()[ 0 ] / 2, volume.getSize()[ 1 ] / 2, 0 ),
                       nll::core::vector2f( 1, 1 ) );
      Image& slice = sliceReal.getStorage();
      Image sliceMask( volume.getSize()[ 0 ], volume.getSize()[ 1 ], 1, false );

      double dx = 0;
      double dy = 0;
      unsigned nbPoints = 0;
      std::vector<unsigned> lineSumX( slice.sizey() );
      std::vector<unsigned> lineSumY( slice.sizex() );
      for ( ui32 y = 0; y < slice.sizey(); ++y )
      {
         unsigned sumx = 0;
         for ( ui32 x = 0; x < slice.sizex(); ++x )
         {
            const float volVal = volume( x, y, sliceNumber );
            const float val = _lut.transform( volVal )[ 0 ];
            const float valMask = _lutMask.transform( volVal )[ 0 ];
            slice( x, y, 0 ) = val;
            sliceMask( x, y, 0 ) = valMask;

            if ( /*valMask < 230 &&*/ valMask > 10 )
            {
               dx += x;
               dy += y;
               ++nbPoints;
               ++sumx;
               ++lineSumY[ x ];
            }
         }
         lineSumX[ y ] = sumx;
      }


      const nll::core::vector2f center( dx / nbPoints, dy / nbPoints ); 

      // compute the distance we need to crop, retain X% in a direction from the center
      nll::core::vector2i extent = getExtent( sliceMask, nll::core::vector2i( center[ 0 ], center[ 1 ] ), 0.85, 0.85, lineSumX, lineSumY, nbPoints );
      const nll::core::vector2f centerI( slice.sizex() / 2 - center[ 0 ],
                                         slice.sizey() / 2 - center[ 1 ] );

      for ( int n = 0; n < 3; ++n )
      {
         slice( center[ 0 ] + n - extent[ 0 ], center[ 1 ]     - extent[ 1 ], 0 ) = 255;
         slice( center[ 0 ] - n - extent[ 0 ], center[ 1 ]     - extent[ 1 ], 0 ) = 255;
         slice( center[ 0 ]     - extent[ 0 ], center[ 1 ] + n - extent[ 1 ], 0 ) = 255;
         slice( center[ 0 ]     - extent[ 0 ], center[ 1 ] - n - extent[ 1 ], 0 ) = 255;
      }

      for ( int n = 0; n < 3; ++n )
      {
         slice( center[ 0 ] + n , center[ 1 ]     , 0 ) = 255;
         slice( center[ 0 ] - n , center[ 1 ]     , 0 ) = 255;
         slice( center[ 0 ]     , center[ 1 ] + n , 0 ) = 255;
         slice( center[ 0 ]     , center[ 1 ] - n , 0 ) = 255;
      }

      for ( int n = 0; n < 3; ++n )
      {
         slice( center[ 0 ] + n + extent[ 0 ], center[ 1 ]     + extent[ 1 ], 0 ) = 255;
         slice( center[ 0 ] - n + extent[ 0 ], center[ 1 ]     + extent[ 1 ], 0 ) = 255;
         slice( center[ 0 ]     + extent[ 0 ], center[ 1 ] + n + extent[ 1 ], 0 ) = 255;
         slice( center[ 0 ]     + extent[ 0 ], center[ 1 ] - n + extent[ 1 ], 0 ) = 255;
      }
      return sliceReal.getStorage();

      // finally resample the image with the correct center and size
      Slice sliceResampled( nll::core::vector3ui( 2 * extent[ 0 ], 2 * extent[ 1 ], 1 ),
                            nll::core::vector3f( 1, 0, 0 ),
                            nll::core::vector3f( 0, 1, 0 ),
                            nll::core::vector3f( center[ 0 ], center[ 1 ], 0 ),
                            nll::core::vector2f( 1, 1 ) );

      typedef nll::core::InterpolatorNearestNeighbor2D<unsigned char, Image::IndexMapper, Image::Allocator> Interpolator;
      Interpolator interpolator( slice );
      nll::imaging::resampling<unsigned char, Interpolator> ( sliceReal, sliceResampled );

      // finally resample to the final size. Note, this step could be performed at the same time as the resampling...
      core::rescaleBilinear( sliceResampled.getStorage(), _params.preprocessSizeX, _params.preprocessSizeY );
      return sliceResampled.getStorage();
   }
}
}