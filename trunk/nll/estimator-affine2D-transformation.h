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

#ifndef NLL_ESTIMATOR_AFFINE2D_TRANSFORMATION_DLT_H_
# define NLL_ESTIMATOR_AFFINE2D_TRANSFORMATION_DLT_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @note for all 2D/3D estimators, they must estimate as best as they can the transformation, especially when they are facing errors
          or transformation given contraints can't be enforced. If this is the case, the <getLastResult> will return ERROR an error fflag, but also
          a matrix letting the user decide if even if the constraint is not enforced, the result may be somehow valid...
    */

   /**
    @ingroup algorithm
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
      EstimatorTransformAffine2dDlt( double minimumScale = 0.7, double maximumScale = 1.6, double maxShearing = 0.1 ) : _result( ERROR ), _minScale( minimumScale ), _maxScale( maximumScale ), _maxShearing( maxShearing )
      {}

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
         const size_t nbPoints = static_cast<size_t>( points1.size() );
         const size_t nbDim = static_cast<size_t>( points1[ 0 ].size() );

         Vector mean = Vector( nbDim );
         Vector std = Vector( nbDim );
         for ( size_t n = 0; n < nbDim; ++n )
         {
            for ( size_t p = 0; p < nbPoints; ++p )
            {
               mean[ n ] += points1[ p ][ n ];
            }
            mean[ n ] /= nbPoints;
         }

         for ( size_t n = 0; n < nbDim; ++n )
         {
            for ( size_t p = 0; p < nbPoints; ++p )
            {
               std[ n ] += core::sqr( points1[ p ][ n ] - mean[ n ] );
            }
            std[ n ] = sqrt( std[ n ] / nbPoints ) + 1e-10;
         }

         points1_out.clear();
         for ( size_t p = 0; p < nbPoints; ++p )
         {
            Point point( nbDim );
            for ( size_t n = 0; n < nbDim; ++n )
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
         const size_t nbPoints = static_cast<size_t>( points1.size() );

         Matrix a( 2 * nbPoints, 6 );
         for ( size_t n = 0; n < nbPoints; ++n )
         {
            a( 2 * n, 3 ) = - points1[ n ][ 0 ];
            a( 2 * n, 4 ) = - points1[ n ][ 1 ];
            a( 2 * n, 5 ) = - 1;

            a( 2 * n + 1, 0 ) = points1[ n ][ 0 ];
            a( 2 * n + 1, 1 ) = points1[ n ][ 1 ];
            a( 2 * n + 1, 2 ) = 1;
         }

         Matrix b( 2 * nbPoints, 1 );
         for ( size_t n = 0; n < nbPoints; ++n )
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

   /**
    @ingroup algorithm
    @brief Find the exact solution for the isometry mapping (i.e., this find the exact solution by using only the minimal number of points to find the transformation)
    
    Uses exactly 2 pairs of points A and B to find the transformation T such that B = T(A), with T = | R(theta) 0 translation |
                                                                                                     | 0        1             |
    @see http://personal.lut.fi/users/joni.kamarainen/downloads/publications/laitosrap111.pdf for implementation reference
         Experimental Study on Fast 2D Homography Estimation from a Few Point Correspondences
         Joni-Kristian Kamarainen
         Pekka Paalanen
    */
   class AffineIsometry2dExact
   {
   public:
      enum Result {OK, ERROR};
      typedef core::Matrix<double>  Matrix;

   public:
      AffineIsometry2dExact() : _result( ERROR )
      {}

      template <class Points1, class Points2>
      Matrix compute( const Points1& points1, const Points2& points2 )
      {
         ensure( points1.size() == 2, "we can only use 2 pairs of points" );
         ensure( points2.size() == 2, "we can only use 2 pairs of points" );

         const double dx = points1[ 0 ][ 0 ] - points1[ 1 ][ 0 ];
         const double dy = points1[ 0 ][ 1 ] - points1[ 1 ][ 1 ];

         const double dxp = points2[ 0 ][ 0 ] - points2[ 1 ][ 0 ];
         const double dyp = points2[ 0 ][ 1 ] - points2[ 1 ][ 1 ];

         const double r   = std::sqrt( core::sqr( dx )  + core::sqr( dy ) );
         const double rp  = std::sqrt( core::sqr( dxp ) + core::sqr( dyp ) );
         const double rrp = r * rp;

         if ( fabs( rrp ) <= 1e-7 )
         {
            _result = ERROR;
            return core::identityMatrix<Matrix>( 3 );
         }

         const double r0 =  ( dx * dxp + dy  * dyp ) / rrp;
         const double r1 = ( -dx * dyp + dxp * dy ) / rrp;
         const double r2 = -r1;
         const double r3 =  r0;

         Matrix tfm( 3, 3 );
         tfm( 2, 2 ) = 1;
         tfm( 0, 0 ) = r0;
         tfm( 0, 1 ) = r1;
         tfm( 0, 2 ) = - r0 * points1[ 0 ][ 0 ] + r2 * points1[ 0 ][ 1 ] + points2[ 0 ][ 0 ];

         tfm( 1, 0 ) = r2;
         tfm( 1, 1 ) = r3;
         tfm( 1, 2 ) = - r0 * points1[ 0 ][ 1 ] - r2 * points1[ 0 ][ 0 ] + points2[ 0 ][ 1 ];
         _result = OK;
         return tfm;
      }

      Result getLastResult() const
      {
         return _result;
      }

   private:
      Result   _result;
   };


   /**
    @ingroup algorithm
    @brief Find the exact solution for the similarity mapping (i.e., this find the exact solution by using only the minimal number of points to find the transformation)
    
    Uses exactly 2 pairs of points A and B to find the transformation T such that B = T(A), with T = | scale * R(theta) 0 translation |
                                                                                                     | 0        1                     |
    @see http://personal.lut.fi/users/joni.kamarainen/downloads/publications/laitosrap111.pdf for implementation reference
         Experimental Study on Fast 2D Homography Estimation from a Few Point Correspondences
         Joni-Kristian Kamarainen
         Pekka Paalanen
    */
   class AffineSimilarity2dExact
   {
   public:
      enum Result {OK, ERROR};
      typedef core::Matrix<double>  Matrix;

   public:
      AffineSimilarity2dExact() : _result( ERROR )
      {}

      template <class Points1, class Points2>
      Matrix compute( const Points1& points1, const Points2& points2 )
      {
         ensure( points1.size() == 2, "we can only use 2 pairs of points" );
         ensure( points2.size() == 2, "we can only use 2 pairs of points" );

         const double dx = points1[ 0 ][ 0 ] - points1[ 1 ][ 0 ];
         const double dy = points1[ 0 ][ 1 ] - points1[ 1 ][ 1 ];

         const double dxp = points2[ 0 ][ 0 ] - points2[ 1 ][ 0 ];
         const double dyp = points2[ 0 ][ 1 ] - points2[ 1 ][ 1 ];

         const double r   = std::sqrt( core::sqr( dx )  + core::sqr( dy ) );
         const double rp  = std::sqrt( core::sqr( dxp ) + core::sqr( dyp ) );
         const double rrp = r * rp;

         if ( fabs( rrp ) <= 1e-7 )
         {
            _result = ERROR;
            return core::identityMatrix<Matrix>( 3 );
         }

         const double scale = rp / r;
         const double r0 = scale * (  dx * dxp + dy  * dyp ) / rrp;
         const double r1 = scale * ( -dx * dyp + dxp * dy )  / rrp;
         const double r2 = -r1;
         const double r3 =  r0;

         Matrix tfm( 3, 3 );
         tfm( 2, 2 ) = 1;
         tfm( 0, 0 ) = r0;
         tfm( 0, 1 ) = r1;
         tfm( 0, 2 ) = - r0 * points1[ 0 ][ 0 ] + r2 * points1[ 0 ][ 1 ] + points2[ 0 ][ 0 ];

         tfm( 1, 0 ) = r2;
         tfm( 1, 1 ) = r3;
         tfm( 1, 2 ) = - r0 * points1[ 0 ][ 1 ] - r2 * points1[ 0 ][ 0 ] + points2[ 0 ][ 1 ];
         _result = OK;
         return tfm;
      }

      Result getLastResult() const
      {
         return _result;
      }

   private:
      Result   _result;
   };

   /**
    @ingroup algorithm
    @brief Compute a non isotropic similarity transformation on a set of points using least square estimation

    @note the algorithm computes the transformation from source to target. Note that the solution is not symetric! (you find another solution if target->source)
          The transformation is defined as T = RS, R = rotation, S = [ s1 0
                                                                       0  s2 ]

    @see Direct implementation of "Least-squares estimation of anisotropic similarity transformations from corresponding 2D point sets"
         Carsten Steger
    */
   class EstimatorAffineSimilarityNonIsotropic2d
   {
   public:
      enum Result {OK, ERROR};
      typedef double Type;
      typedef core::Matrix<Type>  Matrix;
      typedef core::vector2d      Point;

   public:
      EstimatorAffineSimilarityNonIsotropic2d( double minimumScale = 0.7, double maximumScale = 1.6, double maximumAngleRadian = 0.8 ) : 
         _result( ERROR ), _minimumScale( minimumScale ), _maximumScale( maximumScale ), _maximumAngleRadian( maximumAngleRadian )
      {
      }

      template <class Points1, class Points2>
      Matrix compute( const Points1& points1, const Points2& points2 )
      {
         const size_t nbPoints = static_cast<size_t>( points1.size() );
         ensure( points1.size() >= 3, "we need 3 pairs of points" );
         ensure( points2.size() >= 3, "we need 3 pairs of points" );
         ensure( points2.size() == points2.size(), "we need same number of points" );

         // compute the centroid of the two clouds of points
         Point cx( 0, 0 );
         Point cy( 0, 0 );
         for ( size_t n = 0; n < nbPoints; ++n )
         {
            cx[ 0 ] += points1[ n ][ 0 ];
            cx[ 1 ] += points1[ n ][ 1 ];

            cy[ 0 ] += points2[ n ][ 0 ];
            cy[ 1 ] += points2[ n ][ 1 ];
         }
         cx /= static_cast<double>( nbPoints );
         cy /= static_cast<double>( nbPoints );

         // center on (0,0) the points
         std::vector<Point> xp;
         xp.reserve( nbPoints );
         std::vector<Point> yp;
         yp.reserve( nbPoints );

         for ( size_t n = 0; n < nbPoints; ++n )
         {
            xp.push_back( Point( points1[ n ][ 0 ] - cx[ 0 ], points1[ n ][ 1 ] - cx[ 1 ] ) );
            yp.push_back( Point( points2[ n ][ 0 ] - cy[ 0 ], points2[ n ][ 1 ] - cy[ 1 ] ) );
         }

         // compute some constants...
         Type a = 0;
         Type b = 0;
         Type c = 0;
         Type d = 0;
         Type e = 0;
         Type f = 0;
         for ( size_t n = 0; n < nbPoints; ++n )
         {
            a += xp[ n ][ 0 ] * yp[ n ][ 0 ];
            b += xp[ n ][ 0 ] * yp[ n ][ 1 ];
            c += xp[ n ][ 1 ] * yp[ n ][ 0 ];
            d += xp[ n ][ 1 ] * yp[ n ][ 1 ];
            e += core::sqr( xp[ n ][ 0 ] );
            f += core::sqr( xp[ n ][ 1 ] );
         }
         const Type a2 = core::sqr( a );
         const Type b2 = core::sqr( b );
         const Type c2 = core::sqr( c );
         const Type d2 = core::sqr( d );

         if ( fabs( e ) < 1e-7 || fabs( f ) < 1e-7 )
         {
            // generated config
            return core::identityMatrix<Matrix>( 3 );
         }

         const Type g = 0.5 * ( ( a2 - b2 ) / e + ( d2 - c2 ) / f );
         const Type h = c * d / f - a * b / e;

         // computes the transformation parameters
         const Type angle = std::atan2( -h, g ) / 2;
         const Type cosa = std::cos( angle );
         const Type sina = std::sin( angle );
         const Type sc1 = a / e * cosa + b / e * sina;
         const Type sc2 = d / f * cosa - c / f * sina;
         const Type tx = cy[ 0 ] - ( sc1 * cosa * cx[ 0 ] - sc2 * sina * cx[ 1 ] );
         const Type ty = cy[ 1 ] - ( sc1 * sina * cx[ 0 ] + sc2 * cosa * cx[ 1 ] );

         if ( sc1 < _minimumScale || sc2 < _minimumScale ||
              sc1 > _maximumScale || sc2 > _maximumScale ||
              fabs( angle ) > _maximumAngleRadian )
         {
            _result = ERROR;
            return core::identityMatrix<Matrix>( 3 );
         }

         // finally compose the matrix
         _result = OK;
         Matrix tfm(3, 3);
         tfm( 0, 0 ) =  cosa * sc1;
         tfm( 0, 1 ) = -sina * sc2;
         tfm( 1, 0 ) =  sina * sc1;
         tfm( 1, 1 ) =  cosa * sc2;
         tfm( 0, 2 ) =  tx;
         tfm( 1, 2 ) =  ty;
         tfm( 2, 2 ) =  1;
         return tfm;
      }

      Result getLastResult() const
      {
         return _result;
      }

   private:
      Result   _result;
      double   _minimumScale;
      double   _maximumScale;
      double   _maximumAngleRadian;
   };
}
}

#endif