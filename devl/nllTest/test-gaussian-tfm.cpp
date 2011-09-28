#include <nll/nll.h>
#include <tester/register.h>

namespace nll
{
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
    @brief Computes quickly v^t * m * v
    */
   template <class Vector, class Matrix>
   typename Matrix::value_type fastDoubleMultiplication( const Vector& v, const Matrix& m )
   {
      typedef typename Matrix::value_type value_type;

      const ui32 sizex = m.sizex();
      const ui32 sizey = m.sizey();

      ensure( v.size() == sizex, "dim don't match" );

      value_type accum = 0;
      for ( ui32 y = 0; y < sizey; ++y )
      {
         const value_type px = v[ y ];
         for ( ui32 x = 0; x < sizex; ++x )
         {
            accum += m( y, x ) * px * v[ x ];
         }
      }
      return accum;
   }

   /**
    @brief Represent a multivariate gaussian in its canonical form

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
         return std::exp( _g + core::generic_dot<Vector, value_type>( x, _h, x.size() ) + fastDoubleMultiplication( x, _k ) );
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
            index1.reserve( std::max( s1, s2 ) );
            index2.reserve( std::max( s1, s2 ) );
            while ( true )
            {
               if ( id1 == s1 && id2 == s2 )
                  break;
               const ui32 val1 = v1[ id1 ];
               const ui32 val2 = v2[ id2 ];

               if ( val1 == val2 )  // same, so just get the ID
               {
                  index1.push_back( val1 );
                  index1.push_back( val2 );
                  ++id1;
                  ++id2;
               } else if ( val1 < val2 ) // v1 has one value more than v2, so padd v2 with MAX
               {
                  index1.push_back( val1 );
                  index2.push_back( MAX );
               } else if ( val2 < val1 )
               {
                  index2.push_back( val2 );
                  index1.push_back( MAX );
               }
            }

            // finally extract the data
            const ui32 newSize = static_cast<ui32>( index1.size() );
            Vector h( newSize );
            Matrix k( newSize, newSize );
            for ( ui32 n = 0; n < newSize; ++n )
            {
               const ui32 id1 = index1[ n ];
               const ui32 id2 = index2[ n ];
               if ( id1 != MAX )
                  h[ n ] += _h[ id1 ];
               if ( id2 != MAX )
                  h[ n ] += g2._h[ id2 ];
               // TODO fill K
            }

            return GaussianMultivariateCanonical( h, k, _g + g2._g, _id );
         }
      }

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

class TestGaussianTransformation
{
public:
   void test1()
   {
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianTransformation);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
#endif