#include "sliceMapper.h"

/*
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
*/

using namespace nll;

namespace mvv
{
namespace mapper
{
   nll::core::vector2i getExtent( const SliceMapper::Image& slice, const nll::core::vector2i& center, double ratiox, double ratioy, const std::vector<double>& linex, const std::vector<double>& liney, double totalPixels )
   {
      nll::core::vector2i extent;
      double nbPixels = 0;
      int n = 0;
      for ( ; n < (int)slice.sizex() / 2; ++n )
      {
         int x1 = (int)center[ 0 ] + n + 1;
         int x2 = (int)center[ 0 ] - n;
         if ( x1 + 1 >= (int)slice.sizex() || x2 <= 0 )
            break;
         nbPixels += liney[ x1 ] + liney[ x2 ];
         const double ratio = nbPixels / totalPixels;
         if ( ratio >= ratiox )
            break;
      }
      extent[ 0 ] = n;

      nbPixels = 0;
      n = 0;
      for ( ; n < (int)slice.sizey() / 2; ++n )
      {
         int y1 = (int)center[ 1 ] + n + 1;
         int y2 = (int)center[ 1 ] - n;
         if ( y1 + 1 >= (int)slice.sizey() || y2 <= 0 )
            break;
         nbPixels += linex[ y1 ] + linex[ y2 ];
         const double ratio = nbPixels / totalPixels;
         if ( ratio >= ratioy )
            break;
      }
      extent[ 1 ] = n;
      return extent;
   }

   SliceMapper::Image SliceMapper::preprocessSlice( const SliceMapper::Volume& volume, unsigned sliceNumber ) const
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
      double nbPoints = 0;
      std::vector<double> lineSumX( slice.sizey() );
      std::vector<double> lineSumY( slice.sizex() );
      for ( ui32 y = 0; y < slice.sizey(); ++y )
      {
         double sumx = 0;
         for ( ui32 x = 0; x < slice.sizex(); ++x )
         {
            const float volVal = volume( x, y, sliceNumber );
            const float val = _lut.transform( volVal )[ 0 ];
            const float valMask = _lutMask.transform( volVal )[ 0 ];
            slice( x, y, 0 ) = val;
            sliceMask( x, y, 0 ) = valMask;

            if ( valMask > 0 )
            {
               const double bound = valMask / 256;
               dx += x * bound;
               dy += y * bound;
               nbPoints += bound;
               sumx += bound;
               lineSumY[ x ] += bound;
            }
         }
         lineSumX[ y ] = sumx;
      }


      const nll::core::vector2f center( dx / nbPoints, dy / nbPoints ); 

      // compute the distance we need to crop, retain X% in a direction from the center
      nll::core::vector2i extent = getExtent( sliceMask, nll::core::vector2i( center[ 0 ], center[ 1 ] ), 0.85, 0.85, lineSumX, lineSumY, nbPoints );
      const nll::core::vector2f centerI( slice.sizex() / 2 - center[ 0 ],
                                         slice.sizey() / 2 - center[ 1 ] );

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

   SliceMapper::Database SliceMapper::createPreprocessedDatabase( const LandmarkDataset& datasets ) const
   {
      Database database;
      double lastDist = 0;
      for ( unsigned caseid = 0; caseid < datasets.size(); ++caseid )
      {
         Database::Sample::Type type = (Database::Sample::Type)datasets.getSampleType( caseid );
         std::auto_ptr<Volume> v = datasets.loadData( caseid );
         for ( ui32 slice = 0; slice < v->getSize()[ 2 ]; ++slice )
         {
            Image i = preprocessSlice( *v, slice );
            
            Database::Sample::Input input( i.size() );
            Database::Sample::Output output;
            for ( unsigned n = 0; n < i.size(); ++n )
            {
               input[ n ] = i[ n ] / 256.0;
            }
            ensure( 0, "TODO" ); // TODO: assign the correct output, and distance <lastDist>

            Database::Sample sample( input, output, (Database::Sample::Type)type );
            database.add( sample );
         }
      }

      return database;
   }
}
}