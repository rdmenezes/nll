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
   /**
    @brief Represent a multivariate gaussian parametrized by its moments (mean = m, covariance = cov)

    p(x) = 1 / ( (2Pi)^(n/2) * |cov|^(1/2) ) * exp( -0.5 * (x - m)^t * cov^-1 * (x - m) )

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
       */
      GaussianMultivariateMoment( const Vector& m, const Matrix& c, const VectorI id = VectorI() )
      {
      }
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
         _id = id;
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

         // create the hx and hy subvectors
         Matrix hx( (ui32)ids.size(), 1 );
         VectorI indexNew( hx.size() );
         for ( ui32 n = 0; n < hx.size(); ++n )
         {
            hx[ n ] = ids[ n ];
            indexNew[ n ] = ids[ n ];
         }

         Matrix hy( (ui32)mids.size(), 1 );
         for ( ui32 n = 0; n < hy.size(); ++n )
         {
            hy[ n ] = mids[ n ];
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
         Matrix      hnew = hx - knew * hy;
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
            hx[ n ] = ids[ n ];
            indexNew[ n ] = ids[ n ];
         }

         Vector hy( (ui32)ids.size() );
         for ( ui32 n = 0; n < hy.size(); ++n )
         {
            hy[ n ] = mids[ n ];
         }

         // compute the new parameters
         Vector hnew = Matrix( hx, hx.size(), 1 ) - xy * Matrix( vars, vars.size(), 1 );
         Matrix knew = xx;
         value_type gnew = _g + core::dot( hy, vars ) -0.5 * core::fastDoubleMultiplication( vars, yy );
         return GaussianMultivariateCanonical( hnew, knew, gnew, indexNew );
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
}
}

using namespace nll::algorithm;

class TestGaussianTransformation
{
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

      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 0 ], -2.823, 1e-3 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 1 ], 0.176, 1e-3  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 0 ), 0.9411, 1e-3  ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 1 ), 0.9411, 1e-3  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 0 ), 0.9411, 1e-3  ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 1 ), 0.9411, 1e-3  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getG(), -0.200, 1e-3  ) );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianTransformation);
TESTER_TEST(testMarginalization1);
TESTER_TEST(testMul1);
TESTER_TEST_SUITE_END();
#endif