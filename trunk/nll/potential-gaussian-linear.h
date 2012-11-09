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

#ifndef NLL_ALGORITHM_POTENTIAL_GAUSSIAN_LINEAR_H_
# define NLL_ALGORITHM_POTENTIAL_GAUSSIAN_LINEAR_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Efficiently represent a factored gaussian joint distribution
    @note we are building up on the PotentialGaussian* which are used to do the math

    - The domain must be ordered so that domain[ 0 ] < domain[ 1 ] < domain[ 2 ] ... meaning that in a graph,
    - The parent nodes must have a higher domain than the children.
    - The parents are also ordered between themselves

    We model the conditional gaussian as below: let's define  variables y depending on the parents x with the corresponding weight vector w, it represents this potential:
    p(y | x1..xn) = N(b0 + w^t * x; Sigma )

    @note BNT doesn't follow the same notations for g
    @see http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.32.6529
    */
   class PotentialLinearGaussian
   {
   public:
      typedef double                      value_type;
      typedef core::Matrix<value_type>    Matrix;
      typedef core::Buffer1D<value_type>  Vector;
      typedef core::Buffer1D<size_t>      VectorI;

   public:
      struct Dependency
      {
         Dependency()
         {}

         Dependency( std::shared_ptr<PotentialLinearGaussian> p, value_type w ) : potential( p ), weight( w )
         {}

         std::shared_ptr<PotentialLinearGaussian>  potential;
         value_type weight;
      };
      typedef std::vector<Dependency>  Dependencies;

      PotentialLinearGaussian()
      {}

      /**
       @brief Intanciate a potential without dependencies
       @param m mean
       @param c covariance
       @param id naming of the variable
       */
      PotentialLinearGaussian( const PotentialGaussianMoment& potential ) : _potential( potential )
      {}

      /**
      @brief Intanciate a potential with dependencies
       @param m mean
       @param c covariance
       @param id naming of the variable
       @param dependencies the dependencies of the linear gaussian. Dependencies must be ordered from lowest to highest id
       */
      PotentialLinearGaussian( const PotentialGaussianMoment& potential, Dependencies& dependencies ) : _potential( potential ), _dependencies( dependencies )
      {
         // we don't represent it with multi gaussian, as it doesn't seem to be very useful for LG. It would be more difficult to configure...
         ensure( potential.getDomain().size() == 1, "must only represents a 1-variable node, however, it is possible to extend it to multi valued nodes" );
         ensure( dependencies.size() > 0, "use other constructor!" );
         ensure( isOrdered( _dependencies ), "dependencies must be ordered too! higher->lower" );

         // check the dependency assumption: 
         const size_t current = potential.getDomain()[ 0 ];
         for ( size_t n = 0; n < dependencies.size(); ++n )
         {
            for ( size_t nn = 0; nn < dependencies[ n ].potential->getDomain().size(); ++nn )
            {
               const size_t parent = dependencies[ n ].potential->getDomain()[ nn ];
               ensure( parent > current, "the dependencies must be ordered from min->max id" );
            }
         }
      }

      const Vector& getMean() const
      {
         return _potential.getMean();
      }

      const Matrix& getCov() const
      {
         return _potential.getCov();
      }

      const VectorI& getDomain() const
      {
         return _potential.getDomain();
      }

      const Dependencies& getDependencies() const
      {
         return _dependencies;
      }

      /**
       @brief Create a canonical potential out of the linear gaussian format. The purpose is to create a joint gaussian instead of the conditional gaussian!

       Construct the potential representing p(Y|X) in canonical form
       */
      PotentialGaussianCanonical toGaussianCanonical() const
      {
         // get all our variables
         std::vector<const VectorI*> idsptr( _dependencies.size() + 1 );
         for ( size_t n = 0; n < _dependencies.size(); ++n )
         {
            idsptr[ n ] = &(_dependencies[ n ].potential->getDomain());
         }
         idsptr[ _dependencies.size() ] = &_potential.getDomain();
         const std::vector<size_t> ids = mergeIds( idsptr );

         // expand all the weights into a single vector (i.e., in case of multivariate nodes)
         const Vector ww = expandWeights( _dependencies );

         // prepare the parameters of the canonical potential
         Matrix sinv;
         sinv.clone( _potential.getCov() );
         value_type detcov = 0;
         core::inverse( sinv, &detcov );

         const value_type gaussCte = PotentialGaussianMoment::normalizeGaussian( detcov, sinv.sizex() );
         const value_type g = -0.5 * core::fastDoubleMultiplication( _potential.getMean(), sinv ) + std::log( gaussCte );

         // w should be a matrix, not a vector as we have now...
         const Matrix wt = Matrix( ww, 1, ww.size() ); // w is a matrix for multivariate
         const Matrix w = Matrix( ww, ww.size(), 1 );
         const Matrix covinvmean = sinv * _potential.getMean();
         const Matrix wcovinvmean = w * covinvmean;

         // compute h = | covinv * mean        |
         //             | -w * covinv * mmean |
         Vector h( static_cast<size_t>( ids.size() ) );
         for ( size_t n = 0; n < covinvmean.sizey(); ++n )
         {
            h[ n ] = covinvmean( n, 0 );
         }
         for ( size_t n = 0; n < wcovinvmean.sizey(); ++n )
         {
            h[ n + covinvmean.sizey() ] = - wcovinvmean( n, 0 );
         }
        
         // construct this matrix:
         // K = | covinv          -covinv * w'   |
         //     | -w * covinv    w * covinv * w' |
         const Matrix wcovinv = w * sinv;
         const Matrix wcovinvwt = wcovinv * wt;
         ensure( wcovinvwt.sizex() == wcovinvwt.sizey(), "must be square byb design" );

         Matrix k( (size_t)ids.size(), (size_t)ids.size() );
         for ( size_t y = 0; y < sinv.sizey(); ++y )
         {
            for ( size_t x = 0; x < sinv.sizex(); ++x )
            {
               k( y, x ) = sinv( y, x );
            }
         }

         for ( size_t y = 0; y < wcovinvwt.sizey(); ++y )
         {
            for ( size_t x = 0; x < wcovinvwt.sizex(); ++x )
            {
               k( y + sinv.sizey(), x + sinv.sizex() ) = wcovinvwt( y, x );
            }
         }

         for ( size_t y = 0; y < wcovinv.sizey(); ++y )
         {
            for ( size_t x = 0; x < wcovinv.sizex(); ++x )
            {
               k( y + sinv.sizey(), x ) = - wcovinv( y, x );
               k( x, y + sinv.sizex() ) = - wcovinv( y, x );
            }
         }

         VectorI idsi( static_cast<size_t>( ids.size() ) );
         std::copy( ids.begin(), ids.end(), idsi.begin() );
         return PotentialGaussianCanonical( h, k, g, idsi );
      }

   private:
      // expand all the weights into a single vector (i.e., in case of multivariate nodes)
      static Vector expandWeights( const Dependencies& dps )
      {
         // first count the depdendencies
         size_t nbDependencies = 0;
         for ( size_t n = 0; n < dps.size(); ++n )
         {
            nbDependencies += dps[ n ].potential->getDomain().size();
         }

         // then expand them into a single vector
         Vector expandedw( nbDependencies );
         size_t index = 0;   // we start with the dependend variables
         for ( size_t n = 0; n < dps.size(); ++n )
         {
            for ( size_t nbVar = 0; nbVar < dps[ n ].potential->getDomain().size(); ++nbVar )
            {
               expandedw[ index ] = dps[ n ].weight;
               ++index;
            }
         }
         return expandedw;
      }

      static bool isOrdered( const Dependencies& dps )
      {
         int last = std::numeric_limits<int>::min();
         for ( size_t n = 0; n < dps.size(); ++n )
         {
            for ( size_t id = 0; id < dps[ n ].potential->getDomain().size(); ++id )
            {
               const int current = static_cast<int>( dps[ n ].potential->getDomain()[ id ] );
               if ( last > current )
                  return false;
               last = current;
            }
         }
         return true;
      }

      /**
       @brief given a list if domains, build the merged domain
       */
      static std::vector<size_t> mergeIds( const std::vector<const VectorI*>& lists )
      {
         size_t maxIds = 0;
         for ( size_t n = 0; n < lists.size(); ++n )
         {
            maxIds += lists[ n ]->size();
         }

         if ( maxIds == 0 )
            return std::vector<size_t>();   // empty list

         std::vector<size_t> merged;
         merged.reserve( maxIds );
         std::vector<size_t> pointer( lists.size() );


         // the list are already sorted, so we can simply do a mergesort
         while (1)
         {
            size_t lastMin = std::numeric_limits<size_t>::max();
            size_t minIndex = 0;
            size_t min = std::numeric_limits<size_t>::max();  // we don't want duplicates...
            for ( size_t n = 0; n < lists.size(); ++n )
            {
               const size_t pointerIndex = pointer[ n ];
               if ( pointerIndex < lists[ n ]->size() )
               {
                  const size_t id = static_cast<size_t>( (*lists[ n ])[ pointerIndex ] );
                  if ( id < min && id != lastMin )
                  {
                     minIndex = n;
                     min = id;
                  }
               }
            }

            if ( min == std::numeric_limits<size_t>::max() || ( merged.size() && min == *merged.rbegin() ) )
               break;   // no more min this iteration
            merged.push_back( min );
            ++pointer[ minIndex ];
            lastMin = min;
         }

         return merged;
      }

   private:
      PotentialGaussianMoment    _potential;   // only the original potential, i.e., without the dependencies
      Dependencies               _dependencies;
   };
}
}

#endif
