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

#ifndef NLL_ALGORITHM_POTENTIAL_GAUSSIAN_MOMENT_H_
# define NLL_ALGORITHM_POTENTIAL_GAUSSIAN_MOMENT_H_

# pragma warning( push )
# pragma warning( disable:4251 ) // class XXX needs to have dll-interface to be used by clients of class YYY


namespace nll
{
namespace algorithm
{
   // forward declaration
   class PotentialGaussianCanonical;

   /**
    @ingroup algorithm
    @brief Represent a multivariate gaussian parametrized by its moments (mean = m, covariance = cov, alpha)
    @see "A Technique for Painless Derivation of Kalman Filtering Recursions", Ali Taylan Cemgil [1]
         http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.20.3377

    The domain must be ordered so that domain[ 0 ] < domain[ 1 ] < domain[ 2 ] ... meaning that in a graph,
    the parent nodes must have a higher domain than the children.

    p(x) = alpha * exp( -0.5 * (x - m)^t * cov^-1 * (x - m) )

    with p(x) is a PDF, then alpha = 1 / ( (2Pi)^(n/2) * |cov|^(1/2) ) so that it is normalized

    with size(x) = n

    Usually marginalization is done using gaussian moment form and factor multiplication, division, conditioning
    are done in the canonical form.

    @note there is no multiply/divide operations as canonical form is responsible for it
    */
   class NLL_API PotentialGaussianMoment
   {
   public:
      typedef double                      value_type;
      typedef core::Matrix<value_type>    Matrix;
      typedef core::Buffer1D<value_type>  Vector;
      typedef core::Buffer1D<size_t>        VectorI;

   public:
      /**
       @param m mean
       @param c covariance
       @param id naming of the variable
       @note alpha will be set so that the PDF is normaized
       */
      PotentialGaussianMoment( const Vector& m, const Matrix& c, const VectorI id = VectorI() ) : _mean( m ), _id( id )
      {
         _init( m, c, id );
         normalizeGaussian(); // we represent a real PDF if there is no alpha
      }

      /**
       @param m mean
       @param c covariance
       @param id naming of the variable
       @param alpha the nozmalization constante
       */
      PotentialGaussianMoment( const Vector& m, const Matrix& c, const VectorI id, value_type alpha )
      {
         _init( m, c, id );
         _alpha = alpha;   // unormalized potential
      }

      PotentialGaussianMoment()
      {}

      /**
       @brief Create a multivariate gaussian pdf given a set of points using Maximum likelihood estimation
       */
      template <class Points>
      PotentialGaussianMoment( const Points& points, const VectorI id = VectorI() )
      {
         ensure( points.size(), "cannot do an estimation with no points" );
         const size_t nbDim = (size_t)points[ 0 ].size();
         if ( id.size() == 0 )
         {
            // generate the id
            _id = VectorI( nbDim );
            for ( size_t n = 0; n < _id.size(); ++n )
            {
               _id[ n ] = n;
            }
         }

         // now do a Maximum likelihood estimate
         Vector mean;
         Matrix cov = core::covariance( points, &mean );

         // setup the potential
         _init( mean, cov, id );
         normalizeGaussian();
      }

      value_type value( const Vector& x ) const
      {
         ensure( x.size() == _mean.size(), "data size don't match the model parameters" );

         const Matrix& covInv = getCovInv();
         Vector xm( x.size() );
         for ( unsigned n = 0; n < x.size(); ++n )
         {
            xm[ n ] = x[ n ] - _mean[ n ];
         }

         return getAlpha() * std::exp( -0.5 * core::fastDoubleMultiplication( x, covInv ) );
      }

      const Matrix& getCovInv() const
      {
         return _covInv;
      }

      /**
       @brief ensure integral -inf/+inf p(x)dx = 1 by adjusting alpha
       */
      void normalizeGaussian()
      {
         _alpha = normalizeGaussian( getCovDet(), _cov.sizex() );
      }

      /**
       @brief Compute the normalization factor for a gaussian PDF given the determinant of the covariance matrix and its dimension
       */
      static value_type normalizeGaussian( value_type det, size_t size )
      {
         const value_type piCte = std::pow( 2 * core::PI, size * 0.5 );
         const value_type detCte = sqrt( det );
         return 1.0 / ( piCte * detCte );
      }

      const Matrix& getCov() const
      {
         return _cov;
      }

      // note: beware of alpha. It is not updated, the potential will not be normalized
      void setCov( const Matrix& cov )
      {
         _covDet = 0;
         _cov.clone( cov );
         _covInv.clone( cov );
         const bool inverted = core::inverse( _covInv, &_covDet );
         ensure( inverted, "cov is not a PDF!" );
      }

      const Vector& getMean() const
      {
         return _mean;
      }

      void print( std::ostream& o ) const
      {
         o << "alpha=" << _alpha << std::endl
           << "mean=";
         _mean.print( o );
         o << "covariance=";
         _cov.print( o );
      }

      value_type getAlpha() const
      {
         return _alpha;
      }

      // force a specific alpha, it will remove synchronization
      void setAlpha( value_type alpha )
      {
         _alpha = alpha;
      }

      value_type getCovDet() const
      {
         return _covDet;
      }

