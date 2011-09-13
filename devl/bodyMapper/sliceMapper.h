#ifndef MVV_SLICE_MAPPER_H_
# define MVV_SLICE_MAPPER_H_

# include "def.h"
# include "dataset.h"

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

         minDistanceBetweenRoiInMM = 20;

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
            << " minDistanceBetweenRoiInMM=" << minDistanceBetweenRoiInMM << std::endl
            << " verticalCroppingRatio="     << verticalCroppingRatio << std::endl
            << " horizontalCroppingRatio="   << horizontalCroppingRatio << std::endl;
      }

      void write( std::ostream& o ) const
      {
         nll::core::write<unsigned>( preprocessSizeX, o );
         nll::core::write<unsigned>( preprocessSizeY, o );
         nll::core::write<float>( lutMin, o );
         nll::core::write<float>( lutMax, o );
         nll::core::write<float>( lutMaskMin, o );
         nll::core::write<float>( lutMaskMax, o );
         nll::core::write<float>( minDistanceBetweenRoiInMM, o );
         nll::core::write<float>( verticalCroppingRatio, o );
         nll::core::write<float>( horizontalCroppingRatio, o );
      }

      void read( std::istream& i )
      {
         nll::core::read( preprocessSizeX, i );
         nll::core::read( preprocessSizeY, i );
         nll::core::read( lutMin, i );
         nll::core::read( lutMax, i );
         nll::core::read( lutMaskMin, i );
         nll::core::read( lutMaskMax, i );
         nll::core::read( minDistanceBetweenRoiInMM, i );
         nll::core::read( verticalCroppingRatio, i );
         nll::core::read( horizontalCroppingRatio, i );
      }

      unsigned preprocessSizeX;     // the number of pixels the slice will be resampled to
      unsigned preprocessSizeY;     // the number of pixels the slice will be resampled to
      float lutMin;                 // the LUT min value
      float lutMax;                 // the LUT max value
      float lutMaskMin;             // the LUT min value
      float lutMaskMax;             // the LUT max value
      float minDistanceBetweenRoiInMM; // the distance where it is not possible to have a NOTHING slice from a <anything landmark>
      float verticalCroppingRatio;  // the ratio of the pixel removed for the preprocessed slice before resampling
      float horizontalCroppingRatio;  // the ratio of the pixel removed for the preprocessed slice before resampling
   };

   /// store the parameters used for the classifier preprocessing
   struct BODYMAPPER_API SliceMapperPreprocessingClassifierParametersInput
   {
      SliceMapperPreprocessingClassifierParametersInput()
      {
         nbFinalFeatures = 512;
      }

      unsigned nbFinalFeatures;
   };  

   /**
    @brief Handle the basis slice preprocessing
    */
   class BODYMAPPER_API SliceBasicPreprocessing
   {
   public:
      typedef nll::core::Buffer1D<double>                      Point;
      typedef nll::core::ClassificationSample<Point, unsigned> Sample;
      typedef nll::core::Database<Sample>                      Database;
      typedef nll::core::Image<unsigned char>                  Image;
      typedef nll::core::Image<double>                         Imagef;
      typedef nll::imaging::LookUpTransformWindowingRGB        Lut;
      typedef LandmarkDataset::Volume                          Volume;

      SliceBasicPreprocessing( const SliceMapperPreprocessingParameters params = SliceMapperPreprocessingParameters() ) : _params( params ), _lut( params.lutMin, params.lutMax, 256, 1 ),
                                                                                                       _lutMask( params.lutMaskMin, params.lutMaskMax, 256, 1 )
      {
         _lut.createGreyscale();
         _lutMask.createGreyscale();
      }

      // preprocess the slice (i.e. LUT transform the image, extract biggest CC, cropping, center, resample...)
      Image preprocessSlice( const Volume& volume, unsigned slice ) const;
      Imagef preprocessSlicef( const Volume& volume, unsigned slice ) const;
      
      // create a database with all the volumes already preprocessed and export it. A database is created for each landmark (except <0>)
      Database createPreprocessedDatabase( const LandmarkDataset& datasets ) const;

      const SliceMapperPreprocessingParameters& getPreprocessingParameters() const
      {
         return _params;
      }

   private:
      SliceMapperPreprocessingParameters     _params;
      Lut                                    _lut;
      Lut                                    _lutMask;
   };

   /**
    @brief This class is a helper class to handle the feature preprocessing and database generation used
           by a classifier
    */
   class BODYMAPPER_API SliceMapperPreprocessingClassifierInput
   {
      typedef nll::algorithm::HaarFeatures2d                                  HaarFeatures;
      typedef nll::algorithm::PrincipalComponentAnalysis<SliceBasicPreprocessing::Point>  Pca;
      typedef SliceBasicPreprocessing::Imagef                                 Imagef;
      typedef nll::core::Buffer1D<double>                                     Point;
      typedef SliceBasicPreprocessing::Database                               Database;
      typedef nll::algorithm::IntegralImage                                   IntegralImage;

   public:
      SliceMapperPreprocessingClassifierInput( const SliceMapperPreprocessingClassifierParametersInput& params,
                                               const SliceMapperPreprocessingParameters& paramsPreprocessing ) : _params( params ), _paramsPreprocessing( paramsPreprocessing )
      {
      }

      /**
       @brief Preprocess the image, including haar feature extraction and feature projection
       */
      Point preprocess( const Imagef& preprocessedSlice ) const;

      /**
       @brief Compute the features (Haar + dimension reduction)
       */
      void computeClassifierFeatures( const Database& preprocessedSliceDatabase );

      /**
       @brief Create the databases for the classifiers, one by landmark, except <0>
       */
      std::vector<Database> createClassifierInputDatabases( const Database& preprocessedSliceDatabase ) const;

   private:
      void _createHaarFeatures( const Database& dat );
      std::vector<Point> _computeHaarFeatures( const IntegralImage& input );

      // sort the samples belonging to their respective classifier
      std::vector<Database> _sortDatabaseByClassifier( const Database& preprocessedSliceDatabase );

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