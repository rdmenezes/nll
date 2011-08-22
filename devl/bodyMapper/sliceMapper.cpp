#include "sliceMapper.h"

using namespace nll;


/*
      // extract the largest CC
      typedef nll::algorithm::Labelize<nll::ui8,
                                       nll::core::IndexMapperRowMajorFlat2DColorRGBn,
                                       nll::algorithm::RegionPixelSpecific<nll::ui8> >  Labelize;

      ui8 black[] = { 0 };
      Labelize::DifferentPixel different( 1, black, 10 );

      Labelize l( different );
      Labelize::ComponentsInfo info = l.run( sliceMask, true );
      
      int max = 0;
      int maxIndex = -1;
      for ( ui32 n = 0; n < info.components.size(); ++n )
      {
         if ( info.components[ n ].size > (ui32)max &&
              sliceMask( info.components[ n ].posx, info.components[ n ].posy, 0 ) > 0 )
         {
            maxIndex = n;
            max = info.components[ n ].size;
         }
      }

      if ( maxIndex >= 0 )
      {
         slice = extract( slice, info.labels, info.components[ maxIndex ].id );
      }

      // center the image
      cropVertical( slice, 0.06f, 2 );
      cropHorizontal( slice, 0.03f, 2 );

      // normalize the size
      core::rescaleBilinear( slice, _params.preprocessSizeX, _params.preprocessSizeY );*/

namespace mvv
{
namespace mapper
{
   /*
   SliceMapper::Image SliceMapper::preprocessSlice( const SliceMapper::Volume& volume, unsigned sliceNumber )
   {
      // get the LUT transformed slice
      Image slice( volume.getSize()[ 0 ], volume.getSize()[ 1 ], 1, false );
      Image sliceMask( volume.getSize()[ 0 ], volume.getSize()[ 1 ], 1, false );
      for ( ui32 y = 0; y < slice.sizey(); ++y )
      {
         for ( ui32 x = 0; x < slice.sizex(); ++x )
         {
            const float volVal = volume( x, y, sliceNumber );

            const float val = _lut.transform( volVal )[ 0 ];
            const float valMask = _lutMask.transform( volVal )[ 0 ];
            slice( x, y, 0 ) = val;
            sliceMask( x, y, 0 ) = valMask;
         }
      }

      cropVertical( slice, 0.1f, 2 );
      cropHorizontal( slice, 0.1f, 2 );
      centerImage( slice );
      core::rescaleBilinear( slice, _params.preprocessSizeX, _params.preprocessSizeY );
      return slice;
   }*/

   SliceMapper::Image SliceMapper::preprocessSlice( const SliceMapper::Volume& volume, unsigned sliceNumber )
   {
      // get the LUT transformed slice
      Image slice( volume.getSize()[ 0 ], volume.getSize()[ 1 ], 1, false );
      Image sliceMask( volume.getSize()[ 0 ], volume.getSize()[ 1 ], 1, false );
      for ( ui32 y = 0; y < slice.sizey(); ++y )
      {
         for ( ui32 x = 0; x < slice.sizex(); ++x )
         {
            const float volVal = volume( x, y, sliceNumber );

            const float val = _lut.transform( volVal )[ 0 ];
            const float valMask = _lutMask.transform( volVal )[ 0 ];
            slice( x, y, 0 ) = val;
            sliceMask( x, y, 0 ) = valMask;
         }
      }

      // normalize the size
      const unsigned zoomFactor = 3;
      core::rescaleBilinear( slice, zoomFactor * _params.preprocessSizeX, zoomFactor * _params.preprocessSizeY );
      core::rescaleBilinear( sliceMask, zoomFactor * _params.preprocessSizeX, zoomFactor * _params.preprocessSizeY );

      // extract the largest CC
      typedef nll::algorithm::Labelize<nll::ui8,
                                       nll::core::IndexMapperRowMajorFlat2DColorRGBn,
                                       nll::algorithm::RegionPixelSpecific<nll::ui8> >  Labelize;

      ui8 black[] = { 0 };
      Labelize::DifferentPixel different( 1, black, 10 );

      Labelize l( different );
      Labelize::ComponentsInfo info = l.run( sliceMask, true );
      
      int max = 0;
      int maxIndex = -1;
      for ( ui32 n = 0; n < info.components.size(); ++n )
      {
         if ( info.components[ n ].size > (ui32)max &&
              sliceMask( info.components[ n ].posx, info.components[ n ].posy, 0 ) > 0 )
         {
            maxIndex = n;
            max = info.components[ n ].size;
         }
      }

      if ( maxIndex >= 0 )
      {
         slice = extract( slice, info.labels, info.components[ maxIndex ].id );
      }

      // center the image
      cropVertical( slice, 0.06f, 2 );
      cropHorizontal( slice, 0.06f, 2 );

      core::rescaleBilinear( slice, _params.preprocessSizeX, _params.preprocessSizeY );
      return slice;
   }
}
}