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

#ifndef NLL_ALGORITHM_GAUSSIAN_REPRESENTATION_H_
# define NLL_ALGORITHM_GAUSSIAN_REPRESENTATION_H_

# pragma warning( push )
# pragma warning( disable:4251 ) // class XXX needs to have dll-interface to be used by clients of class YYY


namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Represent a multivariate gaussian in its canonical form
    @see "A Technique for Painless Derivation of Kalman Filtering Recursions", Ali Taylan Cemgil [1]
         http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.20.3377

    p(x) = exp( g + h^t * x - 0.5 x^t * K * x )
    */
   class NLL_API PotentialGaussianCanonical
   {
   public:
      typedef double                      value_type;
      typedef core::Matrix<value_type>    Matrix;
      typedef core::Buffer1D<value_type>  Vector;
      typedef core::Buffer1D<size_t>      VectorI;

   public:
      PotentialGaussianCanonical( const Vector& h, const Matrix& k, value_type g, const VectorI id = VectorI() )
      {
         ensure( PotentialTable::isDomainSorted( id ), "the id list must be ordered first" );
         _h = h;
         _k = k;
         _g = g;

         if ( id.size() == 0 )
         {
            // generate the id
            _id = VectorI( h.size() );
            for ( size_t n = 0; n < _id.size(); ++n )
            {
               _id[ n ] = n;
            }
         } else {
            _id = id;
         }
      }

      PotentialGaussianCanonical()
      {
         // nothing, unsusable moment
      }

      /**
       @brief computes p(x)
       */
      value_type value( const Vector& x ) const
      {
         ensure( x.size() == _h.size(), "size doesn't match" );
         return std::exp( _g + core::generic_dot<Vector, value_type>( x, _h, x.size() ) + core::fastDoubleMultiplication( x, _k ) );
      }

      PotentialGaussianCanonical operator*( const PotentialGaussianCanonical& g2 ) const
      {
         PotentialGaussianCanonical g = *this;
         return g.mul( g2 );
      }

      PotentialGaussianCanonical operator/( const PotentialGaussianCanonical& g2 ) const
      {
         PotentialGaussianCanonical g = *this;
         return g.div( g2 );
      }

      /**
       @brief computes g(k1, h1, g1) * g(k2, h2, g2) = g( k1 + k2, h1 + h2, g1 + g2) if the 2 gaussians
       have the same scope, else, we need to padd with 0s on the missing variables
       */
      PotentialGaussianCanonical mul( const PotentialGaussianCanonical& g2 ) const
      {
         if ( deepCheck( _id, g2._id ) )
         {
            // simple case
            return PotentialGaussianCanonical( _h + g2._h, _k + g2._k, _g + g2._g, _id );
         } else {
            return opGeneric( g2, std::plus<value_type>() );
         }
      }

      /**
       @brief computes g(k1, h1, g1) / g(k2, h2, g2) = g( k1 - k2, h1 - h2, g1 - g2) if the 2 gaussians
       have the same scope, else, we need to padd with 0s on the missing variables
       */
      PotentialGaussianCanonical div( const PotentialGaussianCanonical& g2 ) const
      {
         if ( deepCheck( _id, g2._id ) )
         {
            // simple case
            return PotentialGaussianCanonical( _h + g2._h, _k + g2._k, _g + g2._g, _id );
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
      PotentialGaussianCanonical marginalization( const VectorI& varIndexToRemove ) const
      {
         // compute the remaining set
         std::vector<size_t> ids;
         std::vector<size_t> mids;
         computeIndexInstersection( varIndexToRemove, ids, mids );
         ensure( mids.size() == varIndexToRemove.size(), "wrong index: some vars are missing!" );

         // create the hx and hy subvectors
         Matrix hx( (size_t)ids.size(), 1 );
         VectorI indexNew( hx.size() );
         for ( size_t n = 0; n < hx.size(); ++n )
         {
            hx[ n ] = _h[ ids[ n ] ];
            indexNew[ n ] = _id[ ids[ n ] ];
         }

         Matrix hy( (size_t)mids.size(), 1 );
         for ( size_t n = 0; n < hy.size(); ++n )
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

         // new params:
         Matrix      knew = xx - xyyyinv * yx;
         Matrix      hnew = hx - xyyyinv * hy;

         // Note: det A^-1 = 1 / det A
         ensure( !core::equal( detyyinv, 0.0, 1e-10 ), "the determinant is null!" );
         value_type  gnew = _g - std::log( PotentialGaussianMoment::normalizeGaussian( 1 / detyyinv, yy.sizex() ) ) + 0.5 * core::fastDoubleMultiplication( hy, yyinv );
         return PotentialGaussianCanonical( hnew, knew, gnew, indexNew );
      }

      /**
       @brief computes p(X, Y=y) i.e., entering evidence Y = y
       @param vars the values of Y=y
       @param varsIndex the index of Y's, must be sorted 0->+inf
       */
      PotentialGaussianCanonical conditioning( const Vector& vars, const VectorI& varsIndex ) const
      {
         // sort the data
         std::vector<size_t> ids;
         std::vector<size_t> mids;
         computeIndexInstersection( varsIndex, ids, mids );

         Matrix xx, yy, xy, yx;
         partitionMatrix( _k, ids, mids, xx, yy, xy, yx );

         Vector hx( (size_t)ids.size() );
         VectorI indexNew( hx.size() );
         for ( size_t n = 0; n < hx.size(); ++n )
         {
            hx[ n ] = _h[ ids[ n ] ];
            indexNew[ n ] = _id[ ids[ n ] ];
         }

         Vector hy( (size_t)mids.size() );
         for ( size_t n = 0; n < hy.size(); ++n )
         {
            hy[ n ] = _h[ mids[ n ] ];
         }

         // compute the new parameters
         Vector hnew = Matrix( hx, hx.size(), 1 ) - xy * Matrix( vars, vars.size(), 1 );
         Matrix knew = xx;
         value_type gnew = _g + core::dot( hy, vars ) - 0.5 * core::fastDoubleMultiplication( vars, yy );
         return PotentialGaussianCanonical( hnew, knew, gnew, indexNew );
      }

      PotentialGaussianMoment toGaussianMoment() const
      {
         Matrix cov;
         cov.clone( _k );

         value_type detk;
         const bool r = core::inverse( cov, &detk );
         ensure( r, "can't inverse precision matrix" );
         
         Vector mean = cov * Matrix( _h,  _h.size(), 1 );

         ensure( detk > 0, "determinant K must be > 0" );
         const value_type alpha = std::exp( _g + 0.5 * core::fastDoubleMultiplication( _h, cov ) );

         return PotentialGaussianMoment( mean, cov, _id, alpha );
      }

      void print( std::ostream& o ) const
      {
         o << "PotentialGaussianCanonical:" << std::endl;
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
      PotentialGaussianCanonical opGeneric( const PotentialGaussianCanonical& g2, const Functor& op ) const
      {
         // compute the id intersection and new order
         const size_t s1 = _id.size();
         const size_t s2 = g2._id.size();
         const VectorI& v1 = _id;
         const VectorI& v2 = g2._id;
         const size_t MAX = std::numeric_limits<size_t>::max();
         
         // we know the ids are ordered, which will make the comparison very simple
         size_t id1 = 0;
         size_t id2 = 0;

         std::vector<size_t> index1;
         std::vector<size_t> index2;
         std::vector<size_t> ids;
         index1.reserve( std::max( s1, s2 ) );
         index2.reserve( std::max( s1, s2 ) );
         ids.reserve( std::max( s1, s2 ) );
         while ( true )
         {
            if ( id1 == s1 && id2 == s2 )
               break;
            const size_t val1 = ( id1 == s1 ) ? ( MAX - 2 ) : v1[ id1 ];
            const size_t val2 = ( id2 == s2 ) ? ( MAX - 1 ) : v2[ id2 ];

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
         const size_t newSize = static_cast<size_t>( index1.size() );
         Vector h( newSize );
         Matrix k( newSize, newSize );
         VectorI mergedId( newSize );
         for ( size_t n = 0; n < newSize; ++n )
         {
            mergedId[ n ] = ids[ n ];
            const size_t id1 = index1[ n ];
            const size_t id2 = index2[ n ];
            if ( id1 != MAX )
            {
               h[ n ] = op( h[ n ], _h[ id1 ] );

               for ( size_t nn = 0; nn < newSize; ++nn )
               {
                  const size_t id1b = index1[ nn ];
                  if ( id1b != MAX )
                  {
                     k( n, nn ) = _k( id1, id1b );
                  }
               }
            }
            if ( id2 != MAX )
            {
               h[ n ] = op( h[ n ], g2._h[ id2 ] );

               for ( size_t nn = 0; nn < newSize; ++nn )
               {
                  const size_t id2b = index2[ nn ];
                  if ( id2b != MAX )
                  {
                     k( n, nn ) = op( k( n, nn ), g2._k( id2, id2b ) );
                  }
               }
            }
         }

         return PotentialGaussianCanonical( h, k, op( _g, g2._g ), mergedId );
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

      static bool deepCheck( const VectorI& v1, const VectorI& v2 )
      {
         if ( v1.size() != v2.size() )
            return false;
         const size_t s = v1.size();

         for ( size_t n = 0; n < s; ++n )
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
