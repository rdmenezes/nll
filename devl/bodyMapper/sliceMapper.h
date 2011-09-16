#ifndef MVV_SLICE_MAPPER_H_
# define MVV_SLICE_MAPPER_H_

# include "def.h"
# include "dataset.h"
# include "datasetSliceMapper.h"

# pragma warning( push )
# pragma warning( disable:4251 ) // dll interface for STL

namespace mvv
{
namespace mapper
{
   /// store the parameters used for the simple preprocessing
   struct BODYMAPPER_API SliceMapperPreprocessingParameters
   {
      SliceMapperPreprocessingParameters()
      {
         preprocessSizeX = 66;
         preprocessSizeY = 44;

         lutMin = -100;
         lutMax = 250;

         lutMaskMin = -200;
         lutMaskMax = 50;

         verticalCroppingRatio = 0.06f;
         horizontalCroppingRatio = 0.03f;
      }

      void print( std::ostream& o ) const
      {
         o  << "slice mapper parameters=" << std::endl
            << " preprocessSizeX=" << preprocessSizeX << std::endl
            << " preprocessSizeY=" << preprocessSizeY << std::endl
            << " lutMin="     << lutMin << std::endl
            << " lutMax="     << lutMax << std::endl
            << " lutMaskMin=" << lutMaskMin << std::endl
            << " lutMaskMax=" << lutMaskMax << std::endl
            << " verticalCroppingRatio="     << verticalCroppingRatio << std::endl
            << " horizontalCroppingRatio="   << horizontalCroppingRatio << std::endl;
      }

      void write( const std::string& str ) const
      {
         std::ofstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         write( f );
      }

      void write( std::ostream& o ) const
      {
         nll::core::write<unsigned>( preprocessSizeX, o );
         nll::core::write<unsigned>( preprocessSizeY, o );
         nll::core::write<float>( lutMin, o );
         nll::core::write<float>( lutMax, o );
         nll::core::write<float>( lutMaskMin, o );
         nll::core::write<float>( lutMaskMax, o );
         nll::core::write<float>( verticalCroppingRatio, o );
         nll::core::write<float>( horizontalCroppingRatio, o );
      }

      void read( const std::string& str )
      {
         std::ifstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         read( f );
      }

      void read( std::istream& i )
      {
         nll::core::read( preprocessSizeX, i );
         nll::core::read( preprocessSizeY, i );
         nll::core::read( lutMin, i );
         nll::core::read( lutMax, i );
         nll::core::read( lutMaskMin, i );
         nll::core::read( lutMaskMax, i );
         nll::core::read( verticalCroppingRatio, i );
         nll::core::read( horizontalCroppingRatio, i );
      }

      unsigned preprocessSizeX;     // the number of pixels the slice will be resampled to
      unsigned preprocessSizeY;     // the number of pixels the slice will be resampled to
      float lutMin;                 // the LUT min value
      float lutMax;                 // the LUT max value
      float lutMaskMin;             // the LUT min value
      float lutMaskMax;             // the LUT max value
      float verticalCroppingRatio;  // the ratio of the pixel removed for the preprocessed slice before resampling
      float horizontalCroppingRatio;  // the ratio of the pixel removed for the preprocessed slice before resampling
   };

   /// store the parameters used for the classifier preprocessing
   struct BODYMAPPER_API SliceMapperPreprocessingClassifierParametersInput
   {
      SliceMapperPreprocessingClassifierParametersInput()
      {
         nbFinalFeatures = 512;
         skipSliceInterval = 8;
         minDistanceBetweenSliceOfInterest = 10;
      }

      void write( const std::string& str ) const
      {
         std::ofstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         write( f );
      }

      void write( std::ostream& o ) const
      {
         nll::core::write<unsigned>( nbFinalFeatures, o );
         nll::core::write<unsigned>( skipSliceInterval, o );
         nll::core::write<unsigned>( minDistanceBetweenSliceOfInterest, o );
      }

      void read( std::istream& i )
      {
         nll::core::read( nbFinalFeatures, i );
         nll::core::read( skipSliceInterval, i );
         nll::core::read( minDistanceBetweenSliceOfInterest, i );
      }

      void read( const std::string& str )
      {
         std::ifstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         read( f );
      }

