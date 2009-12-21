#ifndef NLL_CORE_GEOMETRY_H_
# define NLL_CORE_GEOMETRY_H_

namespace nll
{
namespace core
{
   namespace impl
   {
   }

   /**
    @ingroup core
    @brief Defines common operations on a plane
    */
   class GeometryPlane
   {
   public:
      typedef core::Matrix<float>   Matrix;

   public:
      GeometryPlane()
      {
         // non defined plane
      }

      /**
       @brief Defines a plane with an origin and a base
       @note norm(axisx) & norm(axisy) can be different than 1
       */
      GeometryPlane( const core::vector3f& origin,
                     const core::vector3f& axisx,
                     const core::vector3f& axisy ) : _origin( origin ), _axisx( axisx ), _axisy( axisy )
      {
         // computes the normal
         _orthonorm = core::cross( _axisx, _axisy );
         _orthonorm /= static_cast<float>( _orthonorm.norm2() );

         // computes constant for plane a * x + b * y + c * z + planed = 0
         _planed = - _origin.dot( _orthonorm );

         // just init the matrix
         _inverseIntersection = Matrix( 3, 3 );
      }

      /**
       @brief Creates a plane with an origin and a normal vector.
       @note the base will be computed from (normal, origin). The norm of the normal
             will not influence the base chosen
       */
      GeometryPlane( const core::vector3f& origin,
                     const core::vector3f& normal ) : _origin( origin )
      {
         // normalize the normal
         float norm = static_cast<float>( normal.norm2() );
         if ( core::equal<double>( norm, 0, 1e-5f ) )
            throw std::exception( "error: normal is null" );
         _orthonorm = normal / norm;

         // computes constant for plane a * x + b * y + c * z + planed = 0
         _planed = - _origin.dot( _orthonorm );

         // just init the matrix
         _inverseIntersection = Matrix( 3, 3 );

         //
         // compute a base
         // nx * x + ny * y + nz * z + d = 0
         // try x = origin_x + 1, y = origin_y, compute z -> we have another point on the plane...
         //
         std::vector< vector3f > points;
         if ( normal[ 2 ] )
         {
            {
               float dx = _orthonorm[ 0 ] * ( origin[ 0 ] + 1 );
               float dy = _orthonorm[ 1 ] * ( origin[ 1 ] + 0 );
               float dz = ( _planed - dx - dy ) / normal[ 2 ];
               points.push_back( vector3f( origin[ 0 ] + 1, origin[ 1 ], dz ) );
            }

            {
               float dx = _orthonorm[ 0 ] * ( origin[ 0 ] + 0 );
               float dy = _orthonorm[ 1 ] * ( origin[ 1 ] + 1 );
               float dz = ( _planed - dx - dy ) / normal[ 2 ];
               points.push_back( vector3f( origin[ 0 ] + 0, origin[ 1 ] + 1, dz ) );
            }
         }
         if ( normal[ 1 ] )
         {
            {
               float dx = _orthonorm[ 0 ] * ( origin[ 0 ] + 0 );
               float dz = _orthonorm[ 2 ] * ( origin[ 2 ] + 1 );
               float dy = ( _planed - dx - dz ) / normal[ 1 ];
               points.push_back( vector3f( origin[ 0 ] + 0, dy, origin[ 2 ] + 1 ) );
            }

            {
               float dx = _orthonorm[ 0 ] * ( origin[ 0 ] + 1 );
               float dz = _orthonorm[ 2 ] * ( origin[ 2 ] + 0 );
               float dy = ( _planed - dx - dz ) / normal[ 1 ];
               points.push_back( vector3f( origin[ 0 ] + 1, dy, origin[ 2 ] + 0 ) );
            }
         }
         if ( normal[ 0 ] )
         {
            {
               float dy = _orthonorm[ 1 ] * ( origin[ 1 ] + 1 );
               float dz = _orthonorm[ 2 ] * ( origin[ 2 ] + 0 );
               float dx = ( _planed - dy - dz ) / normal[ 0 ];
               points.push_back( vector3f( dx, origin[ 1 ] + 1, origin[ 2 ] ) );
            }

            {
               float dy = _orthonorm[ 1 ] * ( origin[ 1 ] + 0 );
               float dz = _orthonorm[ 2 ] * ( origin[ 2 ] + 1 );
               float dx = ( _planed - dy - dz ) / normal[ 0 ];
               points.push_back( vector3f( dx, origin[ 1 ], origin[ 2 ] + 1) );
            }
         }
         
         // select 2 non colinear vectors and normalize them
         i32 index = -1;
         for ( i32 n = 0; n < static_cast<i32>( points.size() ); ++n )
         {
            if ( origin != points[ n ] )
            {
               vector3f p = points[ n ] - origin;
               p /= static_cast<float>( p.norm2() );
               _axisx = p;
               index = n;
               break;
            }
         }

         for ( i32 n = 0; n < static_cast<i32>( points.size() ); ++n )
         {
            if ( index != n && origin != points[ n ] )
            {
               vector3f p = points[ n ] - origin;
               p /= static_cast<float>( p.norm2() );
               _axisy = p;
               break;
            }
         }

         assert( _axisx.norm2() && _axisy.norm2() );
         _axisx /= static_cast<float>( _axisx.norm2() );
         _axisy /= static_cast<float>( _axisy.norm2() );
      }

