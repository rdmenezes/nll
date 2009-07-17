#ifndef NLL_FEATURE_SELECTION_BEST_FIRST_H_
# define NLL_FEATURE_SELECTION_BEST_FIRST_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Feature selection using a wrapper approach and a greedy algorithm. It will
           start with an empty set of feature. Each round, every feature not selected will
           be tested. The best feture will be added to the list of selected features and
           a new round will be started until the error is not lowered anymore by adding
           new features.
   
    only suitable for low dimensionality
    greedy search
    */
   template <class Point>
   class FeatureSelectionBestFirst : public FeatureSelectionWrapper<Point>
   {
   public:
      typedef FeatureSelectionWrapper<Point> Base;
      typedef typename Base::Database        Database;
      typedef typename Base::Classifier      Classifier;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureSelectionBestFirst( ui32 maxFeatures = LONG_MAX ) : _maxFeatures( maxFeatures )
      {}

      FeatureSelectionBestFirst( const core::Buffer1D<bool>& selectedFeatures ) : Base( selectedFeatures )
      {}

   protected:
      virtual core::Buffer1D<bool> _compute( const Classifier* classifier, const core::Buffer1D<f64>& parameters, const Database& dat )
      {
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "BestFirst search started" );
         if ( ! dat.size() )
            return core::Buffer1D<bool>();
         const ui32 nbFeatures = dat[ 0 ].input.size();
         core::Buffer1D<bool> selectedFeatures( nbFeatures );
         core::Buffer1D<bool> bestSelectedFeatures;
         double bestSelectedFeaturesRate = LONG_MIN;

         ui32 nbSelectedFeatures = 0;
         while ( nbSelectedFeatures < _maxFeatures && nbSelectedFeatures < nbFeatures )
         {
            double best = LONG_MIN;
            ui32 index = 0;
            for ( ui32 n = 0; n < nbFeatures; ++n )
            {
               if ( !selectedFeatures[ n ] )
               {
                  core::Buffer1D<bool> testFeatures;
                  testFeatures.clone( selectedFeatures );
                  testFeatures[ n ] = true;
                  FeatureSelectionBase<Point> utility( testFeatures );
                  Database dnew = utility.process( dat );

                  double res = classifier->evaluate( parameters, dnew );
                  if ( res > best )
                  {
                     best = res;
                     index = n;
                  }

                  // log
                  std::stringstream s;
                  s << "test=";
                  testFeatures.print( s );
                  s << " rate=" << res;
                  core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, s.str() );
               }
            }

            // keep track of the best selected features;
            selectedFeatures[ index ] = true;
            ++nbSelectedFeatures;
            if ( best > bestSelectedFeaturesRate )
            {
               bestSelectedFeaturesRate = best;
               bestSelectedFeatures.clone( selectedFeatures );
            }
         }

         std::stringstream s;
         s << "Bestfirst solution=";
         bestSelectedFeatures.print( s );
         s << " rate=" << bestSelectedFeaturesRate;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, s.str() );
         return bestSelectedFeatures;
      }

   private:
      ui32 _maxFeatures;
   };
}
}

#endif
