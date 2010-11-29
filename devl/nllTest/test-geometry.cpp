#include <nll/nll.h>
#include <tester/register.h>

using namespace nll::core;

class TestGeometry
{
public:
   void testColinearity()
   {
      
      TESTER_ASSERT( nll::core::isCollinear( nll::core::vector3f( 1, 1, 1 ),
                                            nll::core::vector3f( -2, -2, -2 ) ) );
      TESTER_ASSERT( nll::core::isCollinear( nll::core::vector3f( 1, 0, 0 ),
                                            nll::core::vector3f( 2, 0, 0 ) ) );
      TESTER_ASSERT( nll::core::isCollinear( nll::core::vector3f( 0, -1, 0 ),
                                            nll::core::vector3f( 0, 1, 0 ) ) );
      TESTER_ASSERT( nll::core::isCollinear( nll::core::vector3f( 0, 0, 0.1f ),
                                            nll::core::vector3f( 0, 0, 0.01f ) ) );

      TESTER_ASSERT( !nll::core::isCollinear( nll::core::vector3f( 0, 0, 1 ),
                                             nll::core::vector3f( 0, 0.001f, 1 ) ) );
      TESTER_ASSERT( !nll::core::isCollinear( nll::core::vector3f( 0, 1.1f, 0 ),
                                             nll::core::vector3f( 0, 0, 1 ) ) );
      TESTER_ASSERT( !nll::core::isCollinear( nll::core::vector3f( 0, 0, 0 ),
                                             nll::core::vector3f( 0, 0, 0 ) ) );
   }

   void testPlaneContains()
   {
      nll::core::GeometryPlane planeId( nll::core::vector3f( 0, 0, 0 ),
                                        nll::core::vector3f( 1, 0, 0 ),
                                        nll::core::vector3f( 0, 1, 0 ) );
      TESTER_ASSERT( planeId.contains( nll::core::vector3f( 0, 0, 0 ) ) );
      TESTER_ASSERT( planeId.contains( nll::core::vector3f( 10, 0, 0 ) ) );
      TESTER_ASSERT( planeId.contains( nll::core::vector3f( 0, 10, 0 ) ) );
      TESTER_ASSERT( !planeId.contains( nll::core::vector3f( 0, 0, 0.1f ) ) );
      TESTER_ASSERT( !planeId.contains( nll::core::vector3f( 0.1f, 0.1f, -0.1f ) ) );

      nll::core::GeometryPlane planeTrans( nll::core::vector3f( 10, 10, 20 ),
                                           nll::core::vector3f( 0, 0, 1 ),
                                           nll::core::vector3f( 0, 1, 0 ) );
      TESTER_ASSERT( planeTrans.contains( nll::core::vector3f( 10, 10, 20 ) ) );
      TESTER_ASSERT( planeTrans.contains( nll::core::vector3f( 10, 10, 21 ) ) );
      TESTER_ASSERT( planeTrans.contains( nll::core::vector3f( 10, 11, 21 ) ) );
      TESTER_ASSERT( !planeTrans.contains( nll::core::vector3f( 9, 10, 21 ) ) );
      TESTER_ASSERT( !planeTrans.contains( nll::core::vector3f( 11, 10, 21 ) ) );
   }

   void testPlaneCoordinates()
   {
      
      // translation
      nll::core::GeometryPlane planeTrans( nll::core::vector3f( 10, 10, 20 ),
                                           nll::core::vector3f( 1, 0, 0 ),
                                           nll::core::vector3f( 0, 0, 1 ) );

      nll::core::vector3f p1( 15, 10, 20 );
      nll::core::vector2f p1t = planeTrans.worldToPlaneCoordinate( p1 );
      TESTER_ASSERT( p1t == nll::core::vector2f( 5, 0 ) );
      TESTER_ASSERT( planeTrans.planeToWorldCoordinate( p1t ) == p1 );

      nll::core::vector3f p2( 15, 10, 26 );
      nll::core::vector2f p2t = planeTrans.worldToPlaneCoordinate( p2 );
      TESTER_ASSERT( p2t == nll::core::vector2f( 5, 6 ) );
      TESTER_ASSERT( planeTrans.planeToWorldCoordinate( p2t ) == p2 );

      // spacing
      nll::core::GeometryPlane planeSpaci( nll::core::vector3f( 10, 10, 20 ),
                                           nll::core::vector3f( 5, 0, 0 ),
                                           nll::core::vector3f( 0, 0, 2 ) );

      nll::core::vector3f p3( 15, 10, 20 );
      nll::core::vector2f p3t = planeSpaci.worldToPlaneCoordinate( p3 );
      TESTER_ASSERT( p3t == nll::core::vector2f( 1, 0 ) );
      TESTER_ASSERT( planeSpaci.planeToWorldCoordinate( p3t ) == p3 );

      nll::core::vector3f p4( 15, 10, 26 );
      nll::core::vector2f p4t = planeSpaci.worldToPlaneCoordinate( p4 );
      TESTER_ASSERT( p4t == nll::core::vector2f( 1, 3 ) );
      TESTER_ASSERT( planeSpaci.planeToWorldCoordinate( p4t ) == p4 );

      // inv
      nll::core::GeometryPlane planeSpacinv( nll::core::vector3f( 0, 0, 0 ),
                                             nll::core::vector3f( 0, -1, 0 ),
                                             nll::core::vector3f( 1, -1, 0 ) );

      nll::core::vector3f p5( 10, 20, 0 );
      nll::core::vector2f p5t = planeSpacinv.worldToPlaneCoordinate( p5 );
      TESTER_ASSERT( fabs( p5t[ 0 ] - ( -30 ) ) < 1e3 );
      TESTER_ASSERT( fabs( p5t[ 1 ] - ( 10 ) ) < 1e3 );
      nll::core::vector3f p5tt = planeSpacinv.planeToWorldCoordinate( p5t );
      TESTER_ASSERT( fabs( p5tt[ 0 ] - p5[ 0 ] ) < 1e3 );
      TESTER_ASSERT( fabs( p5tt[ 1 ] - p5[ 1 ] ) < 1e3 );


      // inv 2
      nll::core::GeometryPlane planeSpacinv2( nll::core::vector3f( 0, 0, 0 ),
                                             nll::core::vector3f( 0, 0, -1 ),
                                             nll::core::vector3f( 0, 1, -1 ) );

      nll::core::vector3f p6( 0, 10, 20 );
      nll::core::vector2f p6t = planeSpacinv2.worldToPlaneCoordinate( p6 );
      TESTER_ASSERT( fabs( p6t[ 0 ] - ( -30 ) ) < 1e3 );
      TESTER_ASSERT( fabs( p6t[ 1 ] - ( 10 ) ) < 1e3 );
      nll::core::vector3f p6tt = planeSpacinv2.planeToWorldCoordinate( p6t );
      TESTER_ASSERT( fabs( p6tt[ 0 ] - p6[ 0 ] ) < 1e3 );
      TESTER_ASSERT( fabs( p6tt[ 1 ] - p6[ 1 ] ) < 1e3 );

      {
         nll::core::GeometryPlane planeSpaci( nll::core::vector3f( 10, 10, 10 ),
                                              nll::core::vector3f( 0.5, 0, 0 ),
                                              nll::core::vector3f( 0, 2, 0 ) );

         nll::core::vector3f p3( 15, 15, 10 );
         nll::core::vector2f p3t = planeSpaci.worldToPlaneCoordinate( p3 );
         TESTER_ASSERT( p3t == nll::core::vector2f( 10, 2.5 ) );
         TESTER_ASSERT( planeSpaci.planeToWorldCoordinate( p3t ) == p3 );

         nll::core::vector3f p4( 15, 26, 10 );
         nll::core::vector2f p4t = planeSpaci.worldToPlaneCoordinate( p4 );
         TESTER_ASSERT( p4t == nll::core::vector2f( 10, 8 ) );
         TESTER_ASSERT( planeSpaci.planeToWorldCoordinate( p4t ) == p4 );
      }
   }

   void testNormalConstructor()
   {
      nll::core::GeometryPlane orthoproj0( nll::core::vector3f( 0, 0, 0 ),
                                          nll::core::vector3f( 0, 1, 0 ) );

      nll::core::GeometryPlane orthoproj1( nll::core::vector3f( 0, 0, 0 ),
                                          nll::core::vector3f( 1, 1, 0 ) );

      nll::core::GeometryPlane orthoproj2( nll::core::vector3f( 10, 10, 10 ),
                                          nll::core::vector3f( 0, 1, -1 ) );
   }

   void testPlaneIntersection()
   {
      nll::core::GeometryPlane orthoproj( nll::core::vector3f( 2, 0, 0 ),
                                          nll::core::vector3f( 0, 1, 0 ),
                                          nll::core::vector3f( 0, 0, 1 ) );

      nll::core::vector3f dir1( -1, 0, 0 );
      nll::core::vector3f p1( 0, 0, 0 );
      nll::core::vector3f out;

      bool projected = orthoproj.getIntersection( p1, dir1, out );
      TESTER_ASSERT( projected );
      TESTER_ASSERT( out == nll::core::vector3f( 2, 0, 0 ) );

      nll::core::vector3f p2( 0, 1, 3 );
      projected = orthoproj.getIntersection( p2, dir1, out );
      TESTER_ASSERT( projected );
      TESTER_ASSERT( out == nll::core::vector3f( 2, 1, 3 ) );

      // test dummy
      nll::core::GeometryPlane planeTrans( nll::core::vector3f( 0, 0, 0 ),
                                           nll::core::vector3f( 0, 1, 0 ),
                                           nll::core::vector3f( 0, 0, 1 ) );
      nll::core::vector3f p3( -10, 5, 5 );
      nll::core::vector3f d2( -1, 0, 0 );
      nll::core::vector2f out2;
      projected = planeTrans.getIntersection( p3, d2, out2 );
      TESTER_ASSERT( projected );
      TESTER_ASSERT( out2 == nll::core::vector2f( 5, 5 ) );
   }

   void testBoxIntersectionSimple()
   {
      nll::core::GeometryBox box1( nll::core::vector3f( 0, 0, 0 ),
                                   nll::core::vector3f( 10, 20, 30 ) );

      nll::core::vector3f p1( -10, 5, 5 );
      nll::core::vector3f d1( 1, 0, 0 );

      nll::core::vector3f r1, r2;
      bool intersected = box1.getIntersection( p1, d1, r1, r2 );
      TESTER_ASSERT( intersected );
   }

   void testBoxContainsSimple()
   {
      nll::core::GeometryBox box1( nll::core::vector3f( 0, 0, 0 ),
                                   nll::core::vector3f( 10, 20, 30 ) );

      TESTER_ASSERT( box1.contains( nll::core::vector3f( 5, 5, 5 ) ) );
      TESTER_ASSERT( !box1.contains( nll::core::vector3f( -5, 5, 5 ) ) );
      TESTER_ASSERT( !box1.contains( nll::core::vector3f( 5, -5, 5 ) ) );
      TESTER_ASSERT( !box1.contains( nll::core::vector3f( 5, 5, -5 ) ) );
      TESTER_ASSERT( !box1.contains( nll::core::vector3f( 5, 50, 5 ) ) );
      TESTER_ASSERT( !box1.contains( nll::core::vector3f( 5, 5, 50 ) ) );
      TESTER_ASSERT( !box1.contains( nll::core::vector3f( 50, 5, 5 ) ) );
   }

   void testGeometryReg1()
   {
      {
         // simple translation
         nll::core::Matrix<float> pst = nll::core::identityMatrix<nll::core::Matrix<float> >( 4 );
         pst( 0, 3 ) = 1;
         pst( 1, 3 ) = 2;
         pst( 2, 3 ) = 3;
         nll::core::Matrix<float> tfm = nll::core::identityMatrix<nll::core::Matrix<float> >( 4 );
         tfm( 0, 3 ) = 10;
         tfm( 1, 3 ) = 20;
         tfm( 2, 3 ) = 30;

         VolumeGeometry geom( pst, tfm );
         nll::core::vector3f c1 = geom.indexToPosition( nll::core::vector3f( 0, 0, 0 ) );
         TESTER_ASSERT( c1 == nll::core::vector3f( 11, 22, 33 ) );
      }

      {
         // PST T R
         nll::core::Matrix<float> pst;
         nll::core::matrix4x4RotationZ( pst, nll::core::PI / 2 );
         pst( 0, 3 ) = 1;

         VolumeGeometry geom( pst );
         nll::core::vector3f c1 = geom.indexToPosition( nll::core::vector3f( 0, 0, 0 ) );
         nll::core::vector3f c2 = geom.indexToPosition( nll::core::vector3f( 1, 0, 0 ) );
         TESTER_ASSERT( c1 == nll::core::vector3f( 1, 0, 0 ) );
         TESTER_ASSERT( c2 == nll::core::vector3f( 1, 1, 0 ) );
      }

      {
         // PST T R
         nll::core::Matrix<float> pst;
         nll::core::matrix4x4RotationZ( pst, nll::core::PI / 2 );
         pst( 0, 3 ) = 1;

         nll::core::Matrix<float> tfm = nll::core::identityMatrix<nll::core::Matrix<float> >( 4 );
         tfm( 0, 3 ) = 10;

         VolumeGeometry geom( pst, tfm );
         nll::core::vector3f c1 = geom.indexToPosition( nll::core::vector3f( 0, 0, 0 ) );
         nll::core::vector3f c2 = geom.indexToPosition( nll::core::vector3f( 1, 0, 0 ) );
         TESTER_ASSERT( c1 == nll::core::vector3f( 11, 0, 0 ) );
         TESTER_ASSERT( c2 == nll::core::vector3f( 11, 1, 0 ) );
      }

      {
         // PST ID TFM R T
         nll::core::Matrix<float> pst = nll::core::identityMatrix<nll::core::Matrix<float> >( 4 );

         nll::core::Matrix<float> tfm;
         nll::core::matrix4x4RotationZ( tfm, nll::core::PI / 2 );
         tfm( 0, 3 ) = 10;

         VolumeGeometry geom( pst, tfm );
         nll::core::vector3f c1 = geom.indexToPosition( nll::core::vector3f( 0, 0, 0 ) );
         nll::core::vector3f c2 = geom.indexToPosition( nll::core::vector3f( 1, 0, 0 ) );
         TESTER_ASSERT( c1 == nll::core::vector3f( 10, 0, 0 ) );
         TESTER_ASSERT( c2 == nll::core::vector3f( 10, 1, 0 ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGeometry);
TESTER_TEST(testColinearity);
TESTER_TEST(testPlaneContains);
TESTER_TEST(testPlaneIntersection);
TESTER_TEST(testNormalConstructor);
TESTER_TEST(testBoxIntersectionSimple);
TESTER_TEST(testBoxContainsSimple);
TESTER_TEST(testGeometryReg1);
TESTER_TEST(testPlaneCoordinates);
TESTER_TEST_SUITE_END();
#endif

