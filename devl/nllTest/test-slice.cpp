#include "stdafx.h"
#include <nll/nll.h>

class TestSlice
{
public:
   typedef nll::imaging::MapperLutColor<unsigned char>               Mapper;
   typedef nll::imaging::LookUpTransform<unsigned char, Mapper>      Lut;

   void testContains()
   {
      typedef nll::f32 type;
      typedef nll::imaging::Slice<type>   Slice;

      unsigned sizex = 512;
      unsigned sizey = 1024;
      Slice t1( nll::core::vector3ui( sizex, sizey, 1 ),
                nll::core::vector3f( 1.0f, 0, 0 ),
                nll::core::vector3f( 0, 1.0f, 0 ),
                nll::core::vector3f( 10, 0, 5 ),
                nll::core::vector2f( 1.0f, 1.0f ) );

      TESTER_ASSERT( t1.isInPlane( nll::core::vector3f( 0, 0, 5 ) ) );
      TESTER_ASSERT( t1.isInPlane( nll::core::vector3f( (float)sizex, 0, 5 ) ) );
      TESTER_ASSERT( t1.isInPlane( nll::core::vector3f( (float)sizex, (float)sizey, 5 ) ) );
      TESTER_ASSERT( !t1.isInPlane( nll::core::vector3f( (float)sizex, (float)sizey, 4 ) ) );
      TESTER_ASSERT( !t1.isInPlane( nll::core::vector3f( (float)sizex, (float)sizey, 6 ) ) );
      TESTER_ASSERT( t1.isInPlane( nll::core::vector3f( 4, 4, 5 ) ) );
      TESTER_ASSERT( t1.isInPlane( nll::core::vector3f( (float)sizex * 1000, (float)sizey, 5 ) ) );

      TESTER_ASSERT( t1.contains( nll::core::vector3f( 0, 0, 5 ) ) );
      TESTER_ASSERT( t1.contains( nll::core::vector3f( (float)sizex / 2, 0, 5 ) ) );
      TESTER_ASSERT( t1.contains( nll::core::vector3f( (float)sizex / 2, (float)sizey / 2, 5 ) ) );
      TESTER_ASSERT( t1.contains( nll::core::vector3f( -128, 0, 5 ) ) );
      TESTER_ASSERT( t1.contains( nll::core::vector3f( (float)sizex / 2, -(float)sizey / 2, 5 ) ) );

      TESTER_ASSERT( !t1.contains( nll::core::vector3f( (float)sizex / 2, (float)sizey / 2, 4 ) ) );
      TESTER_ASSERT( !t1.contains( nll::core::vector3f( (float)sizex / 2, (float)sizey / 2, 6 ) ) );
      TESTER_ASSERT( !t1.contains( nll::core::vector3f( (float)sizex / 2 + 11, (float)sizey / 2, 5 ) ) );

      const nll::core::vector3f p( 0, 4, 5 );
      const nll::core::vector2f ptrans = t1.worldToSliceCoordinate( p );
      TESTER_ASSERT( fabs( ptrans[ 0 ] - ( -10 ) ) < 1e-4 );
      TESTER_ASSERT( fabs( ptrans[ 1 ] - ( 4 ) ) < 1e-4 );
   }

   void testCoordinateSystem()
   {
      typedef nll::f32 type;
      typedef nll::imaging::Slice<type>   Slice;

      unsigned sizex = 512;
      unsigned sizey = 1024;
      Slice t1( nll::core::vector3ui( sizex, sizey, 1 ),
                nll::core::vector3f( -1.0f, 0, 0 ),
                nll::core::vector3f( 0, 2.0f, 0 ),
                nll::core::vector3f( 10, 0, 0 ),
                nll::core::vector2f( 2.0f, 1.0f ) );

      const nll::core::vector3f p( 0, 0, 0 );
      const nll::core::vector2f ptrans = t1.worldToSliceCoordinate( p );
      TESTER_ASSERT( fabs( ptrans[ 0 ] - ( 5 ) ) < 1e-4 );
      TESTER_ASSERT( fabs( ptrans[ 1 ] - ( 0 ) ) < 1e-4 );
      const nll::core::vector3f p0 = t1.sliceToWorldCoordinate( ptrans );
      TESTER_ASSERT( fabs( p0[ 0 ] - ( 0 ) ) < 1e-4 );
      TESTER_ASSERT( fabs( p0[ 1 ] - ( 0 ) ) < 1e-4 );
      TESTER_ASSERT( fabs( p0[ 2 ] - ( 0 ) ) < 1e-4 );


      unsigned sizex2 = 32;
      unsigned sizey2 = 16;
      Slice t2( nll::core::vector3ui( sizex2, sizey2, 1 ),
                nll::core::vector3f( 1.0f, 0, 0 ),
                nll::core::vector3f( 0, 1.0f, 0 ),
                nll::core::vector3f( 10, 10, 0 ),
                nll::core::vector2f( 2.0f, 1.5f ) );
      const nll::core::vector3f p2( 12, 11.5, 0 );
      const nll::core::vector2f p2t = t2.worldToSliceCoordinate( p2 );
      TESTER_ASSERT( fabs( p2t[ 0 ] - ( 1 ) ) < 1e-4 );
      TESTER_ASSERT( fabs( p2t[ 1 ] - ( 1 ) ) < 1e-4 );
   }

   void testResamplingZoom()
   {
      nll::core::Image<nll::ui8>             im1;
      typedef nll::imaging::Slice<nll::ui8>  Slice;

      nll::core::readBmp( im1, NLL_TEST_PATH "data/image/test-image1.bmp" );

      Slice t1( nll::core::vector3ui( 128, 129, 3 ),
                nll::core::vector3f( 1.0f, 0, 0 ),
                nll::core::vector3f( 0, 1.0f, 0 ),
                nll::core::vector3f( 0, 0, 0 ),
                nll::core::vector2f( 1.0f, 1.0f ) );
      t1.getStorage() = im1;

      Slice t2( nll::core::vector3ui( 128, 129, 3 ),
                nll::core::vector3f( 1.0f, 0, 0 ),
                nll::core::vector3f( 0, 1.0f, 0 ),
                nll::core::vector3f( 0, 0, 0 ),
                nll::core::vector2f( 0.5f, 0.5f ) );

//      nll::imaging::resampling<Slice, Slice::BilinearInterpolator>( t1, t2 );

      nll::core::writeBmp( t2.getStorage(), NLL_TEST_PATH "data/slice-zoom1.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSlice);
TESTER_TEST(testContains);
TESTER_TEST(testCoordinateSystem);
TESTER_TEST(testResamplingZoom);
TESTER_TEST_SUITE_END();
#endif
