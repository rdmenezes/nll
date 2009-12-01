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
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSlice);
TESTER_TEST(testContains);
TESTER_TEST_SUITE_END();
#endif
