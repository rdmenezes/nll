#ifndef NLL_PREPROCESSING_IMAGE_GABOR_H_
# define NLL_PREPROCESSING_IMAGE_GABOR_H_

namespace nll
{
namespace preprocessing
{
   /**
    @ingroup preprocessing
    @brief Take a gabor filters bank, optimize the bank to find the best set of filters
           convolve each filter to the input image, concatenate buffer's image into one
    */
   template <class Type, class Mapper, class Output>
   class PreprocessingImageGabor : public Preprocessing<core::Image<Type, Mapper>, core::Image<Output, Mapper> >
   {
   public:
      typedef core::Image<Type, Mapper>                           Point;
      typedef algorithm::GaborFilters<Type, Mapper, Output>       GaborFiltersBank;

      typedef Preprocessing<core::Image<Type, Mapper>, core::Image<Output, Mapper> >       Base;
      typedef typename Base::InputDatabase      InputDatabase;
      typedef typename Base::OutputDatabase     OutputDatabase;
      typedef typename Base::OClassifier        OClassifier;
      typedef typename Base::OutputType         OutputType;

   public:
      /**
       @param nbMaxFilters the maximal number of filters to be used to process an image.
       @param gaborFiltersBank the initial gabor filter bank from which the best one will be used to filter the input
       */
      PreprocessingImageGabor( ui32 nbMaxFilters, const GaborFiltersBank& gaborFiltersBank ) : _maxFilters( nbMaxFilters ), _initialGaborBank( gaborFiltersBank ){}

      /**
       @param nbMaxFilters the maximal number of filters to be used to process an image.
       @param gaborFiltersBank the initial gabor filter bank from which the best one will be used to filter the input
       @param optimizedBank the initial set of gabor to filter an input image
       */
      PreprocessingImageGabor( ui32 nbMaxFilters, const GaborFiltersBank& gaborFiltersBank, const GaborFiltersBank& optimizedBank ) : _maxFilters( nbMaxFilters ), _initialGaborBank( gaborFiltersBank ), _optimizedFilters( optimizedBank ){}

      virtual ~PreprocessingImageGabor(){}

      virtual OutputType process( const Point& p ) const
      {
         return _optimizedFilters.convolve( p );
      }

      /**
       @brief Optimize the filters using the _initialGaborBank as a pool of filters

       Greedily select the best filter. If a new round to select a filter doesn't improve the
       accuracy of the classifier, stop the selection. If the maximum number of filters is reached,
       stop also the optimization.
       */
      virtual OutputDatabase optimize( const InputDatabase& dat, const OClassifier* classifier, const typename OClassifier::ClassifierParameters& params )
      {
         GaborFiltersBank curFilters;
         double maxVal = INT_MIN;
         _optimizedFilters.clear();
         for ( ui32 nn = 0; nn <= _maxFilters && nn < _initialGaborBank.size(); ++nn )
         {
            // we are sure (nn) first filters are already tested so skip them
            for ( ui32 n = nn; n < _initialGaborBank.size(); ++n )
            {
               // test if the filter could be in the filters already tested
               bool isInList = false;
               for ( ui32 nnn = 0; nnn < _optimizedFilters.size(); ++nnn )
                  if ( _optimizedFilters[ nnn ] == _initialGaborBank[ n ] )
                  {
                     isInList = true;
                     break;
                  }
               if ( isInList )
                  continue;

               // process the bank with the current filters and evaluate the filters
               _optimizedFilters = curFilters;
               _optimizedFilters.addFilter( _initialGaborBank[ n ] );
               OutputDatabase output = processDatabase( dat );
               double outputVal = classifier->evaluate( params, output );
               if ( outputVal > maxVal )
               {
                  maxVal = outputVal;
                  curFilters = _optimizedFilters;
               }
            }

            // more filters doesn't improve the results so stop looking for new filters
            if ( nn < curFilters.size() )
               break;
         }
         std::cout << "optimization=" << _optimizedFilters.size() << " filters" << std::endl;
         return processDatabase( dat );
      }

      virtual void write( std::ostream& o ) const
      {
         _initialGaborBank.write( o );
         _optimizedFilters.write( o );
      }

      virtual void read( std::istream& i )
      {
         _initialGaborBank.read( i );
         _optimizedFilters.read( i );
      }

   private:
      GaborFiltersBank  _initialGaborBank;
      GaborFiltersBank  _optimizedFilters;
      ui32              _maxFilters;
   };
}
}

#endif
