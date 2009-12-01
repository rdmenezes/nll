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
                nll::core::vector3f( 1.0f, 0, 0),
                nll::core::vector3f( 0, 1.0f, 0),
                nll::core::vector3f( 0, 0, 0),
                nll::core::vector2f( 1.0f, 1.0f ) );

      TESTER_ASSERT( t1.contains( nll::core::vector3f( 0, 0, 0 ) ) );
      TESTER_ASSERT( t1.contains( nll::core::vector3f( sizex, 0, 0 ) ) );
      TESTER_ASSERT( t1.contains( nll::core::vector3f( sizex, sizey, 0 ) ) );

      TESTER_ASSERT( !t1.contains( nll::core::vector3f( sizex + 1, 0, 0 ) ) );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSlice);
TESTER_TEST(testContains);
TESTER_TEST_SUITE_END();
#endif
