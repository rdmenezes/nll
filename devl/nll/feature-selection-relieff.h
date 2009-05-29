#ifndef NLL_FEATURE_SELECTION_RELIEFF_H_
# define NLL_FEATURE_SELECTION_RELIEFF_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Feature selection using the Relieff algorithm. Suitable for multi-class, high number of features and fast.
           It belongs to the filter category. It takes into account the relationships
           between variables.
    */
   template <class Point>
   class FeatureSelectionFilterRelieff : public FeatureSelectionFilter<Point>
   {
   public:
      typedef FeatureSelectionFilter<Point> Base;
      typedef typename Base::Database       Database;
      typedef typename Base::Classifier     Classifier;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureSelectionFilterRelieff( ui32 nbOfFeatures, ui32 steps = 1000, ui32 k = 10 ) : _nbOfFeatures( nbOfFeatures ), _steps( steps ), _k( k )
      {}

   protected:
      core::Buffer1D<bool> _compute( const Database& dat )
      {
         typedef std::pair<double, ui32> Pair;
         typedef Relieff<Point>     Relieff;
         Relieff relief;

         typename Relieff::FeatureRank rank = relief.process( dat, _steps, _k );
         std::vector<Pair> vecs( rank.size() );
         for ( ui32 n = 0; n < vecs.size(); ++n )
            vecs[ n ] = std::make_pair( rank[ n ], n );
         std::sort( vecs.rbegin(), vecs.rend() );

         // score are sorted, the highest ones are selected
         core::Buffer1D<bool> selection( static_cast<ui32>( vecs.size() ) ); // by default no features are selected
         for ( ui32 n = 0; n < _nbOfFeatures; ++n )
            selection[ vecs[ n ].second ] = true;
         return selection;
      }

   private:
      ui32     _nbOfFeatures;
      ui32     _steps;
      ui32     _k;
   };
}
}

#endif
