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

#ifndef NLL_ALGORITHM_LOCALLY_EMBEDDING_H_
# define NLL_ALGORITHM_LOCALLY_EMBEDDING_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Locally linear embedding algorithm implementation

      Locally linear embedding (LLE) is an unsupervised learning algorithm
      that computes low dimensional, neighborhood preserving embeddings of high dimensional data.
      The data, assumed to be sampled from an underlying manifold, are mapped into a single global
      coordinate system of lower dimensionality. The mapping is derived from the symmetries of locally
      linear reconstructions, and the actual computation of the embedding reduces to a sparse eigenvalue
      problem. Notably, the optimizations in LLE—though capable of generating highly nonlinear
      embeddings—are simple to implement, and they do not involve local minima

      The trick is to do a different linear dimensionality reduction at each point
      (because locally a manifold looks linear) and then combine these with minimal discrepancy. It
      was introduced by Roweis and Saul (2000).

      This method is mainly aimed at displaying high dimentional data

      Note the implementation could be improved a lot by using sparse matrix and a SVD implementation
      working on sparse matrix that do not compute all the EIG/EIV but only the ones needed.

    @see http://jmlr.csail.mit.edu/papers/volume4/saul03a/saul03a.pdf (reference)
         http://www.stat.cmu.edu/~cshalizi/350/lectures/14/lecture-14.pdf for details

    @note it is not possible to project unseen points without recomputing the with the full set of points making
         impracticable to test previously unseen points. To overcome this point, an incremental algorithm can
         be implemented, or train a regression algorithm such as neural network to extrapolate to unseen points
    */
   class LocallyLinearEmbedding
   {
   public:
      /**
       @brief Transform a set of points in high dimention to a lower dimention point
       */
      template <class Points>
      std::vector<typename Points::value_type> transform( const Points& points, ui32 newDim, ui32 nbNeighbours, double eps = 1e-5 ) const
      {
         typedef typename Points::value_type Point;
         typedef KdTree<Point, MetricEuclidian<Point>, 5, Points> KdTreeT;
         typedef core::Matrix<double>  Matrix;

         if ( points.size() == 0 )
            return std::vector<Point>();

         const ui32 dim = (ui32)points[ 0 ].size();
         const ui32 nbPoints = (ui32)points.size();

         {
            std::stringstream ss;
            ss << "computing locally linear embedding." << std::endl <<
                  " number of points=" << nbPoints << std::endl <<
                  " dimentionality=" << dim << std::endl <<
                  " new dimentionality=" << newDim << std::endl <<
                  " number of neightbours=" << nbNeighbours << std::endl <<
                  " epsilon=" << eps;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         ensure( dim > newDim, "the new dimension is too high" );
         ensure( nbNeighbours < points.size(), "problem in neighbours: no enough" );

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, " computing kd-tree..." );
         MetricEuclidian<Point> metric;
         KdTreeT kdtree( metric );
         kdtree.build( points, dim );

         // compute the reconstruction weights
         //
         // z_j = x_j - x_i
         // gi = zz^t
         // wi = lambda / 2 * (gi + alpha * id)^-1 * 1vec
         //
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, " computing reconstruction weights..." );
         std::vector< core::Buffer1D<double> > wis( nbPoints );
         std::vector< std::vector<ui32> > neighbours( nbPoints );
         for ( ui32 i = 0; i < nbPoints; ++i )
         {
            KdTreeT::NearestNeighborList nn = kdtree.findNearestNeighbor( points[ i ], nbNeighbours + 1 ); // nbNeighbours + 1, as nn[ 0 ] is the point we are starting from
            ui32 n = 0;
            std::vector<ui32> index( nbNeighbours );
            for ( KdTreeT::NearestNeighborList::iterator it = ++nn.begin(); it != nn.end(); ++it, ++n )
               index[ n ] = it->id;
            neighbours[ i ] = index;

            // create z_j = xj - xi
            Matrix pp( dim, nbNeighbours );
            for ( ui32 j = 0; j < nbNeighbours; ++j )
            {
               const ui32 ii = index[ j ];
               for ( ui32 k = 0; k < dim; ++k )
               {
                  const double va = points[ ii ][ k ];
                  const double vb = points[ i ][ k ];
                  pp( k, j ) = va - vb;
               }
            }

            Matrix giint = core::covarianceCentred( pp );
            for ( ui32 j = 0; j < giint.size(); ++j )
               giint[ j ] *= nbNeighbours;

            // add regularization in case gi is not invertible
            const double trace = core::trace( giint );
            for ( ui32 j = 0; j < nbNeighbours; ++j )
               giint( j, j ) += eps * trace;
            core::inverse( giint );  // TODO: instead: slove CX = 1

            core::Buffer1D<double> wi( nbNeighbours );
            for ( ui32 j = 0; j < nbNeighbours; ++j )
            {
               double accum = 0;
               for ( ui32 k = 0; k < nbNeighbours; ++k )
               {
                  accum += giint( j, k );
               }
               wi[ j ] = accum;
            }
            double sum = 0;
            for ( ui32 j = 0; j < nbNeighbours; ++j )
            {
               sum += wi[ j ];
            }
            for ( ui32 j = 0; j < nbNeighbours; ++j )
            {
               wi[ j ] /= sum;
            }
            wis[ i ] = wi;
         }

         //
         // Now reconstruct the Y minimizing sum_i || y_i - sum_i!=j (y_j*w_ij)||^2
         // with constraints 1/n sum_i y_i = 0  (translation fixed on the center)
         //                  1/n* y^t * y = Id  (rotation and variance fixed)
         // so that the problem is well posed
         //

         // define M = (I-W)'*(I-W)
         // TODO USE SPARSE MATRIX!!!
         Matrix m( nbPoints, nbPoints, false );
         core::identity( m );

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, " computing M..." );
         for ( ui32 ii = 0; ii < nbPoints; ++ii )
         {
            for ( ui32 jj = 0; jj < nbNeighbours; ++jj )
            {
               const ui32 id = neighbours[ ii ][ jj ];
               m( ii, id ) -= wis[ ii ][ jj ];
               m( id, ii ) -= wis[ ii ][ jj ];
            }

            Matrix mm;
            core::mulidt( wis[ ii ], mm );
            for ( ui32 y = 0; y < nbNeighbours; ++y )
            {
               const ui32 idy = neighbours[ ii ][ y ];
               for ( ui32 x = 0; x < nbNeighbours; ++x )
               {
                  const ui32 idx = neighbours[ ii ][ x ];
                  m( idy, idx ) += mm( y, x );
               }
            }
         }

         // extract the main components
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, " computing SVD of M..." );
         core::Buffer1D<double> eig;
         Matrix eiv;
         bool result = core::svdcmp( m, eig, eiv );
         if ( !result )
         {
            throw std::runtime_error( "singular matrix!" );
         }

         // sort the eigen values, we are interested in the newDim^th eigen vectors, except the first one which is always
         // zero by definition
         std::vector< std::pair<double, ui32> > pairs;
         for ( ui32 n = 0; n < eig.size(); ++n )
         {
            pairs.push_back( std::make_pair( eig[ n ], n ) );
         }
         std::sort( pairs.begin(), pairs.end() );

         {
            Matrix eigs( nbPoints, newDim );
            for ( ui32 n = 0; n < nbPoints; ++n )
            {
               for ( ui32 m = 0; m < newDim; ++m )
               {
                  eigs( n, m ) = eiv( n, pairs[ m + 1 ].second ); // skip the first eig as it is 0 and uninformative
               }
            }

            std::stringstream ss;
            ss << "main eigen vectors=" << std::endl;
            eigs.print( ss );

            ss << "eigen values=";
            for ( ui32 m = 0; m < newDim; ++m )
            {
               ss << pairs[ m + 1 ].first << " ";
            }
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         std::vector<Point> tfmPoints( nbPoints );
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            Point p( newDim );
            for ( ui32 nn = 0; nn < newDim; ++nn )
            {
               const ui32 index = pairs[ nn + 1 ].second;
               p[ nn ] = eiv( n, index );
            }
            tfmPoints[ n ] = p;
         }
         return tfmPoints;
      }
   };
}
}

#endif
