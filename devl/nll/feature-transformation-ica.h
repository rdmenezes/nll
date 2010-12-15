#ifndef NLL_FEATURE_TRANSFORMATION_ICA_H_
# define NLL_FEATURE_TRANSFORMATION_ICA_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Independent Component Analysis, implementaing the FastICA algorithm

    Independent component analysis (ICA) is a statisticalmethod for transforming an observed multidimensional
    random vector into components that are statistically as independent from each other as possible. For example,
    there are several emitters and recepters, the recepters receive a mixed signal of recepters
    and ICA is trying to unmix this mixed signal.
    
    It is assumed that the emitters are not following a gaussian distribution (at maximum one can)

    it is implementing this paper: http://www.cs.helsinki.fi/u/ahyvarin/papers/TNN99new.pdf
    @see Ica
    */
   template <class Point>
   class FeatureTransformationIca : public FeatureTransformation<Point>
   {
      typedef IndependentComponentAnalysis<> Ica;

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
      FeatureTransformationIca( double step = 0.1 ) : _step( step )
      {}

      /**
       @brief Compute the principal components of the database. Only the <code>LEARNING|VALIDATION</code> are
              used.
       @return true if successful
       */
      template <class Database>
      bool compute( const Database& _dat, ui32 nbComponents )
      {
         Database learningDat = core::filterDatabase( _dat,
                                                      core::make_vector<nll::ui32>( Database::Sample::LEARNING,
                                                                                    Database::Sample::VALIDATION ),
                                                      Database::Sample::LEARNING );
         core::DatabaseInputAdapterRead<Database> databaseAdapter( learningDat );
         _ica.compute( databaseAdapter, nbComponents, _step );
         return true;
      }

      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const
      {
         return _ica.transform( p );
      }
   
      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         _ica.read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         _ica.write( o );
      }

   protected:
      Ica         _ica;
      double      _step;
   };
}
}

#endif