      unsigned minDistanceBetweenSliceOfInterest;  // the distance in slices where it is not possible to have a NOTHING slice from a <anything landmark>
      unsigned nbFinalFeatures;        // number of features that are needed for each classifier
      unsigned skipSliceInterval;      // the non point of interest slices will be sampled every <skipSliceInterval>
   };  

   /**
    @brief Handle the basis slice preprocessing
    */
   class BODYMAPPER_API SliceBasicPreprocessing
   {
   public:
      typedef nll::core::Buffer1D<double>                       Point;
      typedef nll::core::ClassificationSample<Point, nll::ui32> Sample;
      typedef nll::core::Database<Sample>                       Database;
      typedef nll::core::Image<unsigned char>                   Image;
      typedef nll::core::Image<double>                          Imagef;
      typedef nll::imaging::LookUpTransformWindowingRGB         Lut;
      typedef LandmarkDataset::Volume                           Volume;

      SliceBasicPreprocessing( const SliceMapperPreprocessingParameters params = SliceMapperPreprocessingParameters() ) : _params( params ), _lut( params.lutMin, params.lutMax, 256, 1 ),
                                                                                                                                             _lutMask( params.lutMaskMin, params.lutMaskMax, 256, 1 )
      {
         _lut.createGreyscale();
         _lutMask.createGreyscale();
      }

      SliceBasicPreprocessing( const std::string& config ) : _lut( 0, 10, 256, 1 ), _lutMask( 0, 10, 256, 1 )
      {
         read( config );
      }

      // preprocess the slice (i.e. LUT transform the image, extract biggest CC, cropping, center, resample...)
      Image preprocessSlice( const Volume& volume, unsigned slice ) const;
      Imagef preprocessSlicef( const Volume& volume, unsigned slice ) const;
      
      // create a database with all the volumes already preprocessed and export it. A database is created for each landmark (except <0>)
      // It will contain all the slices and it is guaranteed the slices are in order, with debug string containing their index
      Database createPreprocessedDatabase( const SliceMapperDataset& datasets ) const;

      const SliceMapperPreprocessingParameters& getPreprocessingParameters() const
      {
         return _params;
      }

      void write( const std::string& str ) const
      {
         std::ofstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         write( f );
      }

      void write( std::ostream& o ) const
      {
         _params.write( o );
      }

      void read( std::istream& i )
      {
         _params.read( i );

         _lut = Lut( _params.lutMin, _params.lutMax, 256, 1 );
         _lutMask = Lut( _params.lutMaskMin, _params.lutMaskMax, 256, 1 );

         _lut.createGreyscale();
         _lutMask.createGreyscale();
      }

      void read( const std::string& str )
      {
         std::ifstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         read( f );
      }

   private:
      SliceMapperPreprocessingParameters     _params;
      Lut                                    _lut;
      Lut                                    _lutMask;
   };

   /**
    @parameters for the SVM classifiers
    */
   struct BODYMAPPER_API SliceMapperClassifierSvmParameters
   {
      SliceMapperClassifierSvmParameters()
      {
         gamma = 0.1;
         costMargin = 100;
      }

      void write( const std::string& str ) const
      {
         std::ofstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         write( f );
      }

      void write( std::ostream& o ) const
      {
         nll::core::write<double>( gamma, o );
         nll::core::write<double>( costMargin, o );
      }

      void read( std::istream& i )
      {
         nll::core::read( gamma, i );
         nll::core::read( costMargin, i );
      }

      void read( const std::string& str )
      {
         std::ifstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         read( f );
      }

      double gamma;        // SVM model paramter 1
      double costMargin;   // SVM model paramter 2
   };

   /**
    @brief Handle the classification of a slice
    */
   class BODYMAPPER_API SliceMapperClassifierSvm
   {
   public:
      typedef nll::core::Buffer1D<double>                       Point;
      typedef nll::algorithm::ClassifierSvm<Point>              Classifier;
      typedef nll::core::ClassificationSample<Point, nll::ui32> Sample;
      typedef nll::core::Database<Sample>                       Database;

   public:
      SliceMapperClassifierSvm(){}

      void learn( const SliceMapperClassifierSvmParameters& params, const std::vector<Database>& databases );

      ~SliceMapperClassifierSvm();

      /**
       @brief destroy and release the memory of the classifiers
       */
      void destroy();

