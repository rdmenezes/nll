#ifndef MVV_PLATFORM_ANNOTATION_LINE_H_
# define MVV_PLATFORM_ANNOTATION_LINE_H_

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
   class MVVMPRPLUGIN_API AnnotationLine : public Annotation
   {
   public:
      AnnotationLine( const nll::core::vector3f& position,
                      const nll::core::vector3f& orientation,
                      float sizeMM,                              // the lenght of the line in millimeter
                      const std::string& caption,
                      Font& font,
                      ui32 fontSize = 16,
                      nll::core::vector3uc color = nll::core::vector3uc( 255, 255, 0 ) ) : _position( position ), _orientation( orientation ), _size( sizeMM ), _caption( caption ), _font( font ), _fontSize( fontSize ),_color( color )
      {
         _orientation /= _orientation.norm2();
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

         // project the vector on a plane to finally compute the line orientation
         nll::core::vector3f p1 = slice.getOrthogonalProjection( _position );
         nll::core::vector2f p1c = slice.worldToSliceCoordinate( p1 );
         nll::core::vector3f p2 = slice.getOrthogonalProjection( _position + _orientation * _size / 2 );
         nll::core::vector2f p2c = slice.worldToSliceCoordinate( p2 );
         nll::core::vector2f orientation = p2c - p1c;
         const float norm = orientation.norm2();
         const ui32 nbSteps = norm; //norm > 0 ? ( norm / _size ) : 1;

         nll::core::vector2i pi( static_cast<int>( p1c[ 0 ] ),
                                    static_cast<int>( p1c[ 1 ] ) );
         nll::core::vector2i size( static_cast<int>( slice.size()[ 0 ] ),
                                   static_cast<int>( slice.size()[ 1 ] ) );

         if ( norm > 1e-4f )
         {
            orientation /= norm;

            nll::core::vector2f start( pi[ 0 ], pi[ 1 ] );
            ui32 step = 0;
            while ( slice.contains( start ) && step <= nbSteps )
            {
               ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( start[ 0 ] + size[ 0 ] / 2, start[ 1 ] + size[ 1 ] / 2 );
               it.pickcol( 0 ) = _color[ 0 ];
               it.pickcol( 1 ) = _color[ 1 ];
               it.pickcol( 2 ) = _color[ 2 ];

               start = start + orientation;
               ++step;
            }

            start = nll::core::vector2f( pi[ 0 ], pi[ 1 ] ) - orientation;
            step = 0;
            while ( slice.contains( start ) && step <= nbSteps )
            {
               ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( start[ 0 ] + size[ 0 ] / 2, start[ 1 ] + size[ 1 ] / 2 );
               it.pickcol( 0 ) = _color[ 0 ];
               it.pickcol( 1 ) = _color[ 1 ];
               it.pickcol( 2 ) = _color[ 2 ];

               start = start - orientation;
               ++step;
            }
         } else {
            if ( slice.contains( p1c ) )
            {
               const int pointSize = 5;

               i32 xmin = std::max( pi[ 0 ] - pointSize, -size[ 0 ] / 2 + 1 );
               i32 xmax = std::min( pi[ 0 ] + pointSize,  size[ 0 ] / 2 - 1 );

               i32 ymin = std::max( pi[ 1 ] - pointSize, -size[ 1 ] / 2 + 1 );
               i32 ymax = std::min( pi[ 1 ] + pointSize,  size[ 1 ] / 2 - 1 );

               ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( xmin + slice.size()[ 0 ] / 2, static_cast<ui32>( p1c[ 1 ] + slice.size()[ 1 ] / 2 ) );
               for ( i32 n = xmin; n < xmax; ++n, it.addx() )
               {
                  it.pickcol( 0 ) = _color[ 0 ];
                  it.pickcol( 1 ) = _color[ 1 ];
                  it.pickcol( 2 ) = _color[ 2 ];
               }

               it = slice.getIterator( static_cast<ui32>( p1c[ 0 ] + slice.size()[ 0 ] / 2 ), ymin + slice.size()[ 1 ] / 2 );
               for ( i32 n = ymin; n < ymax; ++n, it.addy() )
               {
                  it.pickcol( 0 ) = _color[ 0 ];
                  it.pickcol( 1 ) = _color[ 1 ];
                  it.pickcol( 2 ) = _color[ 2 ];
               }
            }
         }

         if ( slice.contains( p1c ) )
         {
            _font.setSize( _fontSize );
            _font.setColor( _color );
            _font.write( _caption, nll::core::vector2ui( pi[ 0 ] + 10 + size[ 0 ] / 2, pi[ 1 ] + size[ 1 ] / 2 ), slice.getStorage() );
         }
      }

   private:
      // copy disabled
      AnnotationLine( const AnnotationLine& );
      AnnotationLine& operator=( const AnnotationLine& );

   private:
      nll::core::vector3f  _position;
      nll::core::vector3f  _orientation;
      float                _size;
      std::string          _caption;
      Font&                _font;
      ui32                 _fontSize;
      nll::core::vector3uc _color;
   };
}
}

#endif