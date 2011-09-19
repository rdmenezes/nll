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
      typedef nll::core::Matrix<float>                     Matrix;
      typedef nll::core::vector3uc                         Color;

   public:
      SliceMapperVisualization( const std::string previewOutputDir = "../../bodyMapper/data/sliceMapperVolumePreview/" ) : _previewOutputDir( previewOutputDir )
      {
         _colors.push_back( Color( 255, 0, 0 ) );
         _colors.push_back( Color( 0, 255, 0 ) );
         _colors.push_back( Color( 0, 0, 255 ) );

         _colors.push_back( Color( 255, 255, 0 ) );
         _colors.push_back( Color( 255, 0, 255 ) );
         _colors.push_back( Color( 0, 255, 255 ) );

         _colors.push_back( Color( 128, 0, 0 ) );
         _colors.push_back( Color( 0, 128, 0 ) );
         _colors.push_back( Color( 0, 0, 128 ) );

         _colors.push_back( Color( 128, 255, 0 ) );
         _colors.push_back( Color( 128, 0, 255 ) );
         _colors.push_back( Color( 0, 255, 128 ) );

         _colors.push_back( Color( 255, 128, 0 ) );
         _colors.push_back( Color( 255, 0, 128 ) );
         _colors.push_back( Color( 0, 255, 128 ) );

         _colors.push_back( Color( 255, 128, 128 ) );
         _colors.push_back( Color( 255, 128, 128 ) );
         _colors.push_back( Color( 128, 255, 128 ) );

         _colors.push_back( Color( 255, 128, 255 ) );
         _colors.push_back( Color( 255, 255, 128 ) );
         _colors.push_back( Color( 255, 255, 128 ) );
      }

      void createPreview( const LandmarkDataset& data ) const
      {
         for ( unsigned n = 0; n < data.size(); ++n )
         {
            std::cout << "case=" << n << " out of=" << data.size() << std::endl;

            std::auto_ptr<Volume> v = data.loadData( n );
            ensure( v.get(), "can't load volume:" + data[ n ].fileId );

            Image i = _extractXZ( *v );
            nll::core::extend( i, 3 );

            const std::string outName = _getPreviewName( data, n );
            nll::core::writeBmp( i, outName );
         }
      }

      /**
       @brief create the preview of a result
       */
      Image annotate( const LandmarkDataset& data, unsigned id, const Matrix& pbs, const std::vector<unsigned>& annotations ) const
      {
         Image res( _getPreviewName( data, id ) );
         ensure( res.sizex() && res.sizey(), "can't load the preview data" );
         ensure( res.sizey() == pbs.sizey(), "the y-size of the preview must match the y size of pbs" );

         const double pbsRatio = 0.3;
         const double step = res.sizex() * pbsRatio / annotations.size();
         ensure( step > 0, "image too small!" );

         // display the pbs
         for ( unsigned y = 0; y < pbs.sizey(); ++y )
         {
            for ( unsigned n = 0; n < pbs.sizex(); ++n )
            {
               const unsigned start = static_cast<unsigned>( n * step );
               const unsigned end = start + static_cast<unsigned>( step ) - 1;
               _drawHorizontalLine( res, y, start, end, _colors[ n ], true );
            }
         }

         // display the final decision
         for ( unsigned l = 0; l < annotations.size(); ++l )
         {
            if ( annotations[ l ] >= 0 )
            {
               const unsigned start = static_cast<unsigned>( res.sizex() * pbsRatio );
               const unsigned end = res.sizex() - 1;
               _drawHorizontalLine( res, annotations[ l ], start, end, _colors[ l ], true );
            }
         }
         return res;
      }

   private:
      std::string _getPreviewName( const LandmarkDataset& data, unsigned id ) const
      {
         return _previewOutputDir + data[ id ].fileId + ".bmp";
      }

      void _drawHorizontalLine( Image& i, unsigned ypos, unsigned min, unsigned max, const Color& color, bool blendWithBackground ) const
      {
         ensure( ypos < i.sizey() && min < max && max <i.sizex(), "out of bound" );
         if ( blendWithBackground )
         {
            for ( unsigned n = min; n <= max; ++n )
            {
               nll::ui8* p = i.point( n, ypos );
               nll::core::vector3f val( ( (float)color[ 0 ] + (float)p[ 0 ] ) * 0.5f,
                                        ( (float)color[ 1 ] + (float)p[ 1 ] ) * 0.5f,
                                        ( (float)color[ 2 ] + (float)p[ 2 ] ) * 0.5f );
               p[ 0 ] = (nll::ui8)NLL_BOUND( val[ 0 ], 0, 255 );
               p[ 1 ] = (nll::ui8)NLL_BOUND( val[ 1 ], 0, 255 );
               p[ 2 ] = (nll::ui8)NLL_BOUND( val[ 2 ], 0, 255 );
            }
         } else {
            for ( unsigned n =  min; n <= max; ++ n )
            {
               nll::ui8* p = i.point( n, ypos );
               p[ 0 ] = color[ 0 ];
               p[ 1 ] = color[ 1 ];
               p[ 2 ] = color[ 2 ];
            }
         }
      }

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

   private:
      std::vector<Color>   _colors;
      std::string          _previewOutputDir;
   };
}
}

# pragma warning( pop )

#endif