      /**
       Given a potential on domain X=[U,V], compute the potential p(U)
       @param varIndexToRemove the index of V's
       */
      PotentialGaussianMoment marginalization( const VectorI& varIndexToRemove ) const
      {
         std::vector<size_t> ids, mids;
         computeIndexInstersection( varIndexToRemove, ids, mids );
         ensure( mids.size() == varIndexToRemove.size(), "wrong index: some vars are missing!" );

         Matrix xx, yy, xy, yx;
         core::partitionMatrix( _cov, ids, mids, xx, yy, xy, yx );

         // inn the doc we describe how to get pot(x2), but here we are computing pot(x1) hence the different letters...
         // we have |Sigma| = |Sigma/Sigma_xx|*|Sigma_xx| => |Sigma/Sigma_xx| = |Sigma| / |Sigma_xx|
         const value_type detToRemove = getCovDet() / core::det( xx );            
         const value_type newAlpha = _alpha / normalizeGaussian( detToRemove, yy.sizex() );

         Vector newMean( (size_t)ids.size() );
         VectorI newId( (size_t)ids.size() );
         for ( size_t n = 0; n < ids.size(); ++n )
         {
            newMean[ n ] = _mean[ ids[ n ] ];
            newId[ n ] = _id[ ids[ n ] ];
         }

         return PotentialGaussianMoment( newMean, xx, newId, newAlpha );
      }

      /**
       @brief Given a potential on domain [X, Y] and evidence Y = y, computes p(X, Y=y)
       @param vars the values of Y=y
       @param varsIndex the index of Y's, must be sorted 0->+inf
       @note to be successful, the covariance must be invertible (i.e., represent a PDF)
       */
      PotentialGaussianMoment conditioning( const Vector& vars, const VectorI& varsIndex ) const
      {
         // sort the data
         std::vector<size_t> ids;
         std::vector<size_t> mids;
         computeIndexInstersection( varsIndex, ids, mids );

         Matrix xx, yy, xy, yx;
         core::partitionMatrix( _cov, ids, mids, xx, yy, xy, yx );

         Vector hx( (size_t)ids.size() );
         VectorI indexNew( hx.size() );
         for ( size_t n = 0; n < hx.size(); ++n )
         {
            hx[ n ] = _mean[ ids[ n ] ];
            indexNew[ n ] = _id[ ids[ n ] ];
         }

         Vector hy( (size_t)mids.size() );
         for ( size_t n = 0; n < hy.size(); ++n )
         {
            hy[ n ] = _mean[ mids[ n ] ];
         }

         Matrix yyinv;
         yyinv.clone( yy );
         value_type yydet;
         bool r = core::inverse( yyinv, &yydet );
         ensure( r, "matrix is singular!" );

         Vector t = vars - hy;
         Matrix xyyyinv = xy * yyinv;

         const Matrix newCov = xx - xyyyinv * yx;
         const Vector newMean = Matrix( hx, hx.size(), 1 ) + xyyyinv * Matrix( t, t.size(), 1 );

         const value_type alphayy = std::exp( -0.5 * core::fastDoubleMultiplication( t, yyinv ) );
         const value_type newAlpha = getAlpha() * alphayy;

         return PotentialGaussianMoment( newMean, newCov, indexNew, newAlpha );
      }

      PotentialGaussianCanonical toGaussianCanonical() const;

   private:
      void _init( const Vector& m, const Matrix& c, const VectorI id )
      {
         _mean = m;
         _id = id;

         ensure( m.size() == c.sizex(), "mean size and cov size don't match" );
         ensure( c.sizex() == c.sizey(), "covariance matrix must be square" );
         ensure( _id.size() == 0 || _id.size() == m.size(), "id has wrong size" );

         ensure( isOrdered( id ), "the id list must be ordered first" );

         if ( id.size() == 0 )
         {
            // generate the id
            _id = VectorI( m.size() );
            for ( size_t n = 0; n < _id.size(); ++n )
            {
               _id[ n ] = n;
            }
         }

         setCov( c );
      }

      static bool isOrdered( const VectorI& v )
      {
         if ( !v.size() )
            return true;
         const size_t s = v.size() - 1;
         for ( size_t n = 0; n < s; ++n )
         {
            if ( v[ n ] >= v[ n + 1 ] )
               return false;
         }
         return true;
      }

      void computeIndexInstersection( const VectorI& varsIndex, std::vector<size_t>& ids, std::vector<size_t>& mids ) const
      {
         // first check the index is in correct order
         if ( varsIndex.size() > 1 )
         {
            for ( size_t n = 0; n < varsIndex.size() - 1; ++n )
            {
               ensure( varsIndex[ n ] < varsIndex[ n + 1 ], "the list must be sorted" );
            }
         }

         ids.clear();
         mids.clear();

         ids.reserve( varsIndex.size() );
         mids.reserve( varsIndex.size() );

         size_t indexToCheck = 0;
         const size_t MAX = std::numeric_limits<size_t>::max();
         for ( size_t n = 0; n < _id.size(); ++n )
         {
            const size_t mid = ( indexToCheck < varsIndex.size() ) ? varsIndex[ indexToCheck ] : MAX;
            const size_t id = _id[ n ];
            if ( mid > id )
            {
               ids.push_back( n );
            } else {
               if ( mid == id )
               {
                  mids.push_back( n );
               }
               // if == or < we need to increase the index to check
               ++indexToCheck;
            }
         }
      }

   private:
      Vector       _mean;
      Matrix       _cov;      // it cannot be manually updated! use "setCov"
      Matrix       _covInv;
      value_type   _covDet;
      value_type   _alpha; 
      VectorI     _id;
   };
}
}

# pragma warning( pop )

#endif