      /**
       @brief Returns the normal with norm( normal ) = 1
       */
      const core::vector3f& getNormal() const
      {
         return _orthonorm;
      }

      /**
       @brief Returns the x-axis of the slice
       */
      const core::vector3f& getAxisX() const
      {
         return _axisx;
      }

      /**
       @brief Returns the y-axis of the slice
       */
      const core::vector3f& getAxisY() const
      {
         return _axisy;
      }

      /**
       @brief Returns the origin
       */
      const core::vector3f& getOrigin() const
      {
         return _origin;
      }

      /**
       @brief Returns true if the point, in world coordinate (in mm) is in the same plane than
              the slice.
       */
      bool contains( const core::vector3f pos, float tol = 1e-4 ) const
      {
         // check first it is in the same plan
         // N . x + d = 0 means we are in the plan
         return ( fabs( _orthonorm.dot( pos ) + _planed ) < tol );
      }

      /**
       @brief Transform a world coordinate (standard x, y, z coordinate system, in mm) to slice coordinate
       @param v a position in world coordinate in mm. It must be located on the plane of the slice!
       */
      core::vector2f worldToPlaneCoordinate( const core::vector3f& v ) const
      {
         if ( !contains( v ) )
            throw std::exception( "error: the point is not on the plane" );

         // Let's have M a point on the slice plane with coordinate (x, y, z), O the the origin of the world U=(1, 0, 0) V=(0, 1, 0) W=(0, 0, 1) a base of the world coordinate system
         // O' origin of the slice, S and T base vectors of the slice. We are looking for (X, Y) coordinate in
         // slice coordinate system.

         // We have: 
         //   S = ax1 * U + ay1 * V + az1 * W
         //   T = ax2 * U + ay2 * V + az2 * W
         //   (ax1,ax2,ay1,ay2,az1,az2) are known
         // 
         // We also have:
         //   OM = OO' + O'M
         //
         //   OM = O' + X * S + Y * T
         //      = O' + X * ( ax1 * U + ay1 * V + az1 * W ) + Y * ( ax2 * U + ay2 * V + az2 * W )
         //
         //   { x = ox' + X * ax1 + Y * ax2
         //   { y = oy' + X * ay1 + Y * ay2
         //   { z = oz' + X * az1 + Y * az2
         //
         // S != 0, meaning ax1 | ay1 | az1 != 0, let's assume ax1 != 0 (we can choose ay1, az1)
         //   X = ( x - ox' - Y * ax2 ) / ax1
         //
         // S and T are not colinear AND T != 0, assuming ay2 != 0 (else we can choose ax2, az2)
         //   Y = ( y - oy' - X * ay1 ) / ay2
         //   Y = ( y - oy' - ay1 * ( x - ox' - Y * ax2 ) / ax1 ) / ay2
         //   Y = ay1 * ax2 * Y / ( ay2 * ax1 ) + ( y - oy' - ay1 * ( x - ox') / ax1 ) / ay2
         //   Y * ( 1 - ay1 * ax2 / ( ay2 * ax1 ) ) = ( y - oy' - ay1 * ( x - ox') / ax1 ) / ay2
         //   Y = ( y - oy' - ay1 * ( x - ox') / ax1 ) / ay2 / ( 1 - ay1 * ax2 / ( ay2 * ax1 ) )
   
         // init the data and it's index
         float m[ 3 ][ 4 ] =
         {
            { v[ 0 ], _origin[ 0 ], _axisx[ 0 ], _axisy[ 0 ] },
            { v[ 1 ], _origin[ 1 ], _axisx[ 1 ], _axisy[ 1 ] },
            { v[ 2 ], _origin[ 2 ], _axisx[ 2 ], _axisy[ 2 ] }
         };

         int i0 = -1;
         for ( ui32 n = 0; n < 3; ++n )
            if ( _axisx[ n ] != 0 )
            {
               i0 = n;
               break;
            }
         int i1 = -1;
         for ( int n = 0; n < 3; ++n )
            if ( _axisy[ n ] != 0 && n != i0 )
            {
               i1 = n;
               break;
            }
         if ( i0 == -1 || i1 == -1 )
            throw std::exception( "error: the slice base is invalid" );

         // create some alias for code readability
         const float x   = m[ i0 ][ 0 ];
         const float ox  = m[ i0 ][ 1 ];
         const float ax1 = m[ i0 ][ 2 ];
         const float ax2 = m[ i0 ][ 3 ];

         const float y   = m[ i1 ][ 0 ];
         const float oy  = m[ i1 ][ 1 ];
         const float ay1 = m[ i1 ][ 2 ];
         const float ay2 = m[ i1 ][ 3 ];

         if ( ( ay2 * ax1 - ay1 * ax2 ) == 0 )
            throw std::exception( "error: base is not valid, vectors are colinear" );

         const float Y = ( y - oy - ay1 * ( x - ox ) / ax1 ) / ( ay2 * ( 1 - ay1 * ax2 / ( ay2 * ax1 ) ) );
         const float X = ( x - ox - Y * ax2 ) / ax1;
         return core::vector2f( X, Y );
      }

