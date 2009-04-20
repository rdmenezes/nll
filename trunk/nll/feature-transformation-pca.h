#ifndef NLL_FEATURE_TRANSFORMATION_PCA_H_
# define NLL_FEATURE_TRANSFORMATION_PCA_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a feature transformation algorithm usin PCA.
    @see Pca
    */
   template <class Point>
   class FeatureTransformationPca : public FeatureTransformation<Point>
   {
   public:
      typedef FeatureTransformation<Point>          Base;
      typedef typename Classifier<Point>::Database  Database;

   public:
      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   private:
      typedef core::DatabaseInputAdapterRead<Database>   Points;
      typedef PrincipalComponentAnalysis<Points>         Pca;

   public:
      FeatureTransformationPca() : _pca( 0 )
      {
      }

      ~FeatureTransformationPca()
      {
         delete _pca;
      }

      /**
       @brief Compute the principal components of the database. Only the <code>LEARNING|VALIDATION</code> are
              used.
       @return true if successful
       */
      bool compute( const Database& _dat, ui32 nbComponents )
      {
         if ( _pca )
            delete _pca;
         _pca = new Pca( nbComponents );
         Database learningDat = core::filterDatabase( _dat,
                                                      core::make_vector<nll::ui32>( Database::Sample::LEARNING,
                                                                                    Database::Sample::VALIDATION ),
                                                      Database::Sample::LEARNING );
         core::DatabaseInputAdapterRead<Database> databaseAdapter( learningDat );
         return _pca->compute( databaseAdapter );
      }

      /**
       @brief Returns the projection found.
       */
      const core::Matrix<double>& getProjection() const
      {
         ensure( _pca, "no PCA computed" );
         return _pca->getProjection();
      }

      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const
      {
         assert( _pca );
         return _pca->process( p );
      }

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         if ( !_pca )
            _pca = new Pca( 1 );
         _pca->read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         ensure( _pca, "no PCA computed" );
         _pca->write( o );
      }

   protected:
      Pca*     _pca;
   };
}
}

#endif
