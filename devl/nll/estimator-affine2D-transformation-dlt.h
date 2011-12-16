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

#ifndef NLL_ESTIMATOR_AFFINE2D_TRANSFORMATION_DLT_H_
# define NLL_ESTIMATOR_AFFINE2D_TRANSFORMATION_DLT_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Affine 2D transformation estimation using DLT

      Estimates non isotropic affine transformations

      See for a very good introduction of the different kind of methods for 2D:
      EXPERIMENTAL STUDY ON FAST 2D HOMOGRAPHY ESTIMATION FROM A FEW POINT CORRESPONDENCES
      Joni-Kristian Kämäräinen and Pekka Paalanen
      www.it.lut.fi/publications/files/publications/603/report01_laitosraportti.pdf

      First the points are normalized so that no attribut will have a higher weight, compute
      and solve the DLT problem, unnormalize the resulting matrix.
    */
   class EstimatorTransformAffine2dDlt
   {
   public:
      enum Result {OK, ERROR};

      typedef double value_type;
      typedef core::Matrix<value_type>     Matrix;
      typedef core::Buffer1D<value_type>   Vector;
      typedef Vector                       Point;
      typedef std::vector<Point>           Points;

      /**
       @brief Affine estimation using DLT method
       @param maxShearing the maximum shearing factor allowed
       @param minimumScale the minimal scaling for a transformation to have for not begin rejected
       @param maximumScale the maximal scaling for a transformation to have for not begin rejected
       */
      EstimatorTransformAffine2dDlt( double minimumScale = 0.7, double maximumScale = 1.6, double maxShearing = 0.1 ) : _minScale( minimumScale ), _maxScale( maximumScale ), _maxShearing( maxShearing )
      {}

      /**
       @param scale if set to 0, the algorithm will determine the proper scaling, else it will use it
              as an additional constraint
       */
      template <class Points1, class Points2>
      Matrix compute( const Points1& points1, const Points2& points2 )
      {
         ensure( points1.size() == points2.size() && points2.size() > 0, "must be pair of points, not empty" );
         
         const ui32 nbPoints = static_cast<ui32>( points1.size() );
         const ui32 nbDim = static_cast<ui32>( points1[ 0 ].size() );
         ensure( nbDim == 2, "only for 2D cases - check directly applicable to more dim" );
         ensure( nbPoints >= 3, "Affine 2D DLT needs at least 3 points to estimate the transformation" );

         // normalize the points
         Points p1Norm;
         Points p2Norm;
         Matrix normalize1;
         Matrix normalize2;
         normalize( points1, p1Norm, normalize1 );
         normalize( points2, p2Norm, normalize2 );

         Matrix a, b;
         createMatrices( p1Norm, p2Norm, a, b );

         try
         {
            Vector h = core::leastSquareSvd( a, b );

            Matrix tfm( 3, 3 );
            tfm( 0, 0 ) = h[ 0 ];
            tfm( 0, 1 ) = h[ 1 ];
            tfm( 0, 2 ) = h[ 2 ];

            tfm( 1, 0 ) = h[ 3 ];
            tfm( 1, 1 ) = h[ 4 ];
            tfm( 1, 2 ) = h[ 5 ];

            tfm( 2, 0 ) = 0;
            tfm( 2, 1 ) = 0;
            tfm( 2, 2 ) = 1;

            core::inverse( normalize2 );

            _result = OK;

            // check the constraints
            Matrix matrix = normalize2 * tfm * normalize1;
            core::vector2f sp = getSpacing3x3( matrix );
            const double shearingFactor = fabs( matrix( 0, 0 ) / sp[ 0 ] * matrix( 0, 1 ) / sp[ 1 ] +
                                                matrix( 1, 0 ) / sp[ 0 ] * matrix( 1, 1 ) / sp[ 1 ] );
            if ( sp[ 0 ] < _minScale || sp[ 0 ] > _maxScale ||
                 sp[ 1 ] < _minScale || sp[ 1 ] > _maxScale || 
                 shearingFactor > _maxShearing )
            {
               matrix = core::identityMatrix<Matrix>( 3 );
            }
            return matrix;
         } catch (...)
         {
            _result = ERROR;
            return core::identityMatrix<Matrix>( 3 );
         }
      }

      Result getLastResult() const
      {
         return _result;
      }

   private:
      template <class Points1>
      void normalize( const Points1& points1, Points& points1_out, Matrix& normalization )
      {
         const ui32 nbPoints = static_cast<ui32>( points1.size() );
         const ui32 nbDim = static_cast<ui32>( points1[ 0 ].size() );

         Vector mean = Vector( nbDim );
         Vector std = Vector( nbDim );
         for ( ui32 n = 0; n < nbDim; ++n )
         {
            for ( ui32 p = 0; p < nbPoints; ++p )
            {
               mean[ n ] += points1[ p ][ n ];
            }
            mean[ n ] /= nbPoints;
         }

         for ( ui32 n = 0; n < nbDim; ++n )
         {
            for ( ui32 p = 0; p < nbPoints; ++p )
            {
               std[ n ] += core::sqr( points1[ p ][ n ] - mean[ n ] );
            }
            std[ n ] = sqrt( std[ n ] / nbPoints ) + 1e-10;
         }

         points1_out.clear();
         for ( ui32 p = 0; p < nbPoints; ++p )
         {
            Point point( nbDim );
            for ( ui32 n = 0; n < nbDim; ++n )
            {
               point[ n ] = ( points1[ p ][ n ] - mean[ n ] ) / std[ n ];
            }
            points1_out.push_back( point );
         }

         // compose the normalization matrix M: normalizedPoints = M * x
         normalization = Matrix( 3, 3 );
         normalization( 0, 0 ) = 1 / std[ 0 ];
         normalization( 1, 1 ) = 1 / std[ 1 ];
         normalization( 2, 2 ) = 1;
         normalization( 0, 2 ) = - mean[ 0 ] / std[ 0 ];
         normalization( 1, 2 ) = - mean[ 1 ] / std[ 1 ];
      }

      template <class Points1, class Points2>
      static void createMatrices( const Points1& points1, const Points2& points2, Matrix& a_out, Matrix& b_out )
      {
         const ui32 nbPoints = static_cast<ui32>( points1.size() );

         Matrix a( 2 * nbPoints, 6 );
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            a( 2 * n, 3 ) = - points1[ n ][ 0 ];
            a( 2 * n, 4 ) = - points1[ n ][ 1 ];
            a( 2 * n, 5 ) = - 1;

            a( 2 * n + 1, 0 ) = points1[ n ][ 0 ];
            a( 2 * n + 1, 1 ) = points1[ n ][ 1 ];
            a( 2 * n + 1, 2 ) = 1;
         }

         Matrix b( 2 * nbPoints, 1 );
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            b( 2 * n, 0 ) = - points2[ n ][ 1 ];
            b( 2 * n + 1, 0 ) = points2[ n ][ 0 ];
         }

         a_out = a;
         b_out = b;
      }

   private:
      Result    _result;
      double    _minScale;
      double    _maxScale;
      double    _maxShearing;
   };
}
}

#endif