      /**
       @brief Transform the slice coordinate to a 3D point in world coordinate (in mm)
       */
      core::vector3f planeToWorldCoordinate( const core::vector2f& v ) const
      {
         return _axisx * v[ 0 ] + _axisy * v[ 1 ] + _origin;
      }

      /**
       @brief Compute the intersection of the plane with a line defined by (position, direction).
       @param outIntersection returns the intersection in world coordinate
       */
      bool getIntersection( const core::vector3f& p, const core::vector3f& dir, core::vector3f& outIntersection ) const
      {
         // X = p + dir * t
         // X = u * ax + v * ay + orig

         // ox + dirx * t = ux * ax + vx * ay + origx
         // oy + diry * t = uy * ax + vy * ay + origy
         // oz + dirz * t = uz * ax + vz * ay + origz

         // ox - origx = -dirx * t + ax * ux + ay * vx
         // oy - origy = -diry * t + ax * uy + ay * vy
         // oz - origz = -dirz * t + ax * uz + ay * vz

         // | ox - origx |   | -dirx ux vx |   | t  |
         // | oy - origx | = | -diry uy vy | * | ax |
         // | oz - origx |   | -dirz uz vz |   | ay |

         // | -dirx ux vx |^-1   | ox - origx |   | t  |
         // | -diry uy vy |    . | oy - origx | = | ax |
         // | -dirz uz vz |      | oz - origx |   | ay |

         // we are caching the presious result, and check against it
         if ( p != _lastOrigin || dir != _lastDir )
         {
            _lastDir = dir;
            _lastOrigin = p;
            _inverseIntersection( 0, 0 ) = - dir[ 0 ];
            _inverseIntersection( 1, 0 ) = - dir[ 1 ];
            _inverseIntersection( 2, 0 ) = - dir[ 2 ];

            _inverseIntersection( 0, 1 ) = _axisx[ 0 ];
            _inverseIntersection( 1, 1 ) = _axisx[ 1 ];
            _inverseIntersection( 2, 1 ) = _axisx[ 2 ];

            _inverseIntersection( 0, 2 ) = _axisy[ 0 ];
            _inverseIntersection( 1, 2 ) = _axisy[ 1 ];
            _inverseIntersection( 2, 2 ) = _axisy[ 2 ];

            bool inversed = core::inverse3x3( _inverseIntersection );
            if ( !inversed )
            {
               return false;
            }
         }

         vector3f tmpPoint( p[ 0 ] - _origin[ 0 ],
                            p[ 1 ] - _origin[ 1 ],
                            p[ 2 ] - _origin[ 2 ] );
         core::vector3f res = core::mat3Mulv( _inverseIntersection, tmpPoint );
         outIntersection[ 0 ] = p[ 0 ] + dir[ 0 ] * res[ 0 ];
         outIntersection[ 1 ] = p[ 1 ] + dir[ 1 ] * res[ 0 ];
         outIntersection[ 2 ] = p[ 2 ] + dir[ 2 ] * res[ 0 ];
         return true;
      }

