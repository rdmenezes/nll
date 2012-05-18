#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;


class TestTransformationDdf3d
{
public:
   void testSimpleMapping()
   {
      srand( 0 );
      typedef core::Matrix<float>   Matrix;
      Matrix pst = core::createTransformationAffine3D( core::vector3f( 10, 5, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 1, 1, 1 ) );
      Matrix tfm = core::createTransformationAffine3D( core::vector3f( 1, 0, 0 ), core::vector3f( core::PI / 2, 0, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 1, 1, 1 ) );
      core::vector3f sizeMm( 20, 20, 20 );
      core::vector3ui ddfSize( 10, 10, 10 );

      imaging::TransformationDenseDeformableField ddf = imaging::TransformationDenseDeformableField::create( tfm, pst, sizeMm, ddfSize );

      {
         const core::vector3f p = ddf.getStorage().getOrigin();
         const core::vector3f ptfm = ddf.transform(p);
         const core::vector3f ptfmRef = core::transf4( tfm, p );
         TESTER_ASSERT( (ptfmRef-ptfm).norm2() < 1e-4 );    // no DDF = 0, so we expect to be just an affine tfm
      }

      {
         const core::vector3f displacement( -10, 20, 30 );
         ddf.getStorage()( 1, 8, 3 ) = displacement;  // p will be mapped to this index

         const core::vector3f p( 11, 6, -21 );
         const core::vector3f ptfm = ddf.transform(p);
         const core::vector3f ptfmRef = core::transf4( tfm, p ) + displacement;
         TESTER_ASSERT( (ptfmRef-ptfm).norm2() < 1e-4 );    // no DDF = 0, so we expect to be just an affine tfm
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTransformationDdf3d);
TESTER_TEST(testSimpleMapping);
TESTER_TEST_SUITE_END();
#endif