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

#ifndef NLL_ALGORITHM_GAUSSIAN_REPRESENTATION_H_
# define NLL_ALGORITHM_GAUSSIAN_REPRESENTATION_H_

# pragma warning( push )
# pragma warning( disable:4251 ) // class XXX needs to have dll-interface to be used by clients of class YYY


namespace nll
{
namespace algorithm
{
   // forward declaration
   class GaussianMultivariateCanonical;

   /**
    @brief Represent a multivariate gaussian parametrized by its moments (mean = m, covariance = cov)
    @see "A Technique for Painless Derivation of Kalman Filtering Recursions", Ali Taylan Cemgil [1]
         http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.20.3377

    p(x) = alpha * exp( -0.5 * (x - m)^t * cov^-1 * (x - m) )

    with alpha = 1 / ( (2Pi)^(n/2) * |cov|^(1/2) ) in case of normalized gaussian

    with size(x) = n

    Usually marginalization is done using gaussian moment form and factor multiplication, division, conditioning
    are done in the canonical form.
    */
   class NLL_API GaussianMultivariateMoment
   {
   public:
      typedef double                      value_type;
      typedef core::Matrix<value_type>    Matrix;
      typedef core::Buffer1D<value_type>  Vector;
      typedef core::Buffer1D<ui32>        VectorI;

   public:
      /**
       @param m mean
       @param c covariance
       @param id naming of the variable
       @param alpha if alpha not set, ensure that integral -inf/+inf p(x)dx = 1 (it represents a PDF)
       */
      GaussianMultivariateMoment( const Vector& m, const Matrix& c, const VectorI id = VectorI(), const value_type alpha = std::numeric_limits<value_type>::max() ) : _mean( m ),
         _cov( c ), _isCovSync( false ), _id( id ), _alpha( alpha )
      {
         ensure( m.size() == c.sizex(), "mean size and cov size don't match" );
         ensure( c.sizex() == c.sizey(), "covariance matrix must be square" );
         ensure( _id.size() == 0 || _id.size() == m.size(), "id has wrong size" );

         ensure( isOrdered( id ), "the id list must be ordered first" );

         if ( alpha >= std::numeric_limits<value_type>::max() )
         {
            _isAlphaNormalized = true;
         } else {
            _isAlphaNormalized = false;
         }

         if ( id.size() == 0 )
         {
            // generate the id
            _id = VectorI( m.size() );
            for ( ui32 n = 0; n < _id.size(); ++n )
            {
               _id[ n ] = n;
            }
         }
      }

      GaussianMultivariateMoment() : _isCovSync( false ), _isAlphaNormalized( true )
      {
      }