      /**
       @brief Compute the intersection of the plane with a line defined by (position, direction).
       @param outIntersection returns the intersection in slice coordinate
       */
      bool getIntersection( const core::vector3f& p, const core::vector3f& dir, core::vector2f& outIntersection ) const
      {
         // X = p + dir * t
         // X = u * ax + v * ay + orig

         // ox + dirx * t = ux * ax + vx * ay + origx
         // oy + diry * t = uy * ax + vy * ay + origy
         // oz + dirz * t = uz * ax + vz * ay + origz

         // ox - origx = -dirx * t + ax * ux + ay * vx
         // oy - origy = -diry * t + ax * uy + ay * vy
         // oz - origz = -dirz * t + ax * uz + ay * vz

         // | ox - origx |   | -dirx ux vx |   | t  |
         // | oy - origx | = | -diry uy vy | * | ax |
         // | oz - origx |   | -dirz uz vz |   | ay |

         // | -dirx ux vx |^-1   | ox - origx |   | t  |
         // | -diry uy vy |    . | oy - origx | = | ax |
         // | -dirz uz vz |      | oz - origx |   | ay |

         // we are caching the previous result, and check against it next time...
         if ( p != _lastOrigin || dir != _lastDir )
         {
            _lastDir = dir;
            _lastOrigin = p;
            _inverseIntersection( 0, 0 ) =  - dir[ 0 ];
            _inverseIntersection( 1, 0 ) =  - dir[ 1 ];
            _inverseIntersection( 2, 0 ) =  - dir[ 2 ];

            _inverseIntersection( 0, 1 ) =  _axisx[ 0 ];
            _inverseIntersection( 1, 1 ) =  _axisx[ 1 ];
            _inverseIntersection( 2, 1 ) =  _axisx[ 2 ];

            _inverseIntersection( 0, 2 ) =  _axisy[ 0 ];
            _inverseIntersection( 1, 2 ) =  _axisy[ 1 ];
            _inverseIntersection( 2, 2 ) =  _axisy[ 2 ];

            //bool inversed = core::inverse( _inverseIntersection );
            bool inversed = core::inverse3x3( _inverseIntersection );
            if ( !inversed )
            {
               return false;
            }
         }

         vector3f tmpPoint( p[ 0 ] - _origin[ 0 ],
                            p[ 1 ] - _origin[ 1 ],
                            p[ 2 ] - _origin[ 2 ] );
         core::vector3f res = core::mat3Mulv( _inverseIntersection, tmpPoint );
         outIntersection[ 0 ] = res[ 1 ];
         outIntersection[ 1 ] = res[ 2 ];
         return true;
      }

