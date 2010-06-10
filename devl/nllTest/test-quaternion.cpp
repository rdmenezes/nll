#include <nll/nll.h>
#include <tester/register.h>

template <class T, class Mapper>
static inline nll::core::Matrix<T, Mapper> extractRotation( const nll::core::Matrix<T, Mapper>& m )
{
   nll::core::Matrix<T, Mapper> res( 3, 3 );
   for ( unsigned ny = 0; ny < 3; ++ny )
      for ( unsigned nx = 0; nx < 3; ++nx )
         res( ny, nx ) = m( ny, nx );
   return res;
}

template <class T, class Mapper>
static inline void setRotation( nll::core::Matrix<T, Mapper>& m, nll::core::Matrix<T, Mapper>& rot )
{
   for ( unsigned ny = 0; ny < 3; ++ny )
      for ( unsigned nx = 0; nx < 3; ++nx )
         m( ny, nx ) = rot( ny, nx );
}

class TestQuaternion
{
public:
   void testMatrixToQuaternion()
   {
      typedef nll::core::Matrix<double, nll::core::IndexMapperRowMajorFlat2D> Matrix;
      for ( unsigned nn = 0; nn < 500; ++nn )
      {
         double a = nll::core::generateUniformDistribution( -3, 3 );
         double rotation[ 3 * 3 ] =
         {
            1,      0,      0,
            0, cos(a), -sin(a),
            0, sin(a), cos(a)
         };
         Matrix rot1( nll::core::Buffer1D<double>( rotation, 3 * 3, false ), 3, 3 );

         double transfBuf[ 4 * 4 ] =
         {
            1, 0, 0, 0,
            0, 1, 0 ,5.5,
            0, 0, 1, 0,
            0, 0, 0, 1
         };
         Matrix trans1( nll::core::Buffer1D<double>( transfBuf, 4 * 4, false ), 4, 4 );

         double transfBuf2[ 4 * 4 ] =
         {
            1, 0, 0, 0,
            0, 1, 0 ,4.5,
            0, 0, 1, 0,
            0, 0, 0, 1
         };
         Matrix trans2( nll::core::Buffer1D<double>( transfBuf2, 4 * 4, false ), 4, 4 );
         setRotation( trans2, rot1 );
         nll::core::inverse( trans1 );

         
         Matrix diff = trans1 * trans2;
         Matrix rot = extractRotation( diff );
         nll::core::Quaternion q( rot );
         double angleFromQuaternion = 2 * acos( q[ 0 ] );

         TESTER_ASSERT( diff( 0, 3 ) == 0 );
         TESTER_ASSERT( diff( 1, 3 ) == -1 );
         TESTER_ASSERT( diff( 2, 3 ) == 0 );
         TESTER_ASSERT( nll::core::equal( fabs( angleFromQuaternion ), fabs( a ), 1e-8 ) );
      }
   }

   // TODO: test all the methods...
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestQuaternion);
TESTER_TEST(testMatrixToQuaternion);
TESTER_TEST_SUITE_END();
#endif
