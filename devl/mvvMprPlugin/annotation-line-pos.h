#ifndef MVV_PLATFORM_ANNOTATION_LINE_POS_H_
# define MVV_PLATFORM_ANNOTATION_LINE_POS_H_

# include "annotation.h"
# include <mvvPlatform/font.h>

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Annotate a line defined by 2 points on a slice with a target and linked caption
    */
   class MVVMPRPLUGIN_API AnnotationLinePos : public Annotation
   {
   public:
      AnnotationLinePos( const nll::core::vector3f& position1,
                         const nll::core::vector3f& position2,
                         const std::string& caption,
                         const std::string& caption2,
                         Font& font,
                         ui32 fontSize = 16,
                         nll::core::vector3uc color = nll::core::vector3uc( 255, 255, 0 ) ) : _position1( position1 ), _position2( position2 ), _caption( caption ), _caption2( caption2 ), _font( font ), _fontSize( fontSize ),_color( color )
      {
         _orientation /= _orientation.norm2();
      }

      virtual void setPosition( const nll::core::vector3f& pos )
      {
         _position1 = pos;
      }

      void setPosition2( const nll::core::vector3f& pos )
      {
         _position2 = pos;
      }

      virtual const nll::core::vector3f& getPosition()
      {
         return _position1;
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
         nll::core::vector3f p1 = slice.getOrthogonalProjection( _position1 );
         nll::core::vector2f p1c = slice.worldToSliceCoordinate( p1 );
         nll::core::vector3f p2 = slice.getOrthogonalProjection( _position2 );
         nll::core::vector2f p2c = slice.worldToSliceCoordinate( p2 );

         nll::core::vector2i size( static_cast<int>( slice.size()[ 0 ] ),
                                   static_cast<int>( slice.size()[ 1 ] ) );

         // convert slice coordinate to screen coordinate
         p1c[ 0 ] += size[ 0 ] / 2;
         p1c[ 1 ] += size[ 1 ] / 2;
         p2c[ 0 ] += size[ 0 ] / 2;
         p2c[ 1 ] += size[ 1 ] / 2;

         const bool p1isInside = p1c[ 0 ] > 0 && p1c[ 0 ] < size[ 0 ] &&
                                 p1c[ 1 ] > 0 && p1c[ 1 ] < size[ 1 ];
         const bool p2isInside = p2c[ 0 ] > 0 && p2c[ 0 ] < size[ 0 ] &&
                                 p2c[ 1 ] > 0 && p2c[ 1 ] < size[ 1 ];
         if ( !p1isInside && !p2isInside )
         {
            // nothing to display as both points are outside
            return;
         }

         if ( !p1isInside || !p2isInside )
         {
            // find the intersection
            nll::core::vector3f i1, i2;
            const nll::core::vector3f dir( p2c[ 0 ] - p1c[ 0 ], p2c[ 1 ] - p1c[ 1 ], 1 );
            nll::core::GeometryBox box( nll::core::vector3f( 0, 0, 0 ),
                                        nll::core::vector3f( slice.size()[ 0 ] - 1, slice.size()[ 1 ] - 1, 3 ) );
            bool intersection = box.getIntersection( nll::core::vector3f( p1c[ 0 ], p1c[ 1 ], 1 ),
                                                     dir,
                                                     i1, i2 );
            ensure( intersection, "it must have an intersection!" );

            // check the dot product to know what direction the points are...
            const nll::core::vector3f intersectionDir( i2[ 0 ] - i1[ 0 ], i2[ 1 ] - i1[ 1 ], 1 );
            int sign = nll::core::sign( dir.dot( intersectionDir ) );

            // update the points
            if ( !p1isInside )
            {
               if ( sign > 0 )
               {
                  p1c = nll::core::vector2f( i1[ 0 ], i1[ 1 ] );
               } else {
                  p1c = nll::core::vector2f( i2[ 0 ], i2[ 1 ] );
               }
            }

            if ( !p2isInside )
            {
               if ( sign > 0 )
               {
                  p2c = nll::core::vector2f( i2[ 0 ], i2[ 1 ] );
               } else {
                  p2c = nll::core::vector2f( i1[ 0 ], i1[ 1 ] );
               }
            }
         }

         if ( p1isInside )
         {
            nll::core::vector2i ca1( std::max<int>( 0, p1c[ 0 ] - (int)_fontSize / 2 ),
                                     p1c[ 1 ] );
            nll::core::vector2i ca2( std::min<int>( size[ 0 ] - 1, p1c[ 0 ] + (int)_fontSize / 2 ),
                                     p1c[ 1 ] );
            nll::core::vector2i cb1( p1c[ 0 ],
                                     std::max<int>( 0, p1c[ 1 ] - (int)_fontSize / 2 ) );
            nll::core::vector2i cb2( p1c[ 0 ],
                                     std::min<int>( size[ 1 ] - 1, p1c[ 1 ] + (int)_fontSize / 2 ) );

            nll::core::bresham( slice.getStorage(), ca1, ca2, _color );
            nll::core::bresham( slice.getStorage(), cb1, cb2, _color );
         }

         if ( p2isInside )
         {
            nll::core::vector2i ca1( std::max<int>( 0, p2c[ 0 ] - (int)_fontSize / 2 ),
                                     p2c[ 1 ] );
            nll::core::vector2i ca2( std::min<int>( size[ 0 ] - 1, p2c[ 0 ] + (int)_fontSize / 2 ),
                                     p2c[ 1 ] );
            nll::core::vector2i cb1( p2c[ 0 ],
                                     std::max<int>( 0, p2c[ 1 ] - (int)_fontSize / 2 ) );
            nll::core::vector2i cb2( p2c[ 0 ],
                                     std::min<int>( size[ 1 ] - 1, p2c[ 1 ] + (int)_fontSize / 2 ) );

            nll::core::bresham( slice.getStorage(), ca1, ca2, _color );
            nll::core::bresham( slice.getStorage(), cb1, cb2, _color );
         }

         nll::core::bresham( slice.getStorage(),
                             nll::core::vector2i( p1c[ 0 ], p1c[ 1 ] ),
                             nll::core::vector2i( p2c[ 0 ], p2c[ 1 ] ),
                             _color );

         if ( _caption != "" )
         {
            _font.setSize( _fontSize );
            _font.setColor( _color );
            _font.write( _caption, nll::core::vector2ui( p1c[ 0 ], p1c[ 1 ] + _fontSize / 2 ), slice.getStorage() );
         }

         if ( _caption2 != "" )
         {
            _font.setSize( _fontSize );
            _font.setColor( _color );
            _font.write( _caption2, nll::core::vector2ui( p2c[ 0 ], p2c[ 1 ] + _fontSize / 2 ), slice.getStorage() );
         }
      }

   private:
      // copy disabled
      AnnotationLinePos( const AnnotationLinePos& );
      AnnotationLinePos& operator=( const AnnotationLinePos& );

   private:
      nll::core::vector3f  _position1;
      nll::core::vector3f  _position2;
      nll::core::vector3f  _orientation;
      std::string          _caption;
      std::string          _caption2;
      Font&                _font;
      ui32                 _fontSize;
      nll::core::vector3uc _color;
   };
}
}

#endif