      /**
       @brief Returns true if the point <code>p</code> is in the area defined by min and max
       */
      bool isInArea( const vector2f& min, const vector2f& max, const vector2f& p ) const
      {
         return p[ 0 ] >= min[ 0 ] &&
                p[ 0 ] <= max[ 0 ] &&
                p[ 1 ] >= min[ 1 ] &&
                p[ 1 ] <= max[ 1 ];
      }

   private:
      core::vector3f _origin;
      core::vector3f _axisx;
      core::vector3f _axisy;
      core::vector3f _orthonorm;

      // cached intermediate results but constant
      float          _planed;

      // cached and non constant
      mutable core::vector3f _lastDir;
      mutable core::vector3f _lastOrigin;
      mutable Matrix         _inverseIntersection;
   };

   /**
    @ingroup core
    @brief Contains common operations for box geometry
    */
   class GeometryBox
   {
   public:
      /**
       @brief Construct a box defined by a (min, max) point in standard coordinate system (mm)
       @note in the case the box is not defined in standard coordinate system, just conver the point to the same
             coordinate system.
       */
      GeometryBox( const vector3f& min, const vector3f& max ) : _min( min ), _max( max ), _size( max[ 0 ] - min[ 0 ],
                                                                                                 max[ 1 ] - min[ 1 ],
                                                                                                 max[ 2 ] - min[ 2 ] )
      {
         assert( max[ 0 ] > min[ 0 ] );
         assert( max[ 1 ] > min[ 1 ] );
         assert( max[ 2 ] > min[ 2 ] );
         _initPlanes( min, max );
      }

