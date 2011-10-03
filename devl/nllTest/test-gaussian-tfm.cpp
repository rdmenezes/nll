#include <nll/nll.h>
#include <tester/register.h>


/**
 @note This module is specialized
 */
namespace nll
{
namespace core
{

}
namespace algorithm
{
   class GaussianMultivariateCanonical;

   /**
    @brief Represent a multivariate gaussian parametrized by its moments (mean = m, covariance = cov)

    p(x) = alpha * exp( -0.5 * (x - m)^t * cov^-1 * (x - m) )

    with alpha = 1 / ( (2Pi)^(n/2) * |cov|^(1/2) ) in case of normalized gaussian

    with size(x) = n
    */
   class GaussianMultivariateMoment
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
               _alpha = 1.0 / ( std::pow( (value_type)core::PI, (value_type)_cov.sizex() / 2 ) * sqrt( _covDet ) );
            }
         }

         return _covInv;
      }

      const Matrix& getCov() const
      {
         return _cov;
      }

      const Vector& getMean() const
      {
         return _mean;
      }

      value_type getAlpha() const
      {
         if ( _isAlphaNormalized && !_isCovSync )
         {
            getCovInv();
         }
         return _alpha;
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

         return GaussianMultivariateMoment( newMean, xx, newId );
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
   class GaussianMultivariateCanonical
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
                  h[ n ] += _h[ id1 ];

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
                  h[ n ] += g2._h[ id2 ];

                  for ( ui32 nn = 0; nn < newSize; ++nn )
                  {
                     const ui32 id2b = index2[ nn ];
                     if ( id2b != MAX )
                     {
                        k( n, nn ) += g2._k( id2, id2b );
                     }
                  }
               }
            }

            return GaussianMultivariateCanonical( h, k, _g + g2._g, mergedId );
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

         // new params:
         Matrix      knew = xx - xyyyinv * yx;
         Matrix      hnew = hx - xyyyinv * hy;
         value_type  gnew = _g - 0.5 * log( fabs( detyyinv / ( 2 * core::PI ) ) ) + 0.5 * core::fastDoubleMultiplication( hy, yyinv ); // TODO: check this value as "Probabilistic Graphical Models" D. Koller don't agree on this and was wrong at the previous edition... using as defined in [1]
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

         Vector hy( (ui32)ids.size() );
         for ( ui32 n = 0; n < hy.size(); ++n )
         {
            hy[ n ] = _h[ mids[ n ] ];
         }

         // compute the new parameters
         Vector hnew = Matrix( hx, hx.size(), 1 ) - xy * Matrix( vars, vars.size(), 1 );
         Matrix knew = xx;
         value_type gnew = _g + core::dot( hy, vars ) -0.5 * core::fastDoubleMultiplication( vars, yy );
         return GaussianMultivariateCanonical( hnew, knew, gnew, indexNew );
      }

      GaussianMultivariateMoment toGaussianMoment() const
      {
         Matrix cov;
         cov.clone( _k );

         value_type detcov;
         const bool r = core::inverse( cov, &detcov );
         ensure( r, "can't inverse precision matrix" );
         
         Vector mean = cov * Matrix( _h,  _h.size(), 1 );

         const value_type cte1 = std::log( fabs( detcov ) / ( 2.0 * core::PI ) );
         const value_type cte2 = 0.5 * cte1 - 0.5 * core::fastDoubleMultiplication( _h, cov );
         const value_type alpha = std::exp( _g  - cte2 );

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

   GaussianMultivariateCanonical GaussianMultivariateMoment::toGaussianCanonical() const
   {
      const Matrix& covInv = getCovInv();

      Matrix k;
      k.clone( covInv );

      Vector h = k * Matrix( _mean, _mean.size(), 1 );

      const value_type cte1 = log( 1.0 / ( 2.0 * core::PI * fabs( getCovDet() ) ) );
      const value_type cte2 = 0.5 * cte1 - 0.5 * core::fastDoubleMultiplication( h, _cov );
      const value_type g = std::log( getAlpha() ) + cte2;

      VectorI id;
      id.clone( _id );

      return GaussianMultivariateCanonical( h, k, g, id );
   }
}
}

using namespace nll;
using namespace nll::algorithm;

