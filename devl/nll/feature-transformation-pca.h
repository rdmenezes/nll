#ifndef NLL_FEATURE_TRANSFORMATION_PCA_H_
# define NLL_FEATURE_TRANSFORMATION_PCA_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a feature transformation algorithm usin PCA. It is suitable for very high
           dimentional dataset.

    The PCA is decorrelating the features, it is assumed the variance of the feature
    is the important information. Internally, it is selecting the highest (egigen vector, eigen value)
    which "rotates" the data so that they are not correlated anymore.

    If the features have not a normal distribution or the infomation is not in the variance,
    this preprocessing should not be used.

    @see Pca
    */
   template <class Point>
   class FeatureTransformationPca : public FeatureTransformation<Point>
   {
   public:
      typedef FeatureTransformation<Point>          Base;

   public:
      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   private:
      typedef typename Classifier<Point>::Database          PcaDatabase;
      typedef core::DatabaseInputAdapterRead<PcaDatabase>   Points;
      typedef PrincipalComponentAnalysis<Points>            Pca;

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
      template <class Database>
      bool compute( const Database& _dat, ui32 nbComponents )
      {
         if ( _pca )
            delete _pca;
         _pca = new Pca();
         Database learningDat = core::filterDatabase( _dat,
                                                      core::make_vector<nll::ui32>( Database::Sample::LEARNING,
                                                                                    Database::Sample::VALIDATION ),
                                                      Database::Sample::LEARNING );
         core::DatabaseInputAdapterRead<Database> databaseAdapter( learningDat );
         return _pca->compute( databaseAdapter, nbComponents );
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
            _pca = new Pca();
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

   private:
      // copy disabled
      FeatureTransformationPca& operator=( const FeatureTransformationPca& );
      FeatureTransformationPca( const FeatureTransformationPca& );

   protected:
      Pca*     _pca;
   };
}
}

#endif
