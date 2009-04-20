#ifndef NLL_FEATURE_SELECTION_H_
# define NLL_FEATURE_SELECTION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a base to factorize the work of a feature selection task

    Point needs to provide: size(), operator[], constructor(size)
    Only handle the transformation of the database according to the feature descriptor 
   */
   template <class Point>
   class FeatureSelectionBase : public FeatureTransformation<Point>
   {
   public:
      typedef FeatureTransformation<Point>   Base;
      typedef Classifier<Point>              TClassifier;
      typedef typename TClassifier::Database Database;

      // don't override these
      using Base::read;
      using Base::write;
      using Base::process;

      virtual ~FeatureSelectionBase(){}

      FeatureSelectionBase(){}

      FeatureSelectionBase( const core::Buffer1D<bool>& selectedFeatures ) : _selectedFeatures( selectedFeatures )
      {
         _nbSelectedFeatures = getNumberOfSelectedFeatures();
      }

   public:
      void read( std::istream& f )
      {
         this->_selectedFeatures.read( f );
      }

      void write( std::ostream& f ) const
      {
         this->_selectedFeatures.write( f );
      }

      /**
        @brief transform a Point with full feature to a new one with only selected features
        */
      Point process( const Point& point ) const
      {
         assert( point.size() == _selectedFeatures.size() );   // error : feature set doesn't match
         assert( _nbSelectedFeatures );                        // no selected feature
         Point newPoint( _nbSelectedFeatures );
         ui32 n = 0;
         ui32 i = 0;
         while ( n != _nbSelectedFeatures )
         {
            assert( n < point.size() );
            if ( _selectedFeatures[ i ] )
            {
               newPoint[ n ] = point[ i ];
               ++n;
            }
            ++i;
         }
         return newPoint;
      }

      /**
       @return the number of features that are currently selected
       */
      ui32 getNumberOfSelectedFeatures()
      {
         ui32 n = 0;
         for ( ui32 i = 0; i < _selectedFeatures.size(); ++i )
            n += ( _selectedFeatures[ i ] == true );
         return n;
      }

      /**
       @brief Returns the curently selected features
       */
      const core::Buffer1D<bool>& getSelectedFeatures() const
      {
         return _selectedFeatures;
      }

   protected:
      /**
       @brief Set the features to be selected. Must be called by the derived class.
       */
      void _setSelectedFeatures( core::Buffer1D<bool>& f )
      {
         _selectedFeatures = f;
         _nbSelectedFeatures = getNumberOfSelectedFeatures();
      }

   private:
      core::Buffer1D<bool> _selectedFeatures;
      ui32                 _nbSelectedFeatures;
   };

   /**
    @ingroup algorithm
    @brief Actual base class for feature selection using a wrapper approach
    */
   template <class Point>
   class FeatureSelectionWrapper : public FeatureSelectionBase<Point>
   {
   public:
      typedef FeatureSelectionBase<Point> Base;
      typedef typename Base::TClassifier  Classifier;
      typedef typename Base::Database     Database;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureSelectionWrapper(){}
      virtual ~FeatureSelectionWrapper(){}

      FeatureSelectionWrapper( const core::Buffer1D<bool>& selectedFeatures ) : Base( selectedFeatures )
      {}

      // parameters : parameters of the classifier
      const core::Buffer1D<bool>& compute( const Classifier* classifier, const core::Buffer1D<f64>& parameters, const Database& dat )
      {
         core::Buffer1D<bool> f = _compute( classifier, parameters, dat );
         Base::_setSelectedFeatures( f );
         return Base::getSelectedFeatures();
      }

   protected:
      // parameters : parameters of the classifier
      // compute the set of selected features
      virtual core::Buffer1D<bool> _compute( const Classifier* classifier, const core::Buffer1D<f64>& parameters, const Database& dat ) = 0;
   };

   /**
    @ingroup algorithm
    @brief Actual base class for feature selection using a filter approach
    */
   template <class Point>
   class FeatureSelectionFilter : public FeatureSelectionBase<Point>
   {
   public:
      typedef FeatureSelectionBase<Point> Base;
      typedef typename Base::TClassifier  Classifier;
      typedef typename Base::Database     Database;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureSelectionFilter(){}
      virtual ~FeatureSelectionFilter(){}

      FeatureSelectionFilter( const core::Buffer1D<bool>& selectedFeatures ) : Base( selectedFeatures )
      {}

      const core::Buffer1D<bool>& compute( const Database& dat )
      {
         core::Buffer1D<bool> f = _compute( dat );
         Base::_setSelectedFeatures( f );
         return Base::getSelectedFeatures();
      }

   protected:
      // compute the set of selected features
      virtual core::Buffer1D<bool> _compute( const Database& dat ) = 0;
   };
}
}

#endif