      /**
       @brief Create a multivariate gaussian pdf given a set of points using Maximum likelihood estimation
       */
      template <class Points>
      GaussianMultivariateMoment( const Points& points, const VectorI id = VectorI() ) : _isCovSync( false ), _id( id ), _isAlphaNormalized( true )
      {
         ensure( points.size(), "cannot do an estimation with no points" );
         const ui32 nbDim = (ui32)points[ 0 ].size();
         if ( id.size() == 0 )
         {
            // generate the id
            _id = VectorI( nbDim );
            for ( ui32 n = 0; n < _id.size(); ++n )
            {
               _id[ n ] = n;
            }
         }

         // now do a Maximum likelihood estimate
         Vector mean;
         Matrix cov = core::covariance( points, &mean );

         _mean = mean;
         _cov = cov;
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

      // this will sync alpha & cov if necessary
      const Matrix& getCovInv() const
      {
         if ( !_isCovSync )
         {
            // if cov and covInv are not sunchronized, then do it...
            _covInv.clone( _cov );
            bool r = core::inverse( _covInv, &_covDet );
            ensure( r, "covariance is singular!" );
            _isCovSync = true;
            if ( _isAlphaNormalized )
            {
               normalizeGaussian();
            }
         }

         return _covInv;
      }

      /**
       @brief ensure integral -inf/+inf p(x)dx = 1 by adjusting alpha
       */
      void normalizeGaussian() const
      {
         value_type det = getCovDet();
         _alpha = 1.0 / ( std::pow( (value_type)core::PI, (value_type)_cov.sizex() / 2 ) * sqrt( det ) );
         _isAlphaNormalized = true;
      }

      const Matrix& getCov() const
      {
         return _cov;
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
         if ( _isAlphaNormalized && !_isCovSync )
         {
            getCovInv();
         }
         return _alpha;
      }

      // force a specific alpha, it will remove synchronization
      void setAlpha( value_type alpha )
      {
         _isAlphaNormalized = false;
         _alpha = alpha;
      }

      value_type getCovDet() const
      {
         if ( !_isCovSync )
         {
            getCovInv();
         }
         return _covDet;
      }

      /**
       @brief computes p(X | Y=y)
       @param vars the values of Y=y
       @param varsIndex the index of Y's, must be sorted 0->+inf
       */
      GaussianMultivariateMoment marginalization( const VectorI& varIndexToRemove ) const
      {
         std::vector<ui32> ids, mids;
         computeIndexInstersection( varIndexToRemove, ids, mids );
         ensure( mids.size() == varIndexToRemove.size(), "wrong index: some vars are missing!" );
         ensure( ids.size() > 0, "marginalization of a gaussian on all its variables is 1!" );

         Matrix xx;
         partitionMatrix( _cov, ids, xx );

         Vector newMean( (ui32)ids.size() );
         VectorI newId( (ui32)ids.size() );
         for ( ui32 n = 0; n < ids.size(); ++n )
         {
            newMean[ n ] = _mean[ ids[ n ] ];
            newId[ n ] = _id[ ids[ n ] ];
         }

         // _alpha stays the same! TODO: check as the marginalization in canonical form doesn't give the same resutls
         return GaussianMultivariateMoment( newMean, xx, newId, _alpha );
      }

      /**
       @brief computes p(X | Y=y)
       @param vars the values of Y=y
       @param varsIndex the index of Y's, must be sorted 0->+inf
       */
      GaussianMultivariateMoment conditioning( const Vector& vars, const VectorI& varsIndex ) const
      {
         // sort the data
         std::vector<ui32> ids;
         std::vector<ui32> mids;
         computeIndexInstersection( varsIndex, ids, mids );

         Matrix xx, yy, xy, yx;
         core::partitionMatrix( _cov, ids, mids, xx, yy, xy, yx );

         Vector hx( (ui32)ids.size() );
         VectorI indexNew( hx.size() );
         for ( ui32 n = 0; n < hx.size(); ++n )
         {
            hx[ n ] = _mean[ ids[ n ] ];
            indexNew[ n ] = _id[ ids[ n ] ];
         }

         Vector hy( (ui32)mids.size() );
         for ( ui32 n = 0; n < hy.size(); ++n )
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


         Matrix newCov = xx - xyyyinv * yx;
         Vector newMean = Matrix( hx, hx.size(), 1 ) + xyyyinv * Matrix( t, t.size(), 1 );

         const value_type newAlpha = getAlpha() / std::sqrt( 2 * core::PI * yydet ) * 
            std::exp( -0.5 * core::fastDoubleMultiplication( t, yyinv ) );

         return GaussianMultivariateMoment( newMean, newCov, indexNew, newAlpha );
      }

      GaussianMultivariateCanonical toGaussianCanonical() const;

   private:
      static bool isOrdered( const VectorI& v )
      {
         if ( !v.size() )
            return true;
         const ui32 s = v.size() - 1;
         for ( ui32 n = 0; n < s; ++n )
         {
            if ( v[ n ] >= v[ n + 1 ] )
               return false;
         }
         return true;
      }

      void computeIndexInstersection( const VectorI& varsIndex, std::vector<ui32>& ids, std::vector<ui32>& mids ) const
      {
         // first check the index is in correct order
         if ( varsIndex.size() > 1 )
         {
            for ( ui32 n = 0; n < varsIndex.size() - 1; ++n )
            {
               ensure( varsIndex[ n ] < varsIndex[ n + 1 ], "the list must be sorted" );
            }
         }

         ids.clear();
         mids.clear();

         ids.reserve( varsIndex.size() );
         mids.reserve( varsIndex.size() );

         ui32 indexToCheck = 0;
         const ui32 MAX = std::numeric_limits<ui32>::max();
         for ( ui32 n = 0; n < _id.size(); ++n )
         {
            const ui32 mid = ( indexToCheck < varsIndex.size() ) ? varsIndex[ indexToCheck ] : MAX;
            const ui32 id = _id[ n ];
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

      /**
        Recompose a matrix given 2 index vector X and Y so that the matrix has this format:
                src = | XX XY |
                      | YX XX |
              => export XX
       */
      void partitionMatrix( const Matrix& src,
                            const std::vector<ui32>& x,
                            Matrix& xx ) const
      {
         xx = Matrix( (ui32)x.size(), (ui32)x.size() );
         for ( ui32 ny = 0; ny < xx.sizey(); ++ny )
         {
            const ui32 idy = x[ ny ];
            for ( ui32 nx = 0; nx < xx.sizex(); ++nx )
            {
               const ui32 idx = x[ nx ];
               xx( ny, nx ) = src( idy, idx );
            }
         }
      }

   private:
      Vector   _mean;
      Matrix   _cov;
      mutable Matrix       _covInv;       // NOTE: this must NEVER be used alone, but use the getter getCovInv() to ensure correct update
      mutable value_type   _covDet;
      mutable value_type   _alpha;        // the normalization constante
      mutable bool         _isCovSync;    // if true, it means <_cov> and <_covInv> are synchronized, else <_covInv> will need to be recomputed
      mutable bool         _isAlphaNormalized;    // if true, the alpha will make sure integral -inf/+inf p(x)dx = 1
      VectorI  _id;
   };

   /**
    @brief Represent a multivariate gaussian in its canonical form
    @see "A Technique for Painless Derivation of Kalman Filtering Recursions", Ali Taylan Cemgil [1]
         http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.20.3377

    p(x) = exp( g + h^t * x - 0.5 x^t * K * x )

    K = cov^-1
    h = cov^-1 * mean
    g = -0.5 * m^t * cov^-1 * m - log( (2pi)^(n/2)|cov|^0.5)
    */
   class NLL_API GaussianMultivariateCanonical
   {
   public:
      typedef double                      value_type;
      typedef core::Matrix<value_type>    Matrix;
      typedef core::Buffer1D<value_type>  Vector;
      typedef core::Buffer1D<ui32>        VectorI;

   public:
      GaussianMultivariateCanonical( const Vector& h, const Matrix& k, value_type g, const VectorI id = VectorI() )
      {
         ensure( isOrdered( id ), "the id list must be ordered first" );
         _h = h;
         _k = k;
         _g = g;

         if ( id.size() == 0 )
         {
            // generate the id
            _id = VectorI( h.size() );
            for ( ui32 n = 0; n < _id.size(); ++n )
            {
               _id[ n ] = n;
            }
         } else {
            _id = id;
         }
      }

      /**
       @brief computes p(x)
       */
      value_type value( const Vector& x ) const
      {
         ensure( x.size() == _h.size(), "size doesn't match" );
         return std::exp( _g + core::generic_dot<Vector, value_type>( x, _h, x.size() ) + core::fastDoubleMultiplication( x, _k ) );
      }

      GaussianMultivariateCanonical operator*( const GaussianMultivariateCanonical& g2 ) const
      {
         GaussianMultivariateCanonical g = *this;
         return g.mul( g2 );
      }

      GaussianMultivariateCanonical operator/( const GaussianMultivariateCanonical& g2 ) const
      {
         GaussianMultivariateCanonical g = *this;
         return g.div( g2 );
      }

      /**
       @brief computes g(k1, h1, g1) * g(k2, h2, g2) = g( k1 + k2, h1 + h2, g1 + g2) if the 2 gaussians
       have the same scope, else, we need to padd with 0s on the missing variables
       */
      GaussianMultivariateCanonical mul( const GaussianMultivariateCanonical& g2 ) const
      {
         if ( deepCheck( _id, g2._id ) )
         {
            // simple case
            return GaussianMultivariateCanonical( _h + g2._h, _k + g2._k, _g + g2._g, _id );
         } else {
            return opGeneric( g2, std::plus<value_type>() );
         }
      }

      /**
       @brief computes g(k1, h1, g1) / g(k2, h2, g2) = g( k1 - k2, h1 - h2, g1 - g2) if the 2 gaussians
       have the same scope, else, we need to padd with 0s on the missing variables
       */
      GaussianMultivariateCanonical div( const GaussianMultivariateCanonical& g2 ) const
      {
         if ( deepCheck( _id, g2._id ) )
         {
            // simple case
            return GaussianMultivariateCanonical( _h + g2._h, _k + g2._k, _g + g2._g, _id );
         } else {
            return opGeneric( g2, std::minus<value_type>() );
         }
      }



      /**
       @brief Given a gaussian representing P( X, Y ), compute P( X ) by integrating over Y
       @param varIndexToRemove the variable to marginalize, assuming to be sorted 0->+inf


       Internally, we group K = | XX XY |
                                | YX YY |
       */
      GaussianMultivariateCanonical marginalization( const VectorI& varIndexToRemove ) const
      {
         // compute the remaining set
         std::vector<ui32> ids;
         std::vector<ui32> mids;
         computeIndexInstersection( varIndexToRemove, ids, mids );
         ensure( mids.size() == varIndexToRemove.size(), "wrong index: some vars are missing!" );
         ensure( ids.size() > 0, "marginalization of a gaussian on all its variables is 1!" );

         // create the hx and hy subvectors
         Matrix hx( (ui32)ids.size(), 1 );
         VectorI indexNew( hx.size() );
         for ( ui32 n = 0; n < hx.size(); ++n )
         {
            hx[ n ] = _h[ ids[ n ] ];
            indexNew[ n ] = _id[ ids[ n ] ];
         }

         Matrix hy( (ui32)mids.size(), 1 );
         for ( ui32 n = 0; n < hy.size(); ++n )
         {
            hy[ n ] = _h[ mids[ n ] ];
         }

         // now create the submatrix XX YY XY YX
         Matrix xx, yy, xy, yx;
         partitionMatrix( _k, ids, mids, xx, yy, xy, yx );

         Matrix yyinv;
         yyinv.clone( yy );
         value_type detyyinv = 0;
         bool success = core::inverse( yyinv, &detyyinv );
         ensure( success, "could not inverse matrix yy" );

         Matrix xyyyinv = xy * yyinv;
         ensure( detyyinv > 0, "determinant YY must be > 0" );
         const value_type detyy = 1.0 / detyyinv;

         // new params:
         Matrix      knew = xx - xyyyinv * yx;
         Matrix      hnew = hx - xyyyinv * hy;
         value_type  gnew = _g + 0.5 * ( yyinv.sizex() * std::log( 2 * core::PI ) - std::log( detyy )
                                       + core::fastDoubleMultiplication( hy, yyinv ) );
         return GaussianMultivariateCanonical( hnew, knew, gnew, indexNew );
      }

      /**
       @brief computes p(X | Y=y)
       @param vars the values of Y=y
       @param varsIndex the index of Y's, must be sorted 0->+inf
       */
      GaussianMultivariateCanonical conditioning( const Vector& vars, const VectorI& varsIndex ) const
      {
         // sort the data
         std::vector<ui32> ids;
         std::vector<ui32> mids;
         computeIndexInstersection( varsIndex, ids, mids );

         Matrix xx, yy, xy, yx;
         partitionMatrix( _k, ids, mids, xx, yy, xy, yx );

         Vector hx( (ui32)ids.size() );
         VectorI indexNew( hx.size() );
         for ( ui32 n = 0; n < hx.size(); ++n )
         {
            hx[ n ] = _h[ ids[ n ] ];
            indexNew[ n ] = _id[ ids[ n ] ];
         }

         Vector hy( (ui32)mids.size() );
         for ( ui32 n = 0; n < hy.size(); ++n )
         {
            hy[ n ] = _h[ mids[ n ] ];
         }

         // compute the new parameters
         Vector hnew = Matrix( hx, hx.size(), 1 ) - xy * Matrix( vars, vars.size(), 1 );
         Matrix knew = xx;
         value_type gnew = _g + core::dot( hy, vars ) - 0.5 * core::fastDoubleMultiplication( vars, yy );
         return GaussianMultivariateCanonical( hnew, knew, gnew, indexNew );
      }

      GaussianMultivariateMoment toGaussianMoment() const
      {
         Matrix cov;
         cov.clone( _k );

         value_type detk;
         const bool r = core::inverse( cov, &detk );
         ensure( r, "can't inverse precision matrix" );
         
         Vector mean = cov * Matrix( _h,  _h.size(), 1 );

         ensure( detk > 0, "determinant K must be > 0" );
         const value_type cte1 = 0.5 * ( log( detk )
                                       - _h.size() * std::log( 2.0 * core::PI )
                                       - core::fastDoubleMultiplication( mean, _k ) );
         const value_type alpha = std::exp( _g - cte1 );

         return GaussianMultivariateMoment( mean, cov, _id, alpha );
      }

      void print( std::ostream& o )
      {
         o << "GaussianMultivariateCanonical:" << std::endl;
         o << "h=";
         _h.print( o );
         o << "k=";
         _k.print( o );
         o << "g=" << _g << std::endl;
         o << "index=";
         _id.print( o );
      }

      const Vector& getH() const
      {
         return _h;
      }

      const Matrix& getK() const
      {
         return _k;
      }

      value_type getG() const
      {
         return _g;
      }

      const VectorI& getId() const
      {
         return _id;
      }

   private:
      template <class Functor>
      GaussianMultivariateCanonical opGeneric( const GaussianMultivariateCanonical& g2, const Functor& op ) const
      {
         // compute the id intersection and new order
         const ui32 s1 = _id.size();
         const ui32 s2 = g2._id.size();
         const VectorI& v1 = _id;
         const VectorI& v2 = g2._id;
         const ui32 MAX = std::numeric_limits<ui32>::max();
         
         // we know the ids are ordered, which will make the comparison very simple
         ui32 id1 = 0;
         ui32 id2 = 0;

         std::vector<ui32> index1;
         std::vector<ui32> index2;
         std::vector<ui32> ids;
         index1.reserve( std::max( s1, s2 ) );
         index2.reserve( std::max( s1, s2 ) );
         ids.reserve( std::max( s1, s2 ) );
         while ( true )
         {
            if ( id1 == s1 && id2 == s2 )
               break;
            const ui32 val1 = ( id1 == s1 ) ? ( MAX - 2 ) : v1[ id1 ];
            const ui32 val2 = ( id2 == s2 ) ? ( MAX - 1 ) : v2[ id2 ];

            if ( val1 == val2 )  // same, so just get the ID
            {
               index1.push_back( id1 );
               index2.push_back( id2 );
               ids.push_back( val1 );
               ++id1;
               ++id2;
            } else if ( val1 < val2 ) // v1 has one value more than v2, so padd v2 with MAX
            {
               index1.push_back( id1 );
               index2.push_back( MAX );
               ids.push_back( val1 );
               ++id1;
            } else if ( val2 < val1 )
            {
               index2.push_back( id2 );
               index1.push_back( MAX );
               ids.push_back( val2 );
               ++id2;
            }
         }

         // finally extract the data
         const ui32 newSize = static_cast<ui32>( index1.size() );
         Vector h( newSize );
         Matrix k( newSize, newSize );
         VectorI mergedId( newSize );
         for ( ui32 n = 0; n < newSize; ++n )
         {
            mergedId[ n ] = ids[ n ];
            const ui32 id1 = index1[ n ];
            const ui32 id2 = index2[ n ];
            if ( id1 != MAX )
            {
               h[ n ] = op( h[ n ], _h[ id1 ] );

               for ( ui32 nn = 0; nn < newSize; ++nn )
               {
                  const ui32 id1b = index1[ nn ];
                  if ( id1b != MAX )
                  {
                     k( n, nn ) = _k( id1, id1b );
                  }
               }
            }
            if ( id2 != MAX )
            {
               h[ n ] = op( h[ n ], g2._h[ id2 ] );

               for ( ui32 nn = 0; nn < newSize; ++nn )
               {
                  const ui32 id2b = index2[ nn ];
                  if ( id2b != MAX )
                  {
                     k( n, nn ) = op( k( n, nn ), g2._k( id2, id2b ) );
                  }
               }
            }
         }

         return GaussianMultivariateCanonical( h, k, op( _g, g2._g ), mergedId );
      }

      void computeIndexInstersection( const VectorI& varsIndex, std::vector<ui32>& ids, std::vector<ui32>& mids ) const
      {
         // first check the index is in correct order
         if ( varsIndex.size() > 1 )
         {
            for ( ui32 n = 0; n < varsIndex.size() - 1; ++n )
            {
               ensure( varsIndex[ n ] < varsIndex[ n + 1 ], "the list must be sorted" );
            }
         }

         ids.clear();
         mids.clear();

         ids.reserve( varsIndex.size() );
         mids.reserve( varsIndex.size() );

         ui32 indexToCheck = 0;
         const ui32 MAX = std::numeric_limits<ui32>::max();
         for ( ui32 n = 0; n < _id.size(); ++n )
         {
            const ui32 mid = ( indexToCheck < varsIndex.size() ) ? varsIndex[ indexToCheck ] : MAX;
            const ui32 id = _id[ n ];
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

      static bool isOrdered( const VectorI& v )
      {
         if ( !v.size() )
            return true;
         const ui32 s = v.size() - 1;
         for ( ui32 n = 0; n < s; ++n )
         {
            if ( v[ n ] >= v[ n + 1 ] )
               return false;
         }
         return true;
      }

      static bool deepCheck( const VectorI& v1, const VectorI& v2 )
      {
         if ( v1.size() != v2.size() )
            return false;
         const ui32 s = v1.size();

         for ( ui32 n = 0; n < s; ++n )
         {
            if ( v1[ n ] != v2[ n ] )
               return false;
         }
         return true;
      }

   private:
      Vector         _h;
      Matrix         _k;
      value_type     _g;
      VectorI        _id;
   };
}
}

# pragma warning( pop )

#endif
