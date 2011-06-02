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

#ifndef NLL_ESTIMATOR_AFFINE2D_TRANSFORMATION_H_
# define NLL_ESTIMATOR_AFFINE2D_TRANSFORMATION_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Computes the similarity transform [Rotation, translation scale] from a matching set of points
    @see Shinji Umeyama. Least-Squares Estimation of Transformation Parameters
         Between Two Point Patterns. PAMI, vol. 13, no. 4, April 1991.
    */
   class EstimatorTransformAffine2D
   {
   public:
      typedef core::Matrix<double>     Matrix;
      typedef core::Buffer1D<double>   Vector;

      /**
       @param scale if set to 0, the algorithm will determine the proper scaling, else it will use it
              as an additional constraint
       */
      template <class Points1, class Points2>
      Matrix compute( const Points1& points1, const Points2& points2, double scale = 0 )
      {
         ensure( points1.size() == points2.size() && points2.size() > 0, "must be pair of points, not empty" );
         
         const ui32 nbPoints = static_cast<ui32>( points1.size() );
         const ui32 nbDim = static_cast<ui32>( points1[ 0 ].size() );
         ensure( nbDim == 2, "only for 2D cases - check directly applicable to more dim" );

         Vector mean1;
         Vector mean2;
         Matrix cov = core::covariance( points1, points2, mean1, mean2 );

         Vector eiv;
         Matrix v;
         core::svdcmp( cov, eiv, v );
         const double det = core::det( cov ) * core::det( v );

         Matrix s = core::identityMatrix<Matrix>( nbDim );
         if ( det < 0 )
         {
            s( nbDim - 1, nbDim - 1 ) = -1;
         }

         // compute the scale
         if ( scale <= 1e-20 )
         {
            double accum = 0;
            for ( ui32 n = 0; n < nbPoints; ++n )
               for ( ui32 k = 0; k < nbDim; ++k )
               {
                  const double val = points1[ n ][ k ] - mean1[ k ];
                  accum += val * val;
               }

            double trace = 0;
            for ( ui32 k = 0; k < nbDim - 1; ++k )
            {
               trace += eiv[ k ];
            }
            trace += eiv[ nbDim - 1 ] * core::sign( det );
            scale = nbPoints / accum * trace;
         }

         core::transpose( v );
         Matrix rot = cov * s * v;
         Matrix t = Matrix( mean2, nbDim, 1 ) - scale * rot * Matrix( mean1, nbDim, 1 );

         Matrix result( nbDim + 1, nbDim + 1 );
         for ( ui32 y = 0; y < nbDim; ++y )
         {
            for ( ui32 x = 0; x < nbDim; ++x )
            {
               result( y, x ) = rot( y, x ) * scale;
            }
            result( y, nbDim ) = t[ y ];
         }
         result( nbDim, nbDim ) = 1;
         return result;
      }
   };
}
}

#endif