      /**
       @brief test 
       */
      void test( const std::vector<Point>& points, std::vector<unsigned>& classIds_out, std::vector<double>& pbs_out ) const;

      void write( const std::string& str ) const
      {
         std::ofstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         write( f );
      }

      void write( std::ostream& o ) const
      {
         unsigned size = static_cast<unsigned>( _classifiers.size() );
         nll::core::write<unsigned>( size, o );
         for ( size_t n = 0; n < _classifiers.size(); ++n )
         {
            _classifiers[ n ]->write( o );
         }
      }

      void read( std::istream& i )
      {
         unsigned size = 0;
         nll::core::read<unsigned>( size, i );
         for ( size_t n = 0; n < _classifiers.size(); ++n )
         {
            Classifier* c = new Classifier( true, true );
            _classifiers.push_back( c );
            (*_classifiers.rbegin())->read( i );
         }
      }

      void read( const std::string& str )
      {
         std::ifstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         read( f );
      }

   private:
      // copy disabled
      SliceMapperClassifierSvm& operator=( const SliceMapperClassifierSvm& );
      SliceMapperClassifierSvm( const SliceMapperClassifierSvm& );

   private:
      std::vector< Classifier* >  _classifiers;
   };

   /**
    @brief This class is a helper class to handle the feature preprocessing and database generation used
           by a classifier
    */
   class BODYMAPPER_API SlicePreprocessingClassifierInput
   {
      typedef nll::core::Buffer1D<double>                                     Point;
      typedef nll::algorithm::HaarFeatures2d                                  HaarFeatures;
      typedef nll::algorithm::PrincipalComponentAnalysis<Point>               Pca;
      typedef SliceBasicPreprocessing::Imagef                                 Imagef;
      typedef SliceBasicPreprocessing::Database                               Database;
      typedef nll::algorithm::IntegralImage                                   IntegralImage;

   public:
      SlicePreprocessingClassifierInput( const SliceMapperPreprocessingClassifierParametersInput& params,
                                         const SliceMapperPreprocessingParameters& paramsPreprocessing ) : _params( params ), _paramsPreprocessing( paramsPreprocessing )
      {
      }

      SlicePreprocessingClassifierInput( const std::string& classifierFeaturePrerpocessingConfig,
                                         const std::string& slicePreprocessingConfig )
      {
         _params.read( classifierFeaturePrerpocessingConfig );
         _paramsPreprocessing.read( slicePreprocessingConfig );
      }

      /**
       @brief Preprocess the image, including haar feature extraction and feature projection
       */
      Point preprocess( const IntegralImage& preprocessedSlice, unsigned classifierId ) const;

      /**
       @brief Compute the features (Haar + dimension reduction)
       */
      void computeClassifierFeatures( const Database& preprocessedSliceDatabase );

      /**
       @brief Create the databases for the classifiers, one by landmark, except <0>
       */
      std::vector<Database> createClassifierInputDatabases( const Database& preprocessedSliceDatabase ) const;

      void write( const std::string& str ) const
      {
         std::ofstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         write( f );
      }

      void write( std::ostream& o ) const
      {
         _params.write( o );
         _paramsPreprocessing.write( o );
         nll::core::write< std::vector<HaarFeatures> >( _featuresByType, o );
         nll::core::write< std::vector<Pca> >( _featureReductionByType, o );
      }

      void read( std::istream& i )
      {
         _params.read( i );
         _paramsPreprocessing.read( i );
         nll::core::read( _featuresByType, i );
         nll::core::read( _featureReductionByType, i );
      }

      void read( const std::string& str )
      {
         std::ifstream f( str.c_str(), std::ios::binary );
         if ( !f.good() )
            throw std::runtime_error( "cannot write file:" + str );
         read( f );
      }

   private:
      void _createHaarFeatures( const Database& dat );
      std::vector<Point> _computeHaarFeatures( const IntegralImage& input );

      // sort the samples belonging to their respective classifier
      std::vector<Database> _sortAndSelectDatabaseByClassifier( const Database& preprocessedSliceDatabase ) const;

   private:
      SliceMapperPreprocessingClassifierParametersInput  _params;
      SliceMapperPreprocessingParameters                 _paramsPreprocessing;
      std::vector<HaarFeatures>                          _featuresByType;
      std::vector<Pca>                                   _featureReductionByType;
   };
}
}

#endif

# pragma warning( pop )