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

#ifndef NLL_ESTIMATOR_AFFINE2D_TRANSFORMATION_H_
# define NLL_ESTIMATOR_AFFINE2D_TRANSFORMATION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @note for all 2D/3D estimators, they must estimate as best as they can the transformation, especially when they are facing errors
          or transformation given contraints can't be enforced. If this is the case, the <getLastResult> will ERROR an error, but also
          a matrix letting the user decide if even if the constraint is not enforced, the result may be somehow valid...
    */

   /**
    @ingroup algorithm
    @brief Computes the similarity transform [Rotation, translation scale] from a matching set of points
    @see Shinji Umeyama. Least-Squares Estimation of Transformation Parameters
         Between Two Point Patterns. PAMI, vol. 13, no. 4, April 1991.
         http://www.stanford.edu/class/cs273/refs/umeyama.pdf
    */
   class EstimatorTransformSimilarityIsometric
   {
   public:
      typedef core::Matrix<double>     Matrix;
      typedef core::Buffer1D<double>   Vector;
      enum Result {OK, ERROR};

      EstimatorTransformSimilarityIsometric( double scale = 0, double minimumScale = 0.7, double maximumScale = 1.6 ) : 
         _scale( scale ), _minimumScale( minimumScale ), _maximumScale( maximumScale )
      {
      }

      /**
       @param scale if set to 0, the algorithm will determine the proper scaling, else it will use it
              as an additional constraint
       */
      template <class Points1, class Points2>
      Matrix compute( const Points1& points1, const Points2& points2 )
      {
         ensure( points1.size() == points2.size() && points2.size() > 0, "must be pair of points, not empty" );
         
         const size_t nbPoints = static_cast<size_t>( points1.size() );
         const size_t nbDim = static_cast<size_t>( points1[ 0 ].size() );

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
         double scale = _scale;
         if ( _scale <= 1e-20 )
         {
            double accum = 0;
            for ( size_t n = 0; n < nbPoints; ++n )
               for ( size_t k = 0; k < nbDim; ++k )
               {
                  const double val = points1[ n ][ k ] - mean1[ k ];
                  accum += val * val;
               }

            double trace = 0;
            for ( size_t k = 0; k < nbDim - 1; ++k )
            {
               trace += eiv[ k ];
            }
            trace += eiv[ nbDim - 1 ] * core::sign( det );
            scale = nbPoints / accum * trace;
         }

         if ( scale < _minimumScale )
         {
            _result = ERROR;
            scale = _minimumScale;
         }

         if ( scale > _maximumScale )
         {
            _result = ERROR;
            scale = _maximumScale;
         }

         core::transpose( v );
         Matrix rot = cov * s * v;
         Matrix t = Matrix( mean2, nbDim, 1 ) - scale * rot * Matrix( mean1, nbDim, 1 );

         Matrix result( nbDim + 1, nbDim + 1 );
         for ( size_t y = 0; y < nbDim; ++y )
         {
            for ( size_t x = 0; x < nbDim; ++x )
            {
               result( y, x ) = rot( y, x ) * scale;
            }
            result( y, nbDim ) = t[ y ];
         }
         _result = OK;
         result( nbDim, nbDim ) = 1;
         return result;
      }

      Result getLastResult() const
      {
         return _result;
      }

   private:
      Result   _result;
      double   _scale;
      double   _minimumScale;
      double   _maximumScale;
   };
}
}

#endif