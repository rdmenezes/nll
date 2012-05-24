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

#ifndef NLL_PCA_H_
# define NLL_PCA_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Do the principal component analysis of a collection of points. Input points are normalized (mean = 0)
           beforehand.

    To reconstruct the point from a transformed point and a transformation do:
    <code>add( mul( transpose( pca.getTransformation() ), pointTransformedbyPca ), pca.getMean() )</code>

    <code>Points</code> must provide:
    - ui32 size() const;
    - const Point& operator[]( ui32 n ) const;
    - typedef <code>value_type</code>

    <code>value_type</code> must provide:
    - construcor( size );
    - operator[]( ui32 );
    - ui32 size() const;
    - typedef <code>value_type</code>
    */
   template <class Points>
   class PrincipalComponentAnalysis
   {
   public:
      typedef typename Points::value_type    Point;

   private:
      typedef std::pair<double, ui32>        Pair;
      typedef std::vector<Pair>              Pairs;

   public:
      PrincipalComponentAnalysis() : _nbVectors( 0 )
      {
      }

      template <class PPoints>
      PrincipalComponentAnalysis( const PrincipalComponentAnalysis<PPoints>& pca )
      {
         _nbVectors = pca.getNbVectors();
         _mean.clone( pca.getMean() );
         _eigenVectors.clone( pca.getEigenVectors() );
         _eigenValues.clone( pca.getEigenValues() );
         _projection.clone( pca.getProjection() );
         _pairs = pca.getPairs();
      }

      /**
       @param build the PCA directly from a mean & projection matrix
       @param the projection is defined in row, each row a projection, with projection.sizex() == mean.size()

       It is assumed the most important projection is ordered from begining to end
       */
      template <class Vector, class Matrix>
      PrincipalComponentAnalysis( const Vector& mean, const Matrix& projection )
      {
         _nbVectors = projection.sizey();
         _mean.clone( mean );
         _eigenVectors.import( projection );
         core::transpose( _eigenVectors );   // they are stored in column due to the SVD computation!
         _projection.clone( projection );

         // create fake values for eigen values and pairs...
         _eigenValues = core::Matrix<double>( projection.sizey(), 1 );
         for ( ui32 n = 0; n < projection.sizey(); ++n )
         {
            _eigenValues( n, 0 ) = projection.sizey() - n;
            _pairs.push_back( Pair( _eigenValues( n, 0 ), n ) );
         }
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
      const core::Matrix<double>& getMean() const
      {
         return _mean;
      }

      /**
       @brief Return the eigen vectors. They are arranged by column (each column is an eigen vector)

       @note the eigen vectors are not sorted by importance, use <getPairs()> to get the order
       */
      const core::Matrix<double>& getEigenVectors() const
      {
         return _eigenVectors;
      }

      /**
       @brief Return the eigen values

       @note the eigen vectors are not sorted by importance, use <getPairs()> to get the order
       */
      const core::Matrix<double>& getEigenValues() const
      {
         return _eigenValues;
      }

      /**
       @brief Return the current projection
       @note sorted from highest to lowest eigen value

       Each row is an eigen vector
       */
      const core::Matrix<double>& getProjection() const
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

      /**
       @brief Computes PCA, decide the number of eigen vectors retain by the retained variance
       */
      bool computeByVarianceRetained( const Points& points, double varianceToRetain )
      {
         if ( !points.size() )
            return false;

         const bool success = _computeEigenVectors( points );
         if ( !success )
            return false;

         // now compute the number of eigen vectors to retain the specified variance
         double eivSum = 0;
         for ( ui32 n = 0; n < _eigenValues.size(); ++n )
         {
            if ( _eigenValues[ n ] > 0 )
               eivSum += fabs( _eigenValues[ n ] );
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

         std::stringstream ss;
         ss << "PCA, nb of components=" << _nbVectors;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );

         // set the transformation
         _projection = _makeProjection();

         std::stringstream sss;
         sss << " PCA projection=";
         _projection.print( sss );
         sss << " PCA mean=";
         _mean.print( sss );

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sss.str() );
         return true;
      }

      /**
       @brief Compute the PCA on the given set of points.
       */
      bool compute( const Points& points, ui32 nbVectors )
      {
         _nbVectors = nbVectors;
         if ( !points.size() )
            return false;
         const ui32 size = static_cast<ui32>( points[ 0 ].size() );

         const bool success = _computeEigenVectors( points );
         if ( !success )
            return false;

         std::stringstream ss;
         ss << "PCA, nb of components=" << _nbVectors;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );

         // set the transformation
         _projection = _makeProjection();

         std::stringstream sss;
         sss << " PCA projection=";
         _projection.print( sss );
         sss << " PCA mean=";
         _mean.print( sss );

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sss.str() );
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

   private:
      bool _computeEigenVectors( const Points& points )
      {
         if ( !points.size() )
            return false;
         ui32 size = static_cast<ui32>( points[ 0 ].size() );

         core::Matrix<double> p( static_cast<ui32>( points.size() ), size );
         for ( ui32 n = 0; n < points.size(); ++n )
         {
            assert( points[ n ].size() == size );
            for ( ui32 nn = 0; nn < size; ++nn )
               p( n, nn ) = points[ n ][ nn ];
         }

         // compute the mean
         _mean = core::Matrix<double>( core::meanRow( p, 0, p.sizey() - 1 ) );

         core::Matrix<double> cov = core::covariance( p, 0, p.sizey() - 1 );

         core::Buffer1D<double> eigenValues;
         bool res = core::svdcmp( cov, eigenValues, _eigenVectors );
         _eigenValues = core::Matrix<double>( eigenValues, eigenValues.size(), 1 );

         // SVD failed
         if ( !res )
            return false;

         // sort the eigen values by decreasing order
         Pairs sort;
         for ( ui32 n = 0; n < eigenValues.size(); ++n )
            sort.push_back( Pair( eigenValues[ n ], n ) );
         std::sort( sort.rbegin(), sort.rend() );
         _pairs = sort;
         return true;
      }

      core::Matrix<double> _makeProjection() const
      {
         ensure( _eigenVectors.sizex(), "error" );
         ensure( _nbVectors <= _eigenVectors.sizey(), "error" );

         ui32 size = _eigenVectors.sizex();
         core::Matrix<double> t( _nbVectors, size );
         for ( ui32 n = 0; n < _nbVectors; ++n )
            for ( ui32 nn = 0; nn < size; ++nn )
               t( n, nn ) = _eigenVectors( nn, _pairs[ n ].second );  // select the highest first
         return t;
      }

   private:
      ui32                    _nbVectors;
      core::Matrix<double>    _mean;
      core::Matrix<double>    _eigenVectors; // Not sorted! use <_pairs>
      core::Matrix<double>    _eigenValues;  // Not sorted! use <_pairs> // 1 eigen vector = 1 column
      core::Matrix<double>    _projection;   // 1 eigen vector = 1 row
      Pairs                   _pairs;
   };
}
}

#endif
