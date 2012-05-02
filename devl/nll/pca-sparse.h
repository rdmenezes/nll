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


#ifndef NLL_ALGORITHM_PCA_SPARSE_H_
# define NLL_ALGORITHM_PCA_SPARSE_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief PCA algorithm with the assumption that the dimentionality of a sample is much higher than the number of samples

    Given a set of samples of size M, A = [s_0..s_M]

    pca computes the eigen vectors/values such that C = 1 / M sum_i( s_i * s_t')
                                                      = 1 / M * A * A'
    However, with the asumption of M << dimentionality as it is often the case in machine vision algorithms, computing the PCA is
    in too costly as its computation is in o(dimentionality), we can do it in o(M) as below:
    
    Given an eigen vector/value pair (v_i, va_i),
    we have AA' * v_i = va_i * v_i

    now consider the covariance A'A instead:
                   A' * A * u_i = ua_i * u_i
               A * A' * A * u_i = ua_i * A * u_i

    => we can see that A * u_i is an eigen vector of A * A', solving the problem efficiently in the size of M instead of the dimentionality
       of a sample

    @see http://en.wikipedia.org/wiki/Eigenface
    @see http://www.face-rec.org/algorithms/PCA/jcn.pdf
    @see http://www.vision.jhu.edu/teaching/vision08/Handouts/case_study_pca1.pdf
       Eigen faces for recognition, 1991
       Matthew Turk and Alex Pentland
       Vision and Modeling Group, The Media Laboratory
      */
   template <class Points>
   class PcaSparseSimple
   {
   public:
      typedef typename Points::value_type    Point;
      typedef std::pair<double, ui32>        Pair;
      typedef std::vector<Pair>              Pairs;
      typedef core::Matrix<double>           Matrix;
      typedef core::Buffer1D<double>         Vector;


   public:
      /**
       @brief Computes PCA, decide the number of eigen vectors retain by the retained variance
       */
      bool computeByVarianceRetained( const Points& points, double varianceToRetain )
      {
         if ( points.size() == 0 )
            return false;
         const bool succeeded = _computeEigenVectors( points );
         if ( !succeeded )
            return false;

         // set the number of eigen vectors according to the retained variance
         double eivSum = 0;
         for ( ui32 n = 0; n < _eigenValues.size(); ++n )
         {
            if ( _eigenValues[ n ] > 0 )
               eivSum += _eigenValues[ n ];
         }

         double eivSumTmp = 0;
         _nbVectors = 1; // at least 1 component
         for ( ui32 n = 0; n < _eigenValues.size(); ++n )
         {
            const ui32 eivIndex = _pairs[ n ].second;
            eivSumTmp += _eigenValues[ eivIndex ];
            const double ratio = eivSumTmp / eivSum;
            if ( ratio >= varianceToRetain )
            {
               _nbVectors = n + 1;
               break;
            }
         }

         // set the transformation
         _projection = _makeProjection();
         return true;
      }

      /**
       @brief Computes PCA with a specific number of eigen vectors to retain
       */
      bool compute( const Points& points, ui32 nbEigenvectors )
      {
         _nbVectors = nbEigenvectors;

         if ( points.size() == 0 )
            return false;
         const bool succeeded = _computeEigenVectors( points );
         if ( succeeded )
            return false;

         // set the transformation
         _projection = _makeProjection();
         return true;
      }

      /**
       @brief Project a point using the previously computed projection.
       */
      template <class Point2>
      Point2 process( const Point2& point ) const
      {
         ensure( _projection.size(), "Empty projection" );
         core::Matrix<double> p( static_cast<ui32>( point.size() ), 1 );
         for ( ui32 n = 0; n < p.size(); ++n )
            p[ n ] = point[ n ] - _mean[ n ];
         core::Matrix<double> r = core::mul( _projection, p );
         Point2 result( r.size() );
         for ( ui32 n = 0; n < r.size(); ++n )
            result[ n ] = static_cast<typename Point2::value_type>( r[ n ] );
         return result;
      }

      /**
       @brief reconstruct a point projected on the PCA back to the original space
       */
      template <class Point2>
      Point2 reconstruct( const Point2& point ) const
      {
         assert( point.size() == _nbVectors );
         assert( _eigenVectors.size() );

         const ui32 finalSize = _mean.size();

         Point2 reconstructed( finalSize );
         for ( ui32 n = 0; n < _nbVectors; ++n )
         {
            for ( ui32 nn = 0; nn < finalSize; ++nn )
            {
               reconstructed[ nn ] += point[ n ] * _eigenVectors( nn, _pairs[ n ].second );  // eigen Vectors are in column
            }
         }

         for ( ui32 nn = 0; nn < finalSize; ++nn )
         {
            reconstructed[ nn ] += _mean[ nn ];
         }

         return reconstructed;
      }

      /**
       @brief This should only be called after a successful <code>compute()</code> This method change the transformation
              Vector without recalculating a SVD.
       */
      void setNbVectors( ui32 nbVectors )
      {
         _nbVectors = nbVectors;
         _projection = _makeProjection();
      }

      /**
       @brief Return the number of eigen vectors for projection
       */
      ui32 getNbVectors() const
      {
         return _nbVectors;
      }

      /**
       @brief Return the mean vector (ie when <code>compute</code>)
       */
      const Matrix& getMean() const
      {
         return _mean;
      }

      /**
       @brief Return the eigen vectors. They are arranged by column (each column is an eigen vector)

       @note the eigen vectors are not sorted by importance, use <getPairs()> to get the order
       */
      const Matrix& getEigenVectors() const
      {
         return _eigenVectors;
      }

      /**
       @brief Return the eigen values

       @note the eigen vectors are not sorted by importance, use <getPairs()> to get the order
       */
      const Vector& getEigenValues() const
      {
         return _eigenValues;
      }

      /**
       @brief Return the current projection
       @note sorted from highest to lowest eigen value

       Each row is an eigen vector
       */
      const Matrix& getProjection() const
      {
         return _projection;
      }

      const Pairs& getPairs() const
      {
         return _pairs;
      }

      /**
       @brief Read from a file the status of the algorithm
       */
      void read( std::istream& i )
      {
         core::read<ui32>( _nbVectors, i );
         _mean.read( i );
         _eigenVectors.read( i );
         _eigenValues.read( i );
         _projection.read( i  );
         ui32 psize = 0;
         core::read<ui32>( psize, i );
         for ( ui32 n = 0; n < psize; ++n )
         {
            double v1;
            ui32 v2;
            core::read<double>( v1, i );
            core::read<ui32>( v2, i );
            _pairs.push_back( Pair( v1, v2 ) );
         }
      }

      void read( const std::string& path )
      {
         std::ifstream f( path.c_str(), std::ios::binary );
         ensure( f.good(), "can't load PCA binaries" );

         read( f );
      }

      /**
       @brief Write to a file the status of the algorithm
       */
      void write( std::ostream& o ) const
      {
         core::write<ui32>( _nbVectors, o );
         _mean.write( o );
         _eigenVectors.write( o );
         _eigenValues.write( o );
         _projection.write( o  );
         ui32 psize = static_cast<ui32>( _pairs.size() );
         core::write<ui32>( psize, o );
         for ( ui32 n = 0; n < psize; ++n )
         {
            core::write<double>( _pairs[ n ].first, o );
            core::write<ui32>( _pairs[ n ].second, o );
         }
      }

      void write( const std::string& path ) const
      {
         std::ofstream f( path.c_str(), std::ios::binary );
         ensure( f.good(), "can't write PCA binaries" );

         write( f );
      }

   private:
      bool _computeEigenVectors( const Points& points )
      {
         const ui32 nbPoints = static_cast<ui32>( points.size() );
         const ui32 dimensionality = static_cast<ui32>( points[ 0 ].size() );

         #ifdef NLL_SECURE
         for ( ui32 n = 1; n < points.size(); ++n )
         {
            ensure( points[ n ].size() == dimensionality, "points must have the same dimensionality" );
         }
         #endif

         // copy the data
         Matrix data( nbPoints, dimensionality );
         for ( ui32 p = 0; p < nbPoints; ++p )
         {
            for ( ui32 n = 0; n < dimensionality; ++n )
            {
               data( p, n ) = static_cast<double>( points[ p ][ n ] );
            }
         }
         _mean = core::meanRow( data, 0, nbPoints - 1 );

         // substract the mean
         for ( ui32 p = 0; p < nbPoints; ++p )
         {
            for ( ui32 n = 0; n < dimensionality; ++n )
            {
               data( p, n ) -= _mean[ n ];
            }
         }

         // compute A'A, which is a symmetric matrix
         Matrix m( nbPoints, nbPoints );
         for ( ui32 y = 0; y < nbPoints; ++y )
         {
            for ( ui32 x = y; x < nbPoints; ++x )
            {
               // computes inner product of each pair of samples
               double val = 0;
               for ( ui32 n = 0; n < dimensionality; ++n )
               {
                  val += data( x, n ) * data( y, n );
               }
               val /= nbPoints;

               // copy
               m( y, x ) = val;
               m( x, y ) = val;
            }
         }

         // compute the eigen vectors/values of A'A
         Matrix eigenVectors;
         Vector eigenValues;
         const bool succeeded = core::svdcmp( m, eigenValues, eigenVectors );
         if ( !succeeded )
            return false;

         // compute the eigen vector sort
         Pairs sort;
         for ( ui32 n = 0; n < eigenValues.size(); ++n )
            sort.push_back( Pair( eigenValues[ n ], n ) );
         std::sort( sort.rbegin(), sort.rend() );
         _pairs = sort;

         // now we just want to use the eigen values that are > than a specific threshold
         ui32 nbEigens = 0;
         for ( ui32 n = 0; n < (ui32)_pairs.size(); ++n )
         {
            nbEigens = n;
            if ( sort[ n ].first < 0.1 )
            {
               ensure( n != 0, "nothing in this matrix!" );
               break;
            }
         }

         // finally extract the eigen vectors of AA'
         _eigenVectors = Matrix( dimensionality, nbEigens );
         _eigenValues = Vector( nbEigens );
         for ( ui32 ei = 0; ei < nbEigens; ++ei )
         {
            Vector eiv( dimensionality );
            for ( ui32 p = 0; p < nbPoints; ++p )
            {
               const ui32 indexEigen = _pairs[ ei ].second;
               const double vlk = eigenVectors( p, indexEigen );
               for ( ui32 n = 0; n < dimensionality; ++n )
               {
                  eiv[ n ] += data( p, n ) * vlk;
               }
            }

            // copy the eiv and normalize them
            const double norm = core::norm2( eiv );
            for ( ui32 n = 0; n < dimensionality; ++n )
            {
               _eigenVectors( n, ei ) = eiv[ n ] / norm;
            }

            // copy the eigen value
            _eigenValues[ ei ] = _pairs[ ei ].first;
         }

         // we sorted the eig/eig, so reset the _pairs
         for ( size_t n = 0; n < _pairs.size(); ++n )
         {
            _pairs[ n ].second = (ui32)n;
         }
         return true;
      }

      // compute the projection according to the number of components needed
      core::Matrix<double> _makeProjection() const
      {
         ensure( _eigenVectors.sizex(), "error" );
         ensure( _nbVectors <= _eigenVectors.sizey(), "error" );

         ui32 size = _eigenVectors.sizey();
         core::Matrix<double> t( _nbVectors, size );
         for ( ui32 n = 0; n < _nbVectors; ++n )
            for ( ui32 nn = 0; nn < size; ++nn )
               t( n, nn ) = _eigenVectors( nn, _pairs[ n ].second );  // select the highest first
         return t;
      }

   private:
      Vector   _mean;               // the mean sample
      Vector   _eigenValues;        // the unsorted eigen values
      Matrix   _eigenVectors;       // the unsorted eigen vectors, arranged in columns
      Pairs    _pairs;              // the sorted eigen values
      ui32     _nbVectors;          // the number of eigen vectors to consider
      Matrix   _projection;         // sorted, 1 row = 1 eigen vector
   };
}
}

#endif