class TestGaussianTransformation
{
   typedef std::vector<double>   Point;
   typedef std::vector<Point>    Points;

public:
   void testMul1()
   {
      GaussianMultivariateCanonical::VectorI i1( 2 );
      i1[ 0 ] = 0;
      i1[ 1 ] = 2;

      GaussianMultivariateCanonical::Vector m1( 2 );
      m1[ 0 ] = 4;
      m1[ 1 ] = 5;

      GaussianMultivariateCanonical::Matrix k1( 2, 2 );
      k1( 0, 0 ) = 1;
      k1( 0, 1 ) = 2;
      k1( 1, 0 ) = 3;
      k1( 1, 1 ) = 4;

      GaussianMultivariateCanonical::VectorI i2( 2 );
      i2[ 0 ] = 0;
      i2[ 1 ] = 1;

      GaussianMultivariateCanonical::Vector m2( 2 );
      m2[ 0 ] = 10;
      m2[ 1 ] = 11;

      GaussianMultivariateCanonical::Matrix k2( 2, 2 );
      k2( 0, 0 ) = 10;
      k2( 0, 1 ) = 11;
      k2( 1, 0 ) = 12;
      k2( 1, 1 ) = 13;

      GaussianMultivariateCanonical g1( m1, k1, 2, i1 );
      GaussianMultivariateCanonical g2( m2, k2, 3, i2 );
      GaussianMultivariateCanonical r = g1.mul( g2 );

      g1.print( std::cout );
      g2.print( std::cout );
      r.print( std::cout );

      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 0 ], 14 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 1 ], 11 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 2 ], 5 ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 0 ), 11 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 1 ), 11 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 2 ), 2 ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 0 ), 12 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 1 ), 13 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 2 ), 0 ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 2, 0 ), 3 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 2, 1 ), 0 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 2, 2 ), 4 ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getG(), 5 ) );
   }

   void testMarginalization1()
   {
      GaussianMultivariateCanonical::VectorI i1( 4 );
      i1[ 0 ] = 0;
      i1[ 1 ] = 1;
      i1[ 2 ] = 2;
      i1[ 3 ] = 3;

      GaussianMultivariateCanonical::Vector m1( 4 );
      m1[ 0 ] = 4;
      m1[ 1 ] = 5;
      m1[ 2 ] = 6;
      m1[ 3 ] = 7;

      GaussianMultivariateCanonical::Matrix k1( 4, 4 );
      unsigned accum = 0;
      for ( unsigned y = 0; y < 4; ++y )
      {
         for ( unsigned x = 0; x < 4; ++x )
         {
            ++accum;
            k1( y, x ) = accum * accum;
         }
      }
      GaussianMultivariateCanonical g1( m1, k1, 2, i1 );
      GaussianMultivariateCanonical::VectorI mids( 2 );
      mids[ 0 ] = 1;
      mids[ 1 ] = 2;
      GaussianMultivariateCanonical r = g1.marginalization( mids );

      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 0 ], 1.5882352941176499, 1e-8 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 1 ], 1.5882352941176485, 1e-8  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 0 ), 0.94117647058823906, 1e-8  ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 1 ), 0.94117647058823906, 1e-8  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 0 ), 0.94117647058823906, 1e-8  ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 1 ), 0.94117647058823906, 1e-8  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getG(), -0.093587560811534409, 1e-8  ) );
   }

   Points generateGaussianData()
   {
      ui32 nbDim = core::generateUniformDistributioni( 1, 15 );

      // generate distribution parameters
      std::vector< std::pair< double, double > > params;
      for ( ui32 nn = 0; nn < nbDim; ++nn )
      {
         const double mean = core::generateUniformDistribution( -30, 30 );
         const double var = core::generateUniformDistribution( 1, 6 );
         params.push_back( std::make_pair( mean, var ) );
      }

      // generate the data
      std::vector<Point> points;
      for ( ui32 nn = 0; nn < 5 * nbDim; ++nn )
      {
         Point p( nbDim );
         for ( ui32 i = 0; i < nbDim; ++i )
         {
            const double val = core::generateGaussianDistribution( params[ i ].first, params[ i ].second );
            p[ i ] = val;
         }
         points.push_back( p );
      }

      return points;
   }

   void testConversion()
   {
      const ui32 nbTests = 200;
      for ( ui32 n = 0; n < nbTests; ++n )
      {
         Points points = generateGaussianData();

         // generate points
         core::Buffer1D<double> mean;
         core::Matrix<double> cov = nll::core::covariance( points, &mean );

         // check the conversion between the gaussian representations
         GaussianMultivariateMoment gm( mean, cov );
         GaussianMultivariateCanonical gc = gm.toGaussianCanonical();
         GaussianMultivariateMoment gm2 = gc.toGaussianMoment();

         const GaussianMultivariateMoment::Matrix& cov1 = gm.getCov();
         const GaussianMultivariateMoment::Matrix& cov2 = gm2.getCov();

         const GaussianMultivariateMoment::Vector& mean1 = gm.getMean();
         const GaussianMultivariateMoment::Vector& mean2 = gm2.getMean();

         TESTER_ASSERT( mean1.equal( mean2, 1e-8 ) );
         TESTER_ASSERT( cov1.equal( cov2, 1e-8 ) );
         TESTER_ASSERT( core::equal( gm.getAlpha(), gm2.getAlpha(), 1e-4 ) );
      }
   }

   void testMarginalization()
   {
      const ui32 nbTests = 200;
      for ( ui32 n = 0; n < nbTests; ++n )
      {
         Points points = generateGaussianData();

         // generate points
         core::Buffer1D<double> mean;
         core::Matrix<double> cov = nll::core::covariance( points, &mean );

         // check the conversion between the gaussian representations
         GaussianMultivariateMoment gm( mean, cov );
         GaussianMultivariateCanonical gc = gm.toGaussianCanonical();

         if ( cov.sizex() <= 2 )
         {
            continue;
         }

         // check we find the same results
         std::vector<ui32> list = core::generateUniqueList( 0, cov.sizex() - 1 );
         ui32 nbVar = core::generateUniformDistributioni( 1, cov.sizex() - 1 );
         
         GaussianMultivariateMoment::VectorI varToMarginalize( nbVar );
         for ( ui32 nn = 0; nn < nbVar; ++nn )
         {
            varToMarginalize[ nn ] = list[ nn ];
         }

         std::sort( varToMarginalize.begin(), varToMarginalize.end() );

         GaussianMultivariateMoment gmm = gm.marginalization( varToMarginalize );
         GaussianMultivariateCanonical gcm = gc.marginalization( varToMarginalize );
         GaussianMultivariateMoment gmm2 = gcm.toGaussianMoment();

         const GaussianMultivariateMoment::Matrix& cov1 = gmm.getCov();
         const GaussianMultivariateMoment::Matrix& cov2 = gmm2.getCov();

         const GaussianMultivariateMoment::Vector& mean1 = gmm.getMean();
         const GaussianMultivariateMoment::Vector& mean2 = gmm2.getMean();

         TESTER_ASSERT( mean1.equal( mean2, 1e-8 ) );
         TESTER_ASSERT( cov1.equal( cov2, 1e-8 ) );
      }
   }

   void testConditioning()
   {
      const double m1 = 2;
      const double m2 = -3;
      const double c1 = 0.5;
      const double c2 = 2.5;

      Points points1;
      Points points2;
      for ( ui32 n = 0; n < 5000; ++n )
      {
         Point p1( 1 );
         p1[ 0 ] = core::generateGaussianDistribution( m1, c1 );

         Point p2( 1 );
         p2[ 0 ] = core::generateGaussianDistribution( m2, c2 );

         points1.push_back( p1 );
         points2.push_back( p2 );
      }

      GaussianMultivariateMoment::VectorI i1( 1 );
      i1[ 0 ] = 0;
      GaussianMultivariateMoment::VectorI i2( 1 );
      i2[ 0 ] = 1;

      GaussianMultivariateMoment g1( points1, i1 );
      GaussianMultivariateMoment g2( points2, i2 );

      GaussianMultivariateCanonical g1c = g1.toGaussianCanonical();
      GaussianMultivariateCanonical g2c = g2.toGaussianCanonical();

      GaussianMultivariateMoment::Vector v( 1 );
      v[ 0 ] = -3;
      GaussianMultivariateCanonical gc = g1c.mul( g2c );
      GaussianMultivariateCanonical gcc = gc.conditioning( v, i2 );
      GaussianMultivariateMoment g = gcc.toGaussianMoment();

      std::cout << std::endl;

   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianTransformation);
/*
TESTER_TEST(testMarginalization1);
TESTER_TEST(testMarginalization);
TESTER_TEST(testMul1);
*/
TESTER_TEST(testConversion);
//TESTER_TEST(testConditioning);
TESTER_TEST_SUITE_END();
#endif