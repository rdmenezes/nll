#include "sliceMapper.h"

/*
      for ( int n = 0; n < 3; ++n )
      {
         sliceMask( minBB[ 0 ] + n, minBB[ 1 ], 0 ) = 255;
         sliceMask( minBB[ 0 ] - n, minBB[ 1 ], 0 ) = 255;
         sliceMask( minBB[ 0 ], minBB[ 1 ] + n, 0 ) = 255;
         sliceMask( minBB[ 0 ], minBB[ 1 ] - n, 0 ) = 255;
      }

      for ( int n = 0; n < 3; ++n )
      {
         sliceMask( center[ 0 ] + n , center[ 1 ]     , 0 ) = 255;
         sliceMask( center[ 0 ] - n , center[ 1 ]     , 0 ) = 255;
         sliceMask( center[ 0 ]     , center[ 1 ] + n , 0 ) = 255;
         sliceMask( center[ 0 ]     , center[ 1 ] - n , 0 ) = 255;
      }

      for ( int n = 0; n < 3; ++n )
      {
         sliceMask( maxBB[ 0 ] + n, maxBB[ 1 ], 0 ) = 255;
         sliceMask( maxBB[ 0 ] - n, maxBB[ 1 ], 0 ) = 255;
         sliceMask( maxBB[ 0 ], maxBB[ 1 ] + n, 0 ) = 255;
         sliceMask( maxBB[ 0 ], maxBB[ 1 ] - n, 0 ) = 255;
      }
      return sliceMask;
*/

using namespace nll;

namespace mvv
{
namespace mapper
{
   // from the center of mass:
   // scan in left, right, up and down direction until desired ratio is reached. If in a direction there is a line that doesn't have enough points, we impose the limit on the extent
   void getExtent( const SliceMapper::Image& slice, const nll::core::vector2i& center, double ratiox, double ratioy, const std::vector<double>& linex, const std::vector<double>& liney, double totalPixels, nll::core::vector2i& minBB, nll::core::vector2i& maxBB )
   {
      minBB = nll::core::vector2i( -1, -1 );
      maxBB = nll::core::vector2i( -1, -1 );
      const double minRatioFactor = 8;

      double nbPixels = 0;
      int n = 0;
      for ( ; n < (int)slice.sizex() / 2; ++n )
      {
         const int x1 = (int)center[ 0 ] + n + 1;
         const int x2 = (int)center[ 0 ] - n;
         if ( x1 + 1 >= (int)slice.sizex() || x2 <= 0 )
            break;
         nbPixels += liney[ x1 ] + liney[ x2 ];
         const double ratio = nbPixels / totalPixels;
         if ( ratio >= ratiox )
            break;

         // now check we have enough voxel on the line we are checking, else constaint the extent within X
         // for this we want the points close to the center to be very likely to be inside, then less and less
         const double ratiox1 = liney[ x1 ] / slice.sizex();
         const double ratiox2 = liney[ x2 ] / slice.sizex();
         const double ratioPos = (double)n / ( slice.sizex() / 2 );
         const double ratioxToMatch = exp( ratioPos / 3 ) - 1;

         if ( maxBB[ 0 ] < 0 && ratiox1 < ratioxToMatch )
         {
            maxBB[ 0 ] = center[ 0 ] + n;
         }
         if ( minBB[ 0 ] < 0 && ratiox2 < ratioxToMatch )
         {
            minBB[ 0 ] = center[ 0 ] - n;
         }
      }

      if ( maxBB[ 0 ] < 0 )
         maxBB[ 0 ] = center[ 0 ] + n;
      if ( minBB[ 0 ] < 0 )
         minBB[ 0 ] = center[ 0 ] - n;

      nbPixels = 0;
      n = 0;
      for ( ; n < (int)slice.sizey() / 2; ++n )
      {
         const int y1 = (int)center[ 1 ] + n + 1;
         const int y2 = (int)center[ 1 ] - n;
         if ( y1 + 1 >= (int)slice.sizey() || y2 <= 0 )
            break;
         nbPixels += linex[ y1 ] + linex[ y2 ];
         const double ratio = nbPixels / totalPixels;
         if ( ratio >= ratioy )
            break;

         // now check we have enough voxel on the line we are checking, else constaint the extent within
         const double ratioy1 = linex[ y1 ] / slice.sizey();
         const double ratioy2 = linex[ y2 ] / slice.sizey();
         const double ratioyToMatch = ratioy / minRatioFactor;
         if ( maxBB[ 1 ] < 0 && ratioy1 < ratioyToMatch )
         {
            maxBB[ 1 ] = center[ 1 ] + n;
         }
         if ( minBB[ 1 ] < 0 && ratioy2 < ratioyToMatch )
         {
            minBB[ 1 ] = center[ 1 ] - n;
         }
      }

      if ( maxBB[ 1 ] < 0 )
         maxBB[ 1 ] = center[ 1 ] + n;
      if ( minBB[ 1 ] < 0 )
         minBB[ 1 ] = center[ 1 ] - n;

      // we want a symetry in X (in case one arm is up, the other down), so do the average in X
      const double sizex = ( abs( maxBB[ 0 ] - center[ 0 ] ) +
                             abs( minBB[ 0 ] - center[ 0 ] ) ) / 2;
      const double sizey = ( abs( maxBB[ 1 ] - center[ 1 ] ) +
                             abs( minBB[ 1 ] - center[ 1 ] ) ) / 2;
      minBB[ 0 ] = center[ 0 ] - sizex;
      maxBB[ 0 ] = center[ 0 ] + sizex;
   }

   SliceMapper::Image SliceMapper::preprocessSlice( const SliceMapper::Volume& volume, unsigned sliceNumber ) const
   {
      // we use 2 luts: one with a lot of soft tissue to find the center of mass (with the bony LUT, it depends on the skeleton which is not smooth)
      // the other one is a bony LUT to compute the extent of the bounding box.

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
      double nbPointsCenter = 0;
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

            const double bound = 1;

            // compute the center
            if ( val > 0 )
            {
               dx += x * bound;
               dy += y * bound;
               nbPointsCenter += bound;
            }

            // compute the line sums
            if ( valMask > 0 )
            {
               sumx += bound;
               lineSumY[ x ] += bound;
               nbPoints += bound;
            }
         }
         lineSumX[ y ] = sumx;
      }


      const nll::core::vector2f center( dx / nbPointsCenter, dy / nbPointsCenter ); 

      // compute the distance we need to crop, retain X% in a direction from the center
      nll::core::vector2i minBB;
      nll::core::vector2i maxBB;
      getExtent( sliceMask, nll::core::vector2i( center[ 0 ], center[ 1 ] ), 0.85, 0.85, lineSumX, lineSumY, nbPoints, minBB, maxBB );
      const nll::core::vector2f centerI( slice.sizex() / 2 - center[ 0 ],
                                         slice.sizey() / 2 - center[ 1 ] );

      // finally resample the image with the correct center and size
      Slice sliceResampled( nll::core::vector3ui( maxBB[ 0 ] - minBB[ 0 ] + 1, maxBB[ 1 ] - minBB[ 1 ] + 1, 1 ),
                            nll::core::vector3f( 1, 0, 0 ),
                            nll::core::vector3f( 0, 1, 0 ),
                            nll::core::vector3f( ( maxBB[ 0 ] + minBB[ 0 ] ) / 2, ( maxBB[ 1 ] + minBB[ 1 ] ) / 2, 0 ),
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