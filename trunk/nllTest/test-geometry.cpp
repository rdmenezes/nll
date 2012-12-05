#include <nll/nll.h>
#include <tester/register.h>

using namespace nll::core;

namespace nll
{
namespace core
{
  
}
}

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
      TESTER_ASSERT( nll::core::isCollinear( nll::core::vector3f( 0, 0, 0 ),
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

   void testSegment()
   {
      srand(0);
      for ( int n = 0; n < 10000; ++n )
      {
         vector2f p1( generateUniformDistributionf( -100, 100 ), 
                      generateUniformDistributionf( -100, 100 ) );
         vector2f p2( generateUniformDistributionf( -100, 100 ), 
                      generateUniformDistributionf( -100, 100 ) );
         if ( equal( p1[ 1], p2[ 1 ] ) )
            continue;

         // test the internals
         GeometrySegment2d s1( p1, p2 );
         TESTER_ASSERT( equal( s1.getA(), ( p2[ 1 ] - p1[ 1 ] ) / ( p2[ 0 ] - p1[ 0 ] ), 1e-3f ) );
         TESTER_ASSERT( equal( s1.getB(), p1[ 1 ] - s1.getA() * p1[ 0 ], 1e-3f ) );

         // test contains or not...
         const float x = generateUniformDistributionf( -100, 100 );
         const float y = s1.getA() * x + s1.getB();
         const float d1 = ( p1 - vector2f( x, y ) ).norm2();
         const float d2 = ( p2 - vector2f( x, y ) ).norm2();
         const float length = ( p1 - p2 ).norm2();
         const bool expectedInside = d1 <= length && d2 <= length;
         if ( s1.contains( vector2f( x, y ), 1e-3 ) != expectedInside )
         {
            std::cout << "x=" << x << " y=" << y << " expectedInside=" << expectedInside << s1.getP1() << s1.getP2();
         }
         ensure( s1.contains( vector2f( x, y ),  0.01 ) == expectedInside, "arg1!" );

         // this one will never contain it..
         if ( fabs( s1.getA() ) > 1e-3 )
         {
            const vector2f normal( -1, s1.getA() );
            const vector2f p( x + normal[ 0 ] * 10, y + normal[ 1 ] * 10 );
            ensure( !s1.contains( p, 0.01 ), "arg2!" );
         }
      }
   }

   void testSegmentProjection()
   {
      srand( 0 );
      for ( int n = 0; n < 10000; ++n )
      {
         vector2f p1( generateUniformDistributionf( -100, 100 ), 
                      generateUniformDistributionf( -100, 100 ) );
         vector2f p2( generateUniformDistributionf( -100, 100 ), 
                      generateUniformDistributionf( -100, 100 ) );
         vector2f p1a( generateUniformDistributionf( -100, 100 ), 
                       generateUniformDistributionf( -100, 100 ) );

         GeometrySegment2d s1( p1, p2 );

         const vector2f proj = s1.getOrthogonalProjection( p1a );
         const float d1 = ( proj - p1 ).norm2();
         const float d2 = ( proj - p1a ).norm2();
         const float dh = ( p1 - p1a ).norm2();
         const float tolerance = 0.001 * sqr( dh );
         ensure( equal<float>( sqr( dh ), sqr( d1 ) + sqr( d2 ), tolerance ), "square triangle dh^2 = d1^2 + d2^2" );
      }
   }

   void testSegmentIntersection()
   {
      for ( int n = 0; n < 10000; ++n )
      {
         vector2f p1( generateUniformDistributionf( -100, 100 ), 
                      generateUniformDistributionf( -100, 100 ) );
         vector2f p2( generateUniformDistributionf( -100, 100 ), 
                      generateUniformDistributionf( -100, 100 ) );

         vector2f p1a( generateUniformDistributionf( -100, 100 ), 
                       generateUniformDistributionf( -100, 100 ) );
         vector2f p2a( generateUniformDistributionf( -100, 100 ), 
                       generateUniformDistributionf( -100, 100 ) );

         GeometrySegment2d s1( p1, p2 );
         GeometrySegment2d s2( p1a, p2a );

         vector2f i;
         const bool good = s1.getIntersection( s2, i );

         const bool expectedIntersection = s1.contains( i ) && s2.contains( i );
         ensure( good == expectedIntersection, "arg!" );
      }
   }

   void testBox()
   {
      for ( int n = 0; n < 10000; ++n )
      {
         vector2f p1( generateUniformDistributionf( -100, 100 ), 
                      generateUniformDistributionf( -100, 100 ) );
         vector2f size( generateUniformDistributionf( -100, 100 ), 
                        generateUniformDistributionf( -100, 100 ) );
         GeometryBox2d box( p1, size );


         vector2f p( generateUniformDistributionf( -100, 100 ), 
                     generateUniformDistributionf( -100, 100 ) );
         const bool expectedInside = p[ 0 ] >= p1[ 0 ] &&
                                     p[ 1 ] >= p1[ 1 ] &&
                                     p[ 0 ] <= p1[ 0 ] + size[ 0 ] &&
                                     p[ 1 ] <= p1[ 1 ] + size[ 1 ];
         ensure( expectedInside == box.contains( p ), "arg!" );
      }
   }

   void testBoxIntersection()
   {
      const vector2f pos( 1, 2 );
      const vector2f size( 4, 5 );
      const GeometryBox2d box( pos, size );

      {
         const vector2f p1( 0, 3 );
         const vector2f p2( 2, 3.5 );
         GeometrySegment2d segment( p1, p2 );

         vector2f i;
         const bool intersection = box.getIntersection( segment, i );
         ensure( intersection, "bad!" );
         ensure( equal<float>( i[ 0 ], 1 ), "arg!" );
         ensure( equal<float>( i[ 1 ], 3.25 ), "arg!" );
      }

      {
         const vector2f p1( 3, 4 );
         const vector2f p2( 8, 4 );
         GeometrySegment2d segment( p1, p2 );

         vector2f i;
         const bool intersection = box.getIntersection( segment, i );
         ensure( intersection, "bad!" );
         ensure( equal<float>( i[ 0 ], 5 ), "arg!" );
         ensure( equal<float>( i[ 1 ], 4 ), "arg!" );
      }

      {
         const vector2f p1( 3, 0 );
         const vector2f p2( 3, 4 );
         GeometrySegment2d segment( p1, p2 );

         vector2f i;
         const bool intersection = box.getIntersection( segment, i );
         ensure( intersection, "bad!" );
         ensure( equal<float>( i[ 0 ], 3 ), "arg!" );
         ensure( equal<float>( i[ 1 ], 2 ), "arg!" );
      }

      {
         const vector2f p1( 3, 8 );
         const vector2f p2( 3, 4 );
         GeometrySegment2d segment( p1, p2 );

         vector2f i;
         const bool intersection = box.getIntersection( segment, i );
         ensure( intersection, "bad!" );
         ensure( equal<float>( i[ 0 ], 3 ), "arg!" );
         ensure( equal<float>( i[ 1 ], 7 ), "arg!" );
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
TESTER_TEST(testPlaneCoordinates);
TESTER_TEST(testSegment);
TESTER_TEST(testSegmentProjection);
TESTER_TEST(testSegmentIntersection);
TESTER_TEST(testBox);
TESTER_TEST(testBoxIntersection);
TESTER_TEST_SUITE_END();
#endif

