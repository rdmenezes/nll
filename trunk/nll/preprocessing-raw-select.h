#ifndef NLL_PREPROCESSING_RAW_SELECT_H_
# define NLL_PREPROCESSING_RAW_SELECT_H_

#pragma warning( push )
#pragma warning( disable:4512 ) // assignment operator could not be generated

namespace nll
{
namespace preprocessing
{
   /**
    @ingroup preprocessing
    @brief select certain features of the input vector

    These classes [PreprocessingRaw*] handle the preprocessing and consider data as a 1D array

    Feature selection using different kind of algorithms (wrapper and filter class algorithms). Run the classification
    algorithm on the selected features to evaluate the features. If there is only 1 algorithm, features quality is not assessed.
    Select the features set according to the evaluation.

    Init the feature selection with the vector select. If select[ n ] = false, the feature n is destroyed.
    optimize : start searching form a 0 feature array, and select greedily the best feature until maxFeature or end.
    */
   template <class Point>
   class PreprocessingRawSelect : public Preprocessing<Point, Point>
   {
   public:
      typedef Preprocessing<Point, Point>       Base;
      typedef typename Base::InputDatabase      InputDatabase;
      typedef typename Base::OutputDatabase     OutputDatabase;
      typedef typename Base::OClassifier        OClassifier;

   public:
      /**
       @brief Defines all the types of handled algorithm
       @todo implement PEARSON, GENETIC_ALGORITHM, BACKWARD_SELECTION
       */
      enum AlgorithmClass
      {
         NONE,             /// no optimisation
         BEST_FIRST,       /// use Best first algorithm
         GENETIC_ALGORITHM,/// use genetic algorithm
         /*BACKWARD_SELECTION*/ /// use the backard selection algorithm
         PEARSON,          /// use pearson correlation algorithm
         RELIEFF           /// use relieff ranking algorithm
      };

      typedef core::Buffer1D<bool>           Vector;
      typedef core::Buffer1D<AlgorithmClass> Options;

   private:
      /**
       @brief Base class to define all the possible algorithms for feature selection.
       */
      class Algo
      {
      public:
         /**
          @brief Select the features.
          */
         virtual Vector compute() const = 0;

         virtual ~Algo(){}
      };

      /**
       @brief Best  first feature selection algorithm.
       */
      class AlgoBestFirst : public Algo
      {
      public:
         /**
          @brief Constructor.
          @param d the input database
          @param classifier the classifier to be tested with
          @param params the parameters of the classifier
          */
         AlgoBestFirst( const InputDatabase& d, const OClassifier* classifier, const typename OClassifier::ClassifierParameters& params, ui32 maxFeatures ) :
            _dat( d ), _classifier( classifier ), _params( params ), _maxFeatures( maxFeatures )
         {}

         virtual Vector compute() const 
         {
            algorithm::FeatureSelectionBestFirst<Point> fsbf( _maxFeatures );
            algorithm::FeatureSelectionWrapper<Point>* fs = &fsbf;
            return fs->compute( _classifier, _params, _dat );
         }

      private:
         const InputDatabase&                         _dat;
         const OClassifier*                           _classifier;
         typename OClassifier::ClassifierParameters   _params;
         ui32                                         _maxFeatures;
      };

      /**
       @brief Relieff feature selection algorithm.
       */
      class AlgoRelieff : public Algo
      {
      public:
         AlgoRelieff( const InputDatabase& d, ui32 maxFeatures ) : _dat( d ), _maxFeatures( maxFeatures )
         {}

         virtual Vector compute() const 
         {
            algorithm::FeatureSelectionFilterRelieff<Point> fsbf( _maxFeatures );
            algorithm::FeatureSelectionFilter<Point>* fs = &fsbf;
            return fs->compute( _dat );
         }

      private:
         const InputDatabase& _dat;
         ui32                 _maxFeatures;
      };

      /**
       @brief Pearson feature selection algorithm.
       */
      class AlgoPearson : public Algo
      {
      public:
         AlgoPearson( const InputDatabase& d, ui32 maxFeatures ) : _dat( d ), _maxFeatures( maxFeatures )
         {}

