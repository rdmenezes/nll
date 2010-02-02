#ifndef MVV_PLATFORM_ANNOTATION_POINT_H_
# define MVV_PLATFORM_ANNOTATION_POINT_H_

# include "annotation.h"

namespace mvv
{
namespace platform
{
   class MVVMPRPLUGIN_API AnnotationPoint : public Annotation
   {
   public:
      AnnotationPoint( const nll::core::vector3f& position,
                       nll::core::vector3uc color = nll::core::vector3uc( 255, 255, 0 ) ) : _position( position ), _color( color )
      {}

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

         nll::core::vector3f p1 = slice.getOrthogonalProjection( _position );
         nll::core::vector2f p2 = slice.worldToSliceCoordinate( p1 );
         nll::core::vector2i pi( static_cast<int>( p2[ 0 ] ),
                                 static_cast<int>( p2[ 1 ] ) );
         nll::core::vector2i size( static_cast<int>( slice.size()[ 0 ] ),
                                   static_cast<int>( slice.size()[ 1 ] ) );

         if ( slice.contains( p2 ) )
         {
            const int pointSize = 6;

            i32 xmin = std::max( pi[ 0 ] - pointSize, -size[ 0 ] / 2 + 1 );
            i32 xmax = std::min( pi[ 0 ] + pointSize,  size[ 0 ] / 2 - 1 );

            i32 ymin = std::max( pi[ 1 ] - pointSize, -size[ 1 ] / 2 + 1 );
            i32 ymax = std::min( pi[ 1 ] + pointSize,  size[ 1 ] / 2 - 1 );

            ResourceSliceuc::value_type::Storage::DirectionalIterator  it = slice.getIterator( xmin + slice.size()[ 0 ] / 2, static_cast<ui32>( p2[ 1 ] + slice.size()[ 1 ] / 2 ) );
            for ( i32 n = xmin; n < xmax; ++n, it.addx() )
            {
               it.pickcol( 0 ) = _color[ 0 ];
               it.pickcol( 1 ) = _color[ 1 ];
               it.pickcol( 2 ) = _color[ 2 ];
            }

            it = slice.getIterator( static_cast<ui32>( p2[ 0 ] + slice.size()[ 0 ] / 2 ), ymin + slice.size()[ 1 ] / 2 );
            for ( i32 n = ymin; n < ymax; ++n, it.addy() )
            {
               it.pickcol( 0 ) = _color[ 0 ];
               it.pickcol( 1 ) = _color[ 1 ];
               it.pickcol( 2 ) = _color[ 2 ];
            }
         }
      }

   private:
      nll::core::vector3f  _position;
      nll::core::vector3uc _color;
   };
}
}

#endif