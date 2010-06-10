#ifndef NLL_FEATURE_SELECTION_PEARSON_H_
# define NLL_FEATURE_SELECTION_PEARSON_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Compute the pearson coefficient for each feature and each class. This feature selection algorithm belongs
           to the filter class, 1 feature at a time is tested. Can be used on datasets with a lots of features.

           It is known that 2 features assessed independantly could be useless, but together can achieve perfect
           discrimination (for example xor problem with x and y axis). This algorithm can't see this. Also it will select
           redundand features (each feature gives discriminativ information, but the same info). But the algorithm is fastest
           and can still achieves very good results in some case.

    @todo evaluate the method for multiclass database.
    */
   template <class Point>
   class FeatureSelectionFilterPearson : public FeatureSelectionFilter<Point>
   {
   public:
      typedef FeatureSelectionFilter<Point> Base;
      typedef typename Base::Database       Database;
      typedef Classifier<Point>             ClassifierType;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureSelectionFilterPearson( ui32 nbFeatures ) : _nbFeatures( nbFeatures )
      {}

   protected:
      /**
       @brief compute the selected features : compute the max for each class of the pearson coefficient,
              return the set of features with the highest number of features.
       */
      virtual core::Buffer1D<bool> _compute( const Database& dat )
      {
         assert( dat.size() );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "FeatureSelectionFilterPearson search started" );
         ui32 nbFeatures = dat[ 0 ].input.size();
         ui32 nbclass = core::getNumberOfClass( dat );

         std::vector<std::pair<f64, ui32> > score( nbFeatures );
         for ( ui32 n = 0; n < nbFeatures; ++n )
         {
            core::Buffer1D<f64> f( dat.size() );
            for ( ui32 nn = 0; nn < dat.size(); ++nn )
               f[ nn ] = dat[ nn ].input[ n ];
            core::Buffer1D<f64> c( dat.size() );

            f64 max = -1;
            for ( ui32 nn = 0; nn < nbclass; ++nn )
            {
               for ( ui32 i = 0; i < dat.size(); ++i )
                  c[ i ] = ( dat[ i ].output == nn );
               f64 pearson = nll::core::absolute( core::correlation( core::Matrix<f64>( f ), core::Matrix<f64>( c ) ) );
               // TODO : for a multiclass database is that correct (or add the correlation)?
               // Can't find any reference on this. Needs to be tested.
               max = std::max( max, pearson );
            }
            score[ n ] = std::make_pair<f64, ui32>( max, n );
         }

         for ( ui32 n = 0; n < nbFeatures; ++n )
         {
            std::stringstream log;
            log << "Feature[" << n << "]=" << score[ n ].first;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, log.str() );
         }

         core::Buffer1D<bool> result( nbFeatures );
         std::sort( score.rbegin(), score.rend() );
         for ( ui32 n = 0; n < _nbFeatures; ++n )
            result[ score[ n ].second ] = true;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "FeatureSelectionFilterPearson search ended" );
         return result;
      }

   protected:
      ui32 _nbFeatures;
   };
}
}

#endif
