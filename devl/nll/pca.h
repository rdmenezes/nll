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
      PrincipalComponentAnalysis( ui32 nbVectors ) : _nbVectors( nbVectors )
      {
         assert( nbVectors );
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
       */
      const core::Matrix<double>& getEigenVectors() const
      {
         return _eigenVectors;
      }

      /**
       @brief Return the eigen values
       */
      const core::Matrix<double>& getEigenValues() const
      {
         return _eigenValues;
      }

      /**
       @brief Return the current projection
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
      void write( std::ostream& o )
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

      void write( const std::string& path )
      {
         std::ofstream f( path.c_str(), std::ios::binary );
         ensure( f.good(), "can't write PCA binaries" );

         write( f );
      }

      /**
       @brief Compute the PCA on the given set of points.
       */
      bool compute( const Points& points )
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
         bool res = core::svdcmp( p, eigenValues, _eigenVectors );

         // SVD failed
         if ( !res )
            return false;

         // sort the eigen values by decreasing order
         Pairs sort;
         for ( ui32 n = 0; n < eigenValues.size(); ++n )
            sort.push_back( Pair( eigenValues[ n ], n ) );
         std::sort( sort.rbegin(), sort.rend() );
         _pairs = sort;

         // set the transformation
         _projection = _makeProjection();
         return true;
      }

      /**
       @brief Project a point using the previously computed projection.
       */
      Point process( const Point& point ) const
      {
         ensure( _projection.size(), "Empty projection" );
         core::Matrix<double> p( static_cast<ui32>( point.size() ), 1 );
         for ( ui32 n = 0; n < p.size(); ++n )
            p[ n ] = point[ n ] - _mean[ n ];
         core::Matrix<double> r = core::mul( _projection, p );
         Point result( r.size() );
         for ( ui32 n = 0; n < r.size(); ++n )
            result[ n ] = r[ n ];
         return result;
      }

   private:
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
      core::Matrix<double>    _eigenVectors;
      core::Matrix<double>    _eigenValues;
      core::Matrix<double>    _projection;
      Pairs                   _pairs;
   };
}
}

#endif
