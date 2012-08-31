/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_TRANSFORMATION_AFFINE_2D_H_
# define NLL_TRANSFORMATION_AFFINE_2D_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief returns true if there is a scaling component in a 4x4 homogenous matrix
    */
   template <class T, class Mapper, class Allocator>
   bool hasSclaling4x4( const core::Matrix<T, Mapper, Allocator>& matrix4x4, float eps = 1e-3f  )
   {
      const vector3f spacing = getSpacing4x4( matrix4x4 );
      return !core::equal<float>( spacing[ 0 ], 1, eps ) ||
             !core::equal<float>( spacing[ 1 ], 1, eps ) ||
             !core::equal<float>( spacing[ 2 ], 1, eps );
   }

   /**
    @ingroup core
    @brief returns true if there is a shearing component in a 4x4 homogenous matrix
    */
   template <class T, class Mapper, class Allocator>
   bool hasShearing4x4( const core::Matrix<T, Mapper, Allocator>& matrix4x4, float eps = 1e-2f  )
   {
      StaticVector<T, 3> v1;
      StaticVector<T, 3> v2;
      StaticVector<T, 3> v3;
      for ( size_t n = 0; n < 3; ++n )
      {
         v1[ n ] = matrix4x4( n, 0 );
         v2[ n ] = matrix4x4( n, 1 );
         v3[ n ] = matrix4x4( n, 2 );
      }

      const T d1 = fabs( v1.dot( v2 ) );
      const T d2 = fabs( v1.dot( v3 ) );
      const T d3 = fabs( v3.dot( v2 ) );

      return d1 >= eps ||
             d2 >= eps ||
             d3 >= eps;
   }

   /**
    @ingroup core
    @brief Create a 2D transformation matrix, given a rotation, scaling and translation parameters
    @param roation the rotation angle from the x-axis in radian

    the matrix has this format:
                       | scalingx * rx0 ; scalingy * ry0 ; tx |
                   M = | scalingx * rx1 ; scalingy * ry1 ; ty |
                       |     0          ;     0          ; 1  |
    */
   inline core::Matrix<double>
   createTransformationAffine2D( double rotation,
                                 const core::vector2d& scaling,
                                 const core::vector2d& translation )
   {
      const double co = cos( rotation );
      const double si = sin( rotation );
      core::Matrix<double> tfm( 3, 3, false );
      tfm( 0, 0 ) =  co * scaling[ 0 ];
      tfm( 0, 1 ) = -si * scaling[ 1 ];
      tfm( 0, 2 ) =  translation[ 0 ];
      tfm( 1, 0 ) =  si * scaling[ 0 ];
      tfm( 1, 1 ) =  co * scaling[ 1 ];
      tfm( 1, 2 ) =  translation[ 1 ];
      tfm( 2, 0 ) =  0;
      tfm( 2, 1 ) =  0;
      tfm( 2, 2 ) =  1;

      return tfm;
   }

   /**
    @ingroup core
    @brief Returns the rotation contained in a 4x4 similarity affine transformation
    @note the matrix must not contain shearing components to be valid
    */
   template <class type, class mapper, class allocator>
   Matrix<type, mapper, allocator> getRotation4x4( const Matrix<type, mapper, allocator>& m )
   {
      ensure( m.sizex() == 4 && m.sizey() == 4, "must be a 4x4 matrix" );

      #ifdef NLL_SECURE
      ensure( !hasShearing4x4( m ), "the transformation must not have any shearing!" );
      #endif

      StaticVector<type, 3> spacing;
      for ( size_t x = 0; x < 3; ++x )
      {
         spacing[ x ] = std::sqrt( core::sqr( m( 0, x ) ) +
                                   core::sqr( m( 1, x ) ) +
                                   core::sqr( m( 2, x ) ) );
      }

      Matrix<type, mapper, allocator> rot( 4, 4 );
      for ( size_t y = 0; y < 3; ++y )
      {
         for ( size_t x = 0; x < 3; ++x )
         {
            rot( y, x ) = m( y, x ) / spacing[ x ];
         }
      }
      rot( 3, 3 ) = 1;
      return rot;
   }

   /**
    @ingroup core
    @brief Assuming a 4x4 transformation matrix defined as
                 | Rx Ry Rz Tx |
           Tfm = | Rx Ry Rz Ty |
                 | Rx Ry Rz Tz |
                 | 0  0  0  1  |
           Returns an homogeneous 4x4 matrix with only the R part
    @note the matrix must not contain shearing components to be valid
    */
   template <class type, class mapper, class allocator>
   Matrix<type, mapper, allocator> getRotationAndSpacing( const Matrix<type, mapper, allocator>& m )
   {
      ensure( m.sizex() == 4 && m.sizey() == 4, "must be a 4x4 matrix" );

      Matrix<type, mapper, allocator> rot( 4, 4 );
      for ( size_t y = 0; y < 3; ++y )
      {
         for ( size_t x = 0; x < 3; ++x )
         {
            rot( y, x ) = m( y, x );
         }
      }
      rot( 3, 3 ) = 1;
      return rot;
   }

   /**
    @ingroup core
    @brief Return true if the 3x3 or 4x4 transformation has only a scaling and/or translation component
    */
   template <class type, class mapper, class allocator>
   bool isScalingTranslationMatrixOnly( const Matrix<type, mapper, allocator>& m, type tolerance = (type)1e-5 )
   {
      ensure( ( m.sizex() == 4 && m.sizey() == 4 ) ||
              ( m.sizex() == 3 && m.sizey() == 3 ) , "must be a 4x4 or 3x3 matrix" );

      // check the affine transform
      const size_t sizeAffine = m.sizex() - 1;
      for ( size_t y = 0; y < sizeAffine; ++y )
      {
         for ( size_t x = 0; x < sizeAffine; ++x )
         {
            if ( x != y && fabs( m( y, x ) ) > tolerance )
            {
               return false;
            }
         }
      }

      // check the last row
      for ( size_t n = 0; n < sizeAffine; ++n )
      {
         if ( fabs( m( sizeAffine, n ) ) > tolerance )
         {
            return false;
         }
      }
      
      // check the last element
      return fabs( m( sizeAffine, sizeAffine ) - 1 ) <= tolerance;
   }


   /**
    @ingroup core
    @brief Assuming a 4x4 transformation matrix defined as
                 | 1 0 0 Tx |
           Tfm = | 0 1 0 Ty |
                 | 0 0 1 Tz |
                 | 0 0 0 1  |
           Returns an homogeneous 4x4 matrix with only the T part
    */
   template <class type>
   Matrix<type> createTranslation4x4( const StaticVector<type, 3>& translation )
   {
      Matrix<type> rot = identityMatrix< Matrix<type> >( 4 );
      rot( 0, 3 ) = translation[ 0 ];
      rot( 1, 3 ) = translation[ 1 ];
      rot( 2, 3 ) = translation[ 2 ];
      return rot;
   }

   /**
    @ingroup core
    @brief Assuming a 4x4 transformation matrix defined as
                 | sx   0  0 |
           Tfm = | 0 sy 0  0 |
                 | 0 0  sz 0 |
                 | 0 0  0  1 |
           Returns an homogeneous 4x4 matrix with only the T part
    */
   template <class type>
   Matrix<type> createScaling4x4( const StaticVector<type, 3>& scaling )
   {
      Matrix<type> rot = identityMatrix< Matrix<type> >( 4 );
      rot( 0, 0 ) = scaling[ 0 ];
      rot( 1, 1 ) = scaling[ 1 ];
      rot( 2, 2 ) = scaling[ 2 ];
      return rot;
   }

   /**
    @ingroup core
    @brief Returns the spacing of a 4x4 homogeneous similarity transformation matrix
    */
   template <class type, class mapper, class allocator>
   vector3f getSpacing4x4( const Matrix<type, mapper, allocator>& m )
   {
      ensure( m.sizex() == 4 && m.sizey() == 4, "must be a 4x4 matrix" );
      vector3f spacing;
      for ( size_t x = 0; x < 3; ++x )
      {
         spacing[ x ] = std::sqrt( core::sqr( m( 0, x ) ) +
                                   core::sqr( m( 1, x ) ) +
                                   core::sqr( m( 2, x ) ) );
      }

      #ifdef NLL_SECURE
      ensure( !hasShearing4x4( m ), "must not have shearing" );
      #endif

      return spacing;
   }

   /**
    @ingroup core
    @brief Returns the spacing of a 3x3 homogeneous transformation matrix
    */
   template <class type, class mapper, class allocator>
   vector2f getSpacing3x3( const Matrix<type, mapper, allocator>& m )
   {
      ensure( m.sizex() == 3 && m.sizey() == 3, "must be a 3x3 matrix" );
      vector2f spacing;
      for ( size_t x = 0; x < 2; ++x )
      {
         spacing[ x ] = (f32)std::sqrt( core::sqr( m( 0, x ) ) +
                                        core::sqr( m( 1, x ) ) );
      }
      return spacing;
   }

   /**
    @ingroup core
    @brief Get the spacing/translation/rotation of a similarity matrix
    @note assuming a 2D affine matrix with no shearing
    */
   template <class type, class mapper, class allocator>
   void getMatrixProperties3x3( const Matrix<type, mapper, allocator>& m, vector2f& outSpacing, float& outAngle, vector2f& outTranslation )
   {
      outSpacing = getSpacing3x3( m );
      outTranslation = vector2f( (f32)m( 0, 2 ), (f32)m( 1, 2 ) );
      outAngle = getAngle( (f32)m( 0, 0 ), (f32)m( 1, 0 ) );
   }

   /**
    @ingroup core
    @brief Mapping from three-dimensional Cartesian coordinates to spherical coordinates
           with theta and phi in [-pi, pi]
    @param x the position in x in cartesian coordinate
    @param y the position in x in cartesian coordinate
    @param z the position in x in cartesian coordinate
    @param r_out the radial coordinate in spherical space
    @param theta_out the azimuthal coordinate in spherical space
    @param phi_out the polar coordinate in spherical space
    @see http://mathworld.wolfram.com/SphericalCoordinates.html
         http://en.wikipedia.org/wiki/List_of_common_coordinate_transformations
    */
   template <class TOUT>
   void cartesianToSphericalCoordinate( double x, double y, double z, TOUT& r_out, TOUT& theta_out, TOUT& phi_out )
   {
      if ( fabs( x ) < std::numeric_limits<double>::epsilon() )
      {
         x = std::numeric_limits<double>::epsilon();
      }
      const double sum2 = x * x + y * y;
      const double dist = std::sqrt( sum2 + z * z );
      const double dist2 = std::sqrt( sum2 );
      if ( dist <= 1e-10 )
      {
         r_out     = 0;
         theta_out = 0;
         phi_out   = 0;
      } else {
         r_out     = static_cast<TOUT>( dist );
         theta_out = static_cast<TOUT>( std::atan2( y, x ) );
         phi_out   = static_cast<TOUT>( std::atan2( z, dist2 ) );
      }
   }

   /**
    @ingroup core
    @brief Mapping from three-dimensional Cartesian coordinates to spherical coordinates
           with theta and phi in [-pi, pi]

    @param cartesianCoordinate [x, y, z]
    @return (radial, azimuthal, polar) or (r, phi, theta)
    */
   inline core::vector3d cartesianToSphericalCoordinate( const core::vector3d& cartesianCoordinate )
   {
      core::vector3d result;
      cartesianToSphericalCoordinate( cartesianCoordinate[ 0 ], cartesianCoordinate[ 1 ], cartesianCoordinate[ 2 ],
                                       result[ 0 ], result[ 1 ], result[ 2 ] );
      return result;
   }

   /**
    @ingroup core
    @brief Convert from spherical coordinates to cartesian coordinates
    @see http://www.mathworks.de/help/techdoc/ref/sph2cart.html
    */
   template <class TOUT>
   void sphericalToCartesianCoordinate( double r, double theta, double phi, TOUT& x_out, TOUT& y_out, TOUT& z_out )
   {
      const double cost = cos( theta );
      const double sint = sin( theta );
      const double cosp = cos( phi );
      const double sinp = sin( phi );

      x_out = static_cast<TOUT>( r * cosp * cost );
      y_out = static_cast<TOUT>( r * cosp * sint );
      z_out = static_cast<TOUT>( r * sinp );
   }

   /**
    @ingroup core
    @brief Convert from spherical coordinates to cartesian coordinates
    */
   inline core::vector3d sphericalToCartesianCoordinate( const core::vector3d& polarCoordinate )
   {
      core::vector3d result;
      sphericalToCartesianCoordinate( polarCoordinate[ 0 ], polarCoordinate[ 1 ], polarCoordinate[ 2 ],
                                      result[ 0 ], result[ 1 ], result[ 2 ] );
      return result;
   }

   /**
    @ingroup core
    @brief Computes the 4x4 homogenous rotation matrix from the euler angles

    Computes Rot(z) * Rot(y) * Rot(x)

    @see createRotationMatrix4x4FromEuler for the inverse operation
    */
   template <class T, class Mapper, class Allocator>
   void createRotationMatrix4x4FromEuler( const core::StaticVector<T, 3>& eulerAngle, core::Matrix<T, Mapper, Allocator>& rotationMatrix4x4 )
   {
      typedef core::Matrix<T, Mapper, Allocator> MatrixT;
      
      MatrixT rotx( 4, 4 );
      MatrixT roty( 4, 4 );
      MatrixT rotz( 4, 4 );

      matrix4x4RotationX( rotx, eulerAngle[ 0 ] );
      matrix4x4RotationY( roty, eulerAngle[ 1 ] );
      matrix4x4RotationZ( rotz, eulerAngle[ 2 ] );
      rotationMatrix4x4 = rotz * roty * rotx;
   }

   /**
    @ingroup core
    @brief Compute the euler rotation angles of a 4x4 homogeneous rotation matrix
    @note the matrix must be exactly a rotation matrix (i.e., no shearing, no scaling...)

    Computes (x, y, z) such that rotationMatrix4x4 = Rot(z) * Rot(y) * Rot(x)

    @see https://truesculpt.googlecode.com/hg-history/38000e9dfece971460473d5788c235fbbe82f31b/Doc/rotation_matrix_to_euler.pdf
         Computing Euler angles from a rotation matrix, Gregory G. Slabaugh

    @see createRotationMatrix4x4FromEuler for the inverse operation
    @note when c1 != 0, we always have 2 solutions for the same matrix!
    */
   template <class T, class Mapper, class Allocator>
   core::StaticVector<T, 3> getEulerAngleFromRotationMatrix( const core::Matrix<T, Mapper, Allocator>& rotationMatrix4x4 )
   {
      STATIC_ASSERT( core::IsFloatingType<T>::value );   // we must be on floating point

      #ifdef NLL_SECURE
      ensure( !hasSclaling4x4( rotationMatrix4x4 ), "must not have any scaling" );
      ensure( !hasShearing4x4( rotationMatrix4x4 ), "must not have any shearing" );
      #endif

      const T eps = 1e-6;

      if ( equal<T>( fabs( rotationMatrix4x4( 2, 0 ) ), 1, eps ) ) // R31 == +/- 1
      {
         const T phi = 0;
         if ( equal<T>( rotationMatrix4x4( 2, 0 ), -1, eps ) )
         {
            const T theta = static_cast<T>( core::PI ) / 2;
            const T v = phi + std::atan2( rotationMatrix4x4( 0, 1 ), rotationMatrix4x4( 0, 2 ) );
            return vector3f( v, theta, phi );
         } else {
            const T theta = - static_cast<T>( core::PI ) / 2;
            const T v = - phi + std::atan2( - rotationMatrix4x4( 0, 1 ), - rotationMatrix4x4( 0, 2 ) );
            return vector3f( v, theta, phi );
         }
      } else {
         const T theta1 = - std::asin( rotationMatrix4x4( 2, 0 ) );
         const T theta2 = static_cast<T>( core::PI ) - theta1;

         const T c1 = std::cos( theta1 );
         const T c2 = std::cos( theta2 );


         const T v1 = std::atan2( rotationMatrix4x4( 2, 1 ) / c1,
                                  rotationMatrix4x4( 2, 2 ) / c1 );
         const T v2 = std::atan2( rotationMatrix4x4( 2, 1 ) / c2,
                                  rotationMatrix4x4( 2, 2 ) / c2 );

         const T phi1 = std::atan2( rotationMatrix4x4( 1, 0 ) / c1,
                                    rotationMatrix4x4( 0, 0 ) / c1 );
         const T phi2 = std::atan2( rotationMatrix4x4( 1, 0 ) / c2,
                                    rotationMatrix4x4( 0, 0 ) / c2 );

         if ( !equal<T>( c1, 0, eps ) )
         {
            return vector3f( v1, theta1, phi1 );
         } else {
            return vector3f( v2, theta2, phi2 );
         }
      }
   }
}
}

#endif