      /**
       @brief Finds the intersection between the box and a ray. A ray must go in and out with exactly 2 intersections
       @param pos the starting position of the ray in mm in patient space
       @param dir a direction for the ray
       @param outIntersectionBegin the first intersection (it doesn't depend on the ray direction/position)
       @param outIntersectionEnd the first intersection (it doesn't depend on the ray direction/position)
       @return true if intersection between the box & (origin, direction). If true, update <code>outIntersection*</code>
       */
      bool getIntersection( const vector3f& pos, const vector3f& dir, vector3f& outIntersectionBegin, vector3f& outIntersectionEnd ) const
      {
         std::vector< vector3f > intersections;

         core::vector2f intersection;
         if ( _planes[ 0 ].getIntersection( pos,  dir, intersection ) &&
              intersection[ 0 ] >= 0 && intersection[ 0 ] <= _size[ 0 ] &&
              intersection[ 1 ] >= 0 && intersection[ 1 ] <= _size[ 1 ] )
         {
            intersections.push_back( vector3f( _min[ 0 ] + intersection[ 0 ],
                                               _min[ 1 ] + intersection[ 1 ],
                                               _min[ 2 ]  ) );
         }

         if ( _planes[ 1 ].getIntersection( pos,  dir, intersection ) &&
              intersection[ 0 ] >= 0 && intersection[ 0 ] <= _size[ 0 ] &&
              intersection[ 1 ] >= 0 && intersection[ 1 ] <= _size[ 2 ] )
         {
            intersections.push_back( vector3f( _min[ 0 ] + intersection[ 0 ],
                                               _min[ 1 ],
                                               _min[ 2 ] + intersection[ 1 ] ) );
         }

         if ( _planes[ 2 ].getIntersection( pos,  dir, intersection ) &&
              intersection[ 0 ] >= 0 && intersection[ 0 ] <= _size[ 1 ] &&
              intersection[ 1 ] >= 0 && intersection[ 1 ] <= _size[ 2 ] )
         {
            intersections.push_back( vector3f( _min[ 0 ],
                                               _min[ 1 ] + intersection[ 0 ],
                                               _min[ 2 ] + intersection[ 1 ] ) );
         }

         if ( _planes[ 3 ].getIntersection( pos,  dir, intersection ) &&
              intersection[ 0 ] >= 0 && intersection[ 0 ] <= _size[ 0 ] &&
              intersection[ 1 ] >= 0 && intersection[ 1 ] <= _size[ 1 ] )
         {
            intersections.push_back( vector3f( _maxZ[ 0 ] + intersection[ 0 ],
                                               _maxZ[ 1 ] + intersection[ 1 ],
                                               _maxZ[ 2 ] ) );
         }

         if ( _planes[ 4 ].getIntersection( pos,  dir, intersection ) &&
              intersection[ 0 ] >= 0 && intersection[ 0 ] <= _size[ 0 ] &&
              intersection[ 1 ] >= 0 && intersection[ 1 ] <= _size[ 2 ] )
         {
            intersections.push_back( vector3f( _maxY[ 0 ] + intersection[ 0 ],
                                               _maxY[ 1 ],
                                               _maxY[ 2 ] + intersection[ 1 ] ) );
         }

         if ( _planes[ 5 ].getIntersection( pos,  dir, intersection ) &&
              intersection[ 0 ] >= 0 && intersection[ 0 ] <= _size[ 1 ] &&
              intersection[ 1 ] >= 0 && intersection[ 1 ] <= _size[ 2 ] )
         {
            intersections.push_back( vector3f( _maxX[ 0 ],
                                               _maxX[ 1 ] + intersection[ 0 ],
                                               _maxX[ 2 ] + intersection[ 1 ] ) );
         }

         // update intersections
         if ( intersections.size() == 2 )
         {
            outIntersectionBegin = intersections[ 0 ];
            outIntersectionEnd = intersections[ 1 ];
            return true;
         } else return false;
      }

      bool contains( const vector3f& p ) const
      {
         return p[ 0 ] >= _min[ 0 ] && p[ 0 ] < _max[ 0 ] &&
                p[ 1 ] >= _min[ 1 ] && p[ 1 ] < _max[ 1 ] &&
                p[ 2 ] >= _min[ 2 ] && p[ 2 ] < _max[ 2 ];
      }

   private:
      void _initPlanes( const vector3f& min, const vector3f& max )
      {
         _maxZ = core::vector3f( min[ 0 ], min[ 1 ], max[ 2 ] );
         _maxY = core::vector3f( min[ 0 ], max[ 1 ], min[ 2 ] );
         _maxX = core::vector3f( max[ 0 ], min[ 1 ], min[ 2 ] );
         _planes[ 0 ] = GeometryPlane( min, vector3f( 1, 0, 0 ),
                                            vector3f( 0, 1, 0 ) );
         _planes[ 1 ] = GeometryPlane( min, vector3f( 1, 0, 0 ),
                                            vector3f( 0, 0, 1 ) );
         _planes[ 2 ] = GeometryPlane( min, vector3f( 0, 1, 0 ),
                                            vector3f( 0, 0, 1 ) );
         _planes[ 3 ] = GeometryPlane( _maxZ, vector3f( 1, 0, 0 ),
                                              vector3f( 0, 1, 0 ) );
         _planes[ 4 ] = GeometryPlane( _maxY, vector3f( 1, 0, 0 ),
                                              vector3f( 0, 0, 1 ) );
         _planes[ 5 ] = GeometryPlane( _maxX, vector3f( 0, 1, 0 ),
                                              vector3f( 0, 0, 1 ) );
      }

   private:
      vector3f    _min;
      vector3f    _max;
      vector3f    _size;

      vector3f    _maxX;
      vector3f    _maxY;
      vector3f    _maxZ;

      GeometryPlane  _planes[ 6 ];
   };
}
}

#endif