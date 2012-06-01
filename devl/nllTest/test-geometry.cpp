#include <nll/nll.h>
#include <tester/register.h>

using namespace nll::core;

namespace nll
{
namespace core
{
   /**
    @brief represents a simple segment in 2D and basic operations

    internally, a line is represented by y = _a * x + _b
    */
   class GeometrySegment2d
   {
   public:
      GeometrySegment2d()
      {}

      GeometrySegment2d( const core::vector2f& p1, const core::vector2f& p2 )
      {
         setGeometry( p1, p2 );
      }

      const core::vector2f& getP1() const
      {
         return _p1;
      }

      const core::vector2f& getP2() const
      {
         return _p2;
      }

      float getA() const
      {
         return _a;
      }

      float getB() const
      {
         return _b;
      }

      void setGeometry( const core::vector2f& p1, const core::vector2f& p2 )
      {
         _p1 = p1;
         _p2 = p2;
         _dirP1P2 = _p2 - _p1;
         _dirP1P2normSquared = _dirP1P2.dot( _dirP1P2 );

         float dx = p1[ 0 ] - p2[ 0 ];
         if ( core::equal( dx, 0.0f ) )
         {
            // we are in trouble here...
            dx = 1e-6f;
         }

         _a = ( p1[ 1 ] - p2[ 1 ] ) / dx;
         _b = p1[ 1 ] - _a * p1[ 0 ];
      }

      bool contains( const core::vector2f& point, const float epsilon = 1e-4 ) const
      {
         // check we are at least on the line so check y == ax + b
         float foundy = _a * point[ 0 ] + _b;
         if ( fabs( foundy - point[ 1 ] ) >= epsilon )
         {
            return false;
         }

         // then check we are within the bounds
         const core::vector2f dstart = point - _p1;
         const float dstartn = dstart.dot( dstart );
         if ( fabs( dstartn ) > _dirP1P2normSquared )
         {
            return false;  // it is too far from p1!!
         }

         const core::vector2f dend = point - _p2;
         const float dendn = dend.dot( dend );
         if ( fabs( dendn ) > _dirP1P2normSquared )
         {
            return false;  // it is too far from p1!!
         }

         // it does contain it!
         return true;
      }

      /**
       @brief Returns one intersection between the segments if possible
       */
      core::vector2f getIntersection( const GeometrySegment2d& segment, bool* found = 0 ) const
      {
         const float diffa = _a - segment._a;
         const float diffb = _b - segment._b;
         if ( core::equal( diffa, 0.0f ) )
         {
            if ( core::equal( diffb, 0.0f ) )
            {
               if ( found )
               {
                  *found = true;
               }

               // the lines are the identical, so any point on one will intersect on the other
               return _p1;
            }
         } else {
            if ( found )
            {
               *found = false;
            }

            // the lines are parallel, there is no solution
            return core::vector2f();
         }

         // here we know the lines will intersect, so find this intersection
         const float x = - diffb / diffa;
         const core::vector2f point( x, _a * x + _b );

         // now check this intersection is on the segment
         if ( found )
         {
            *found = contains( point ) && segment.contains( point );
         }

         return point;
      }

   private:
      core::vector2f    _p1;
      core::vector2f    _p2;
      core::vector2f    _dirP1P2;
      float             _dirP1P2normSquared;

      // line representation y = ax + b
      float _a;
      float _b;
   };

   /**
    @brief Represent a simple 2D box and the most common associated operations
    */
   class GeometryBox2d
   {
   public:
      GeometryBox2d()
      {}

      GeometryBox2d( const core::vector2f& position, const core::vector2f& size )
      {
         setGeometry( position, size );
      }

      void setGeometry( const core::vector2f& position, const core::vector2f& size )
      {
         _cornerMin = position;
         _size = size;
         _cornerMax = position + size;

         // update the segments representing the box
         _bottom.setGeometry( _cornerMin, core::vector2f( _cornerMax[ 0 ], _cornerMin[ 1 ] ) );
         _top.setGeometry( core::vector2f( _cornerMin[ 0 ], _cornerMax[ 1 ] ), _cornerMax );
         _right.setGeometry( core::vector2f( _cornerMax[ 0 ], _cornerMin[ 0 ] ), _cornerMax );
         _left.setGeometry( _cornerMin, core::vector2f( _cornerMin[ 0 ], _cornerMax[ 1 ] ) );
      }

      bool contains( const core::vector2f& position ) const
      {
         return position[ 0 ] >= _cornerMin[ 0 ] &&
                position[ 1 ] >= _cornerMin[ 1 ] &&
                position[ 0 ] <= _cornerMax[ 0 ] &&
                position[ 1 ] <= _cornerMax[ 1 ];
      }

      const core::vector2f& getMinCorner() const
      {
         return _cornerMin;
      }

      const core::vector2f& getMaxCorner() const
      {
         return _cornerMax;
      }

      const core::vector2f& size() const
      {
         return _size;
      }

      /**
       @brief Imagine a segment defined by [p1, p2] such that one point is insed the box, the other is outside.
              This returns the intersection of the segment with the box
       */
      core::vector2f getIntersection( const GeometrySegment2d& segment ) const
      {
         bool isP1Inside = contains( segment.getP1() );
         bool isP2Inside = contains( segment.getP2() );
         ensure( (int)isP1Inside + (int)isP2Inside == 1, "precondition: one of the points must be inside, the other outside the box" );

         bool intersection = false;

         {
            const core::vector2f p = _bottom.getIntersection( segment, &intersection );
            if ( intersection )
               return p;
         }

         {
            const core::vector2f p = _left.getIntersection( segment, &intersection );
            if ( intersection )
               return p;
         }

         {
            const core::vector2f p = _right.getIntersection( segment, &intersection );
            if ( intersection )
               return p;
         }

         {
            const core::vector2f p = _top.getIntersection( segment, &intersection );
            if ( intersection )
               return p;
         }

         ensure( 0, "No intersection" );
      }

   private:
      core::vector2f _cornerMin;
      core::vector2f _cornerMax;

      // tmp variables to speed up computations
      core::vector2f _size;
      GeometrySegment2d _bottom;
      GeometrySegment2d _top;
      GeometrySegment2d _left;
      GeometrySegment2d _right;
   };
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

   void testSegment()
   {
      for ( int n = 0; n < 1000; ++n )
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
         TESTER_ASSERT( equal( s1.getB(), p1[ 1 ] - s1.getA() * p1[ 0 ] ), 1e-3f );
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
TESTER_TEST_SUITE_END();
#endif

