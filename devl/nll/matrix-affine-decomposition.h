/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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

#ifndef NLL_TRANSFORMATION_AFFINE_DECOMPOSITION_H_
# define NLL_TRANSFORMATION_AFFINE_DECOMPOSITION_H_

namespace nll
{
namespace core
{
   /*
    * compute a rotation matrix given a rotation vector using Rodrigues'
    * formula
    *
    *    R = I + sin( theta ) * Sn + ( 1 - cos( theta ) ) * Sn^2                  (1)
    * 
    * for small theta, this can be approximated by
    *
    *    R = I + ( 1 - theta^2 / 6 ) * Sn + ( 1/2 - theta^2 / 24 ) * Sn^2         (2)
    *
    * given
    *
    *            0 -nz  ny
    *    Sn =   nz   0 -nx
    *          -ny  nx   0
    *
    * and
    *
    *             -ny^2-nz^2   nxny         nxnz
    *    Sn^2 =    nxny       -nx^2-nz^2    nynz
    *              nxnz        nynz        -nx^2-n^y^2
    *
    * In (1), Sn is computed using a normalised n, in (2), Sn is computed using a non-normalised n
    * So we have Sn(2) = theta * Sn (1)

    @note this is to be used with rotation and shear vectors created for example by <decomposeAffine4x4>
    */
   template <class T>
   void computeRotationMatrix( const core::StaticVector<T, 3>& theVector, core::Matrix<T>& theTransform )
   {
      // temporary variables used to speed computation time
      T tmp;
      T term2, term3;
      T nxSquared, nySquared, nzSquared;
      core::StaticVector<T, 3> n( theVector );

      // compute the rotation angle
      const T thetaSquared = theVector[ 0 ] * theVector[ 0 ] + 
                             theVector[ 1 ] * theVector[ 1 ] +
                             theVector[ 2 ] * theVector[ 2 ];
      const T theta = static_cast<T>(std::sqrt( thetaSquared ));

      theTransform = core::identityMatrix< core::Matrix<T> >( 3 );

      // special case for small theta
      if ( theta > 1e-5 )
      {
         n /= theta;

         // compute the term involving sin( theta )
         term2 = static_cast<T>(std::sin( theta ));
      
         // compute the term involving ( 1 - cos( theta ) )
         term3 = static_cast<T>(1 - std::cos( theta ));
      } else {
         // for a theta of zero the rotation matrix should be I, so just return
         if ( 0 == theta ) return;

         // compute the term involving 1 - ( theta^2 / 6 )
         term2 = 1 - thetaSquared / 6;

         // compute the term involving ( 1/2 - theta^2 / 24 )
         term3 = (T)(0.5 - thetaSquared / 24);
      }

      //          0 -nz  ny
      //  Sn =   nz   0 -nx
      //        -ny  nx   0
      //
      tmp = term2 * n[ 2 ];
      theTransform( 0, 1 ) = -tmp;
      theTransform( 1, 0 ) =  tmp;
      //
      tmp = term2 * n[ 1 ];
      theTransform( 0, 2 ) =  tmp;
      theTransform( 2, 0 ) = -tmp;
      //
      tmp = term2 * n[ 0 ];
      theTransform( 1, 2 ) = -tmp;
      theTransform( 2, 1 ) =  tmp;

      //           -ny^2-nz^2   nxny         nxnz
      //  Sn^2 =    nxny       -nx^2-nz^2    nynz
      //            nxnz        nynz        -nx^2-n^y^2
      //
      nxSquared = n[ 0 ] * n[ 0 ];
      nySquared = n[ 1 ] * n[ 1 ];
      nzSquared = n[ 2 ] * n[ 2 ];
      //
      theTransform( 0, 0 ) += -term3 * ( nySquared + nzSquared );
      theTransform( 1, 1 ) += -term3 * ( nxSquared + nzSquared );
      theTransform( 2, 2 ) += -term3 * ( nxSquared + nySquared );
      //
      tmp = term3 * n[ 0 ] * n [ 1 ];
      theTransform( 0, 1 ) += tmp;
      theTransform( 1, 0 ) += tmp;
      //
      tmp = term3 * n[ 0 ] * n [ 2 ];
      theTransform( 0, 2 ) += tmp;
      theTransform( 2, 0 ) += tmp;
      //
      tmp = term3 * n[ 1 ] * n [ 2 ];
      theTransform( 2, 1 ) += tmp;
      theTransform( 1, 2 ) += tmp;
   }

   /**
    @brief Returns an unormalized vector wich combines the vector and the angle v = rotVector * theta
    */
   inline core::vector3d getRotationVector( const core::Quaternion& q )
   {
      const double theta = 2 * std::acos( q[ 0 ] );
      const double sin_th_div_2 = std::sin( theta / 2 );
      if ( sin_th_div_2 < 1e-5 )
      {
         return core::vector3d( 1, 0, 0 );   // any vector will do...
      } else {
         return core::vector3d( q[ 1 ] * theta / sin_th_div_2,
                                q[ 2 ] * theta / sin_th_div_2,
                                q[ 3 ] * theta / sin_th_div_2 );
      }
   }

   /**
    @brief This create an affine matrix from its elementary decomposition
    @param rotationVector is an unmormalized rotation vector where Sn(2) = theta * Sn (1) (see http://mathworld.wolfram.com/RodriguesRotationFormula.html)
    @param shearingVector is an unmormalized vector where Sn(2) = theta * Sn (1) (see http://mathworld.wolfram.com/RodriguesRotationFormula.html)
    */
   template <class T>
   core::Matrix<T> createTransformationAffine3D( StaticVector<T, 3>& translationVector,
                                                 StaticVector<T, 3>& rotationVector,
                                                 StaticVector<T, 3>& shearingVector,
                                                 StaticVector<T, 3>& scalingVector )
   {
      core::Matrix<T> scale( 3, 3 );
      scale( 0, 0 ) = scalingVector[ 0 ];
      scale( 1, 1 ) = scalingVector[ 1 ];
      scale( 2, 2 ) = scalingVector[ 2 ];

      core::Matrix<T> rotation;
      core::Matrix<T> shear;
      computeRotationMatrix( rotationVector, rotation );
      computeRotationMatrix( shearingVector, shear );

      const core::Matrix<T> mul = rotation * scale * shear;
      core::Matrix<T> result = core::identityMatrix< core::Matrix<T> >( 4 );
      for ( ui32 i = 0; i < 3; ++i )
      {
         for ( ui32 j = 0; j < 3; ++j )
         {
            result( i, j ) = mul( i, j );
         }
         result( i, 3 ) = translationVector[ i ];
      }
      return result;
   }

   /**
    @brief Decompose a 4x4 affine matrix into a translation, rotation, shearing and scaling parameters

    Note that this decomposition is not unique!
    */
   template <class T, class Mapper, class Allocator>
   void decomposeAffine4x4( const Matrix<T, Mapper, Allocator>& m,
                            StaticVector<T, 3>& translation,
                            StaticVector<T, 3>& rotation,
                            StaticVector<T, 3>& shearing,
                            StaticVector<T, 3>& scaling )
   {
      typedef Matrix<T, Mapper, Allocator>   MatrixT;
      ensure( m.sizex() == 4 && m.sizey() == 4, "must be a 4x4 matrix" );

      // get the rotational part and use SVD on it
      MatrixT rot3( 3, 3 );
      for ( ui32 y = 0; y < 3; ++y )
      {
         for ( ui32 x = 0; x < 3; ++x )
         {
            rot3( y, x ) = m( y, x );
         }
      }

      core::Buffer1D<T> w;
      MatrixT v;
      MatrixT u;
      bool r = core::svd(rot3, u, w, v );

      ensure( r, "matrix badly conditioned" );

      T avg_sc = ( w[ 0 ] + w[ 1 ] + w[ 2 ] ) / 3;
      const T u_det = core::det( u );
      const T v_det = core::det( v );
      const T det_prod_sgn = u_det * v_det;
      u = u * u_det;
      v = v * v_det;
      StaticVector<T, 3> sc( 3 );
      sc[ 0 ] = w[ 0 ] * det_prod_sgn;
      sc[ 1 ] = w[ 1 ] * det_prod_sgn;
      sc[ 2 ] = w[ 2 ] * det_prod_sgn;
      avg_sc = avg_sc  * det_prod_sgn;

      StaticVector<T, 3> sc_diffs( 3 );
      sc_diffs[ 0 ] = fabs( sc[ 0 ] - avg_sc );
      sc_diffs[ 1 ] = fabs( sc[ 1 ] - avg_sc );
      sc_diffs[ 2 ] = fabs( sc[ 2 ] - avg_sc );
      const T max_diff = *std::max_element(&sc_diffs[0], ( &sc_diffs[2] ) + 1 );
      if ( max_diff < 1e-6 * avg_sc )
      {
         for ( ui32 y = 0; y < 3; ++y )
         {
            for ( ui32 x = 0; x < 3; ++x )
            {
               u( y, x ) = m( y, x );
            }
         }
         v = core::identityMatrix<MatrixT>( 3 );
      }

      // export the parameters
      scaling = sc;
      translation[ 0 ] = m( 0, 3 );
      translation[ 1 ] = m( 1, 3 );
      translation[ 2 ] = m( 2, 3 );

      Quaternion quaternion_u( u );
      quaternion_u /= quaternion_u.norm2();
      core::vector3d rotd = getRotationVector( quaternion_u );
      rotation[ 0 ] = (T)rotd[ 0 ];
      rotation[ 1 ] = (T)rotd[ 1 ];
      rotation[ 2 ] = (T)rotd[ 2 ];

      core::transpose(v);
      Quaternion quaternion_v( v );
      quaternion_v /= quaternion_v.norm2();
      core::vector3d shd = getRotationVector( quaternion_v );
      shearing[ 0 ] = (T)shd[ 0 ];
      shearing[ 1 ] = (T)shd[ 1 ];
      shearing[ 2 ] = (T)shd[ 2 ];
   }
}
}

#endif