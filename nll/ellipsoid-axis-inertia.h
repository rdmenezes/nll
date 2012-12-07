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

#ifndef NLL_ALGORITHM_ELLIPSOID_AXIS_INERTIA_H_
# define NLL_ALGORITHM_ELLIPSOID_AXIS_INERTIA_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Given a weighted set of points, compute the ellipsoid fitting the best
           the data and compute the main axis if necessary
    */
   class EllipsoidAxesOfInertia
   {
   public:
      typedef core::Matrix<double>  Matrix;

      /**
       @brief Compute the second moments of a volume transformed by a LUT

       Returns the matrix
          | m_200 m_110 m_101 |
          | m_110 m_020 m_011 |
          | m_101 m_011 m_002 |

          m_pqr = integral( f(x,y,z) (x - xc)^p * (y - yc)^q * (z - zc)^r dxdydz )
          with (xc, yc, zc) the center of mass, (p,q,r) having the values in the above matrix

       @return the moment matrix
       */
      template <class T, class Buffer, class Lut>
      Matrix processSecondMoments( const imaging::Volume<T, Buffer>& volume, const Lut& lut, core::vector3d* outCenterOfMassVoxel = 0 ) const
      {
         if ( lut.getMin() < volume.getBackgroundValue() )
         {
            nllWarning( "processSecondMoments will be biased by the shape of the volume if the lut has a min below background value" );
         }

         const core::vector3f centerOfMassf = imaging::computeBarycentreVoxel( volume, lut );
         const core::vector3d centerOfMass( centerOfMassf[ 0 ], centerOfMassf[ 1 ], centerOfMassf[ 2 ] );
         const core::vector3d size( (double)volume.size()[ 0 ],
                                    (double)volume.size()[ 1 ],
                                    (double)volume.size()[ 2 ] );

         if ( outCenterOfMassVoxel )
         {
            *outCenterOfMassVoxel = centerOfMass;
         }

         double xx = 0;
         double yy = 0;
         double zz = 0;
         double xy = 0;
         double xz = 0;
         double yz = 0;

         // compute the second moments
         double accumRatio = 0;
         const double normCte = lut.getSize() * size[ 0 ] / 2; // to avoid overflow in accum
         for ( double z = 0; z < size[ 2 ]; ++z )
         {
            for ( double y = 0; y < size[ 1 ]; ++y )
            {
               for ( double x = 0; x < size[ 0 ]; ++x )
               {
                  const T value = volume( (size_t)x, (size_t)y, (size_t)z );
                  const double valueTfm = static_cast<double>( lut.transform( value )[ 0 ] );
                  const double ratio = valueTfm / normCte;

                  xx += ratio * core::sqr( x - centerOfMass[ 0 ] );
                  yy += ratio * core::sqr( y - centerOfMass[ 1 ] );
                  zz += ratio * core::sqr( z - centerOfMass[ 2 ] );
                  xy += ratio * ( x - centerOfMass[ 0 ] ) * ( y - centerOfMass[ 1 ] );
                  xz += ratio * ( x - centerOfMass[ 0 ] ) * ( z - centerOfMass[ 2 ] );
                  yz += ratio * ( y - centerOfMass[ 1 ] ) * ( z - centerOfMass[ 2 ] );
                  accumRatio += ratio;
               }
            }
         }

         Matrix moments( 3, 3 );
         moments( 0, 0 ) = xx / accumRatio;
         moments( 1, 1 ) = yy / accumRatio;
         moments( 2, 2 ) = zz / accumRatio;

         moments( 0, 1 ) = xy / accumRatio;
         moments( 1, 0 ) = xy / accumRatio;

         moments( 0, 2 ) = xz / accumRatio;
         moments( 2, 0 ) = xz / accumRatio;

         moments( 1, 2 ) = yz / accumRatio;
         moments( 2, 1 ) = yz / accumRatio;

         return moments;
      }

      /**
       @brief Extract the ellipsoid basis using a SVD ordered by the highest eigen values
       @return the ellipsoid basis, 1 basis vector = 1 column
       @param outEigenValues if not null, the eigenvalues will be returned
       */
      Matrix extractOrderedAxis( const Matrix& moments, core::vector3d* outEigenValues = 0 ) const
      {
         core::Buffer1D<double> ev;
         Matrix v;
         Matrix u;
         const bool success = core::svd( moments, u, ev, v );
         ensure( success, "problem in the moment matrix" );
         ensure( v.sizex() == 3 && v.sizey() == 3, "problem with moment matrix: some eigen values are null or too small" );

         if ( outEigenValues )
         {
            (*outEigenValues)[ 0 ] = ev[ 0 ];
            (*outEigenValues)[ 1 ] = ev[ 1 ];
            (*outEigenValues)[ 2 ] = ev[ 2 ];
         }

         return v;
      }

      /**
       @brief Extract the ellipsoid basis
       @return the ellipsoid basis, 1 basis vector = 1 column
       @param outEigenValues if not null, the eigenvalues will be returned
       */
      template <class T, class Buffer, class Lut>
      Matrix extractOrderedAxis( const imaging::Volume<T, Buffer>& volume, const Lut& lut, core::vector3d* outEigenValues = 0, core::vector3d* outCenterOfMassVoxel = 0 ) const
      {
         Matrix moments = processSecondMoments( volume, lut, outCenterOfMassVoxel );
         return extractOrderedAxis( moments, outEigenValues );
      }
   };
}
}

#endif