         virtual Vector compute() const 
         {
            algorithm::FeatureSelectionFilterPearson<Point> fsbf( _maxFeatures );
            algorithm::FeatureSelectionFilter<Point>* fs = &fsbf;
            return fs->compute( _dat );
         }

      private:
         const InputDatabase& _dat;
         ui32                 _maxFeatures;
      };

      /**
       @brief factory to create the correct defined algorithm. Pointer must be freed after use
       */
      class AlgoFactory
      {
      public:
         AlgoFactory( const InputDatabase& d, const OClassifier* classifier, const typename OClassifier::ClassifierParameters& params, ui32 maxFeatures ) :
            _dat( d ), _classifier( classifier ), _params( params ), _maxFeatures( maxFeatures )
         {}

         Algo* create( AlgorithmClass c )
         {
            switch ( c )
            {
            case BEST_FIRST:
               return new AlgoBestFirst( _dat, _classifier, _params, _maxFeatures );
            case RELIEFF:
               return new AlgoRelieff( _dat, _maxFeatures );
            case PEARSON:
               return new AlgoPearson( _dat, _maxFeatures );
            case NONE:
               return 0;
            default:
               assert( 0 );
            }
            return 0;
         }
      private:
         const InputDatabase&                         _dat;
         const OClassifier*                           _classifier;
         typename OClassifier::ClassifierParameters   _params;
         ui32                                         _maxFeatures;
      };

   public:
      /**
       @param maxFeatures set the number maximum number of features that can be selected.
       @param select the initial input parameter. if select[ n ] == true, the n-th feature of the input vector will be in the output vector
       */
      PreprocessingRawSelect( ui32 maxFeatures, const Vector& select, const Options& options ) : _maxFeatures( maxFeatures ), _select( select ), _options( options )
      {
         _nbFeatures = getNbFeatures();
      }

      virtual ~PreprocessingRawSelect(){}

      /**
       @brief process the input vector.
       
       @return a vector smaller or equal to the input. _select[ n ] == false, n-th feature is not selected.
       */
      virtual Point process( const Point& p ) const
      {
         assert( _nbFeatures );
         Point pp( _nbFeatures );
         ui32 i = 0;
         for ( ui32 n = 0; n < _nbFeatures; ++n )
            if ( _select[ i ] )
               pp[ i++ ] = p[ n ];
         return p;
      }

      virtual OutputDatabase optimize( const InputDatabase& dat, const OClassifier* _classifier, const typename OClassifier::ClassifierParameters& params )
      {
         AlgoFactory factory( dat, _classifier, params, _maxFeatures );

         InputDatabase ndat = processDatabase( dat );
         Vector bestFeatures;
         f64 fitness = INT_MIN;
         for ( ui32 n = 0; n < _options.size(); ++n )
         {
            Algo* algo = factory.create( _options[ n ] );
            if ( algo )
            {
               _select = algo->compute();
               _nbFeatures = getNbFeatures();
               delete algo; // todo std::auto
            }

            // evaluate the features, keep the best feature set
            if ( _options.size() > 1 )
            {
               // reprocess it because the feature selection has changed
               InputDatabase ndat2 = processDatabase( dat );
               f64 fitnessTest = _classifier->evaluate( params, ndat2 );
               if ( fitnessTest > fitness )
               {
                  fitness = fitnessTest;
                  bestFeatures = _select;
               }
            }
         }

         if ( _options.size() > 1 )
         {
            _select = bestFeatures;
            _nbFeatures = getNbFeatures();
         }

         // reprocess the database with the new parameters
         return processDatabase( dat );
      }

      /**
       @todo implement
       */
      virtual void write( std::ostream& o ) const
      {
         _select.write( o );
         // TODO _options.write( o );
      }

      /**
       @todo implement
       */
      virtual void read( std::istream& i )
      {
         _select.read( i );
         // TODO _options.read( i );
      }

   protected:
      ui32 getNbFeatures() const
      {
         ui32 nb = 0;
         for ( ui32 n = 0; n < _select.size(); ++n )
            nb += _select[ n ];
         return nb;
      }

   protected:
      ui32     _maxFeatures;
      ui32     _nbFeatures;
      Vector   _select;
      Options  _options;
   };
}
}

#pragma warning( pop )

#endif
