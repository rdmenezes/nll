#ifndef MVV_PLATFORM_ANNOTATION_COLORS_H_
# define MVV_PLATFORM_ANNOTATION_COLORS_H_

# include "annotation.h"
# include <mvvPlatform/font.h>

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Annotate a specific point on a slice with a target and linked caption
    */
   class MVVMPRPLUGIN_API AnnotationColors : public Annotation
   {
   public:
      /**
       @param heightMM the direction to display
       */
      AnnotationColors( const nll::core::vector3f& position,
                        const nll::core::vector3f& heightMM,
                        ui32 widthPixel,
                        const std::vector<nll::core::vector3uc>& colors,
                        nll::core::vector3uc transparentColor = nll::core::vector3uc( 0, 0, 0 ) ) : _position( position ), _heightMM( heightMM ), _width( widthPixel ), _transparentColor( transparentColor )
      {
         _colors = nll::core::Image<ui8>( 1, (ui32)colors.size(), 3 );
         for ( ui32 n = 0; n < colors.size(); ++n )
         {
            ui8* c = _colors.point( 0, n );
            c[ 0 ] = colors[ n ][ 0 ];
            c[ 1 ] = colors[ n ][ 1 ];
            c[ 2 ] = colors[ n ][ 2 ];
         }
      }

      virtual void setPosition( const nll::core::vector3f& pos )
      {
         _position = pos;
      }

      virtual const nll::core::vector3f& getPosition()
      {
         return _position;
      }

      virtual void updateSegment( ResourceSliceuc s )
      {
         Sliceuc& slice = s.getValue();
         if ( !slice.size()[ 0 ] ||
              !slice.size()[ 1 ] ||
              !slice.size()[ 2 ] ||
              slice.getAxisX().norm2() < 1e-4 || 
              slice.getAxisY().norm2() < 1e-4 )
         {
            return;
         }

         nll::core::vector3f p1, p3;
         nll::core::vector2f p2, p4;
         int nbPixels = 0;
         try
         {
            p1 = slice.getOrthogonalProjection( _position );
            p2 = slice.worldToSliceCoordinate( p1 );
            p3 = slice.getOrthogonalProjection( _position + _heightMM );
            p4 = slice.worldToSliceCoordinate( p3 );
            nbPixels = fabs( p2[ 1 ] - p4[ 1 ] );
            if ( nbPixels <= 0 )
               return;
         } catch(...)
         {
            std::cout << "plane exception" << std::endl;
            return;
         }

         nll::core::Image<ui8> im;
         im.clone( _colors );
         nll::core::rescaleNearestNeighbor( im, 1, nbPixels );


         nll::core::vector2i pi( static_cast<int>( p2[ 0 ] ),
                                 static_cast<int>( p2[ 1 ] ) );
         nll::core::vector2i size( static_cast<int>( slice.size()[ 0 ] ),
                                   static_cast<int>( slice.size()[ 1 ] ) );

         
         if ( slice.contains( p2 ) || slice.contains( p4 ) || slice.contains( (p2 + p4)/2 ))
         {
            for ( int n = 0; n < nbPixels; ++n )
            {
               nll::core::vector2f p( pi[ 0 ], n + static_cast<int>( pi[ 1 ] ) );
               if ( slice.contains( p ) )
               {
                  int xx = (int)p[ 0 ]  + (int)slice.size()[ 0 ] / 2;
                  int yy = (int)p[ 1 ]  + (int)slice.size()[ 1 ] / 2;
                  ui8* col = im.point( 0, n );
                  if ( _transparentColor[ 0 ] == col[ 0 ] && _transparentColor[ 1 ] == col[ 1 ] && _transparentColor[ 2 ] == col[ 2 ] )
                     continue;   // skip, we have a transparent color
                  ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( xx, yy );
                  int max = std::min<int>( p[ 0 ] + (int)_width, slice.size()[ 0 ] / 2 );
                  for ( int x = p[ 0 ]; x < max; ++x )
                  {
                     it.pickcol( 0 ) = col[ 0 ];
                     it.pickcol( 1 ) = col[ 1 ];
                     it.pickcol( 2 ) = col[ 2 ];
                     it.addx();
                  }
               }
            }
         }
      }

   private:
      // copy disabled
      AnnotationColors( const AnnotationColors& );
      AnnotationColors& operator=( const AnnotationColors& );

   private:
      nll::core::vector3f     _position;
      nll::core::vector3f     _heightMM;
      nll::core::Image<ui8>   _colors;
      ui32                    _width;
      nll::core::vector3uc    _transparentColor;
   };
}
}

#endif