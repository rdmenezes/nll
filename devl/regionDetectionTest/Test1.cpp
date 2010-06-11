#include <tester/register.h>
#include "globals.h"
#include <regionDetection/features.h>
#include <regionDetection/read-result.h>
#include <regionDetection/test.h>

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;
using namespace nll::detect;

ui8 colors[ 5 ][ 3 ] = 
{
   { 255, 255, 255},
   { 0, 0, 255 },
   { 255, 0, 0 },
   { 0, 255, 0 },
   { 0, 255, 255 }
};

ui8 colors_src[ 5 ][ 3 ] = 
{
   { 255, 255, 255},
   { 0, 0, 255 },
   { 255, 0, 0 },
   { 0, 255, 0 },
   { 0, 255, 255 }
};

void setColorIntensity( ui32 index, double val )
{
   colors[ index ][ 0 ] = (ui8)( colors_src[ index ][ 0 ] * val );
   colors[ index ][ 1 ] = (ui8)( colors_src[ index ][ 1 ] * val );
   colors[ index ][ 2 ] = (ui8)( colors_src[ index ][ 2 ] * val );
}

struct TestRegion
{
   typedef Buffer1D<double>                  Point;
   typedef ClassifierSvm<Point>::Database    Database;
   
      /*
   void learnMlp()
   {
      srand( (unsigned)time(0) );
      typedef ClassifierMlp<Point>  Classifier;

      Database selectedHaarDatabaseNormalized;
      selectedHaarDatabaseNormalized.read( HAAR_SELECTION_DATABASE );

      Classifier classifier;
      classifier.learn( selectedHaarDatabaseNormalized, make_buffer1D<double>( 15, 1, 30 ) );
      classifier.test( selectedHaarDatabaseNormalized );

      testResultVolumeDatabase( &classifier );
   }*/



   // create a XZ MPR for every volume
   void createPreview()
   {
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      for ( ui32 n = 0; n < results.size(); ++n )
      {
         std::cout << "generate preview id=" << results[ n ].id << std::endl;

         Volume volume1;
         bool loaded = loadSimpleFlatFile( DATA_PATH "case" + val2str( results[ n ].id ) + ".mf2", volume1 );
         TESTER_ASSERT( loaded );

         Image<ui8> xz = extractXZ( volume1 );
         extend( xz, 3 );
         writeBmp( xz, std::string( PREVIEW_CASE ) + val2str( results[ n ].id ) + ".bmp" );
      }
   }

   void createDatasets()
   {
      typedef ClassifierMlp<Point>  Classifier;

      //RegionResult::generateSourceDatabase( CASES_DESC, DATABASE_SOURCE );
      //RegionResult::generateFeatureDatabase();


      std::cout << "haar selection..." << std::endl;
      Database haarDatabaseNormalized;
      haarDatabaseNormalized.read( NORMALIZED_HAAR );
      FeatureSelectionFilterPearson<Point> pearson( FEATURE_SELECTION_SIZE );
      pearson.compute( haarDatabaseNormalized );
      Database features = pearson.transform( haarDatabaseNormalized );

      pearson.write( HAAR_SELECTION );
      features.write( HAAR_SELECTION_DATABASE );
   }

   // create a database for each volume, containing all raw MPR
   void createVolumeDatabase()
   {
      typedef Buffer1D<ui8>         Point;
      typedef ClassifierMlp<Point>  Classifier;
      typedef Classifier::Database  Database;

      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      for ( ui32 n = 0; n < results.size(); ++n )
      {
         std::cout << "processing case:" << n << std::endl;
         Database dat;

         Volume volume;
         bool loaded = loadSimpleFlatFile( DATA_PATH "case" + val2str( results[ n ].id ) + ".mf2", volume );
         TESTER_ASSERT( loaded );

         for ( ui32 nn = 0; nn < volume.size()[ 2 ]; ++nn )
         {
            core::vector3f center = volume.indexToPosition( core::vector3f( volume.size()[ 0 ] / 2.0f,
                                                                            volume.size()[ 1 ] / 2.0f,
                                                                            static_cast<f32>( nn ) ) );
            core::Image<ui8> mpr_xy = extractSlice( volume, center[ 2 ] );

            ui32 classid = 0;
            if ( results[ n ].neckStart == nn )
               classid = 1;
            if ( results[ n ].heartStart == nn )
               classid = 2;
            if ( results[ n ].lungStart == nn )
               classid = 3;
            if ( results[ n ].skullStart == nn )
               classid = 4;
            dat.add( Database::Sample( mpr_xy, classid, Database::Sample::LEARNING ) );
         }

         dat.write( DATABASE_FULL_CASE( results[ n ].id ) );
      }
   }

   void learnSvm()
   {
      // prepare the bins
      const ui32 nbBins = 10;
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<ui32> bins( results.size() );
      for ( ui32 n = 0; n < results.size(); ++n )
      {
         bins[ n ] = n / ( (ui32)results.size() / nbBins );
      }
      core::randomize( bins );
      for ( ui32 n = 0; n < results.size(); ++n )
      {
         std::cout << bins[ n ] << " ";
      }


      for ( ui32 n = 0; n < nbBins; ++n )
      {
         std::cout << "test bin:" << n << std::endl;

         // run learning and test on the bins
         typedef ClassifierSvm<Point>  Classifier;

         Classifier classifier( 1, true );
         TestVolume test( &classifier, HAAR_FEATURES, PREPROCESSING_HAAR, HAAR_SELECTION );
         Database selectedHaarDatabaseNormalized = createLearningDatabase( bins, n, test );

         classifier.learn( selectedHaarDatabaseNormalized, make_buffer1D<double>( 10, 1 ) );
         classifier.test( selectedHaarDatabaseNormalized );

         testResultVolumeDatabase( test, bins, n );
      }
   }

   // select all slice if it is exactly on a ROI, or at least DATABASE_MIN_INTERVAL_ROI from one
   bool useSlice( const RegionResult::Result& result, ui32 currentSlice )
   {
      return result.lungStart == currentSlice  ||
             result.heartStart == currentSlice ||
             result.neckStart == currentSlice  ||
             result.skullStart == currentSlice ||
             (
               fabs( result.lungStart - currentSlice ) > DATABASE_MIN_INTERVAL_ROI &&
               fabs( result.heartStart - currentSlice ) > DATABASE_MIN_INTERVAL_ROI &&
               fabs( result.neckStart - currentSlice ) > DATABASE_MIN_INTERVAL_ROI &&
               fabs( result.skullStart - currentSlice ) > DATABASE_MIN_INTERVAL_ROI
             );
   }

   Database createLearningDatabase( const std::vector<ui32>& bin, ui32 binTest, const TestVolume& test )
   {
      Database outDat;
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      ensure( bin.size() == results.size(), "incompatible size, regenerate volume database!" );

      ui32 nbTest = 0;
      ui32 nbLearning = 0;
      for ( ui32 n = 0; n < bin.size(); ++n )
      {
         std::cout << "read volume database=" << n << std::endl;
         typedef ClassifierMlp< Buffer1D<ui8> >::Database   DatImage;
         DatImage  dat;
         dat.read( DATABASE_FULL_CASE( results[ n ].id ) );
         for ( ui32 nn = 0; nn < dat.size(); ++nn )
         {
            if ( !useSlice( results[ n ], nn ) )
               continue;

            DatImage::Sample& src = dat[ nn ];
            Database::Sample s;

            Buffer1D<ui8> buf( src.input );
            Image<ui8> i( src.input, REGION_DETECTION_SOURCE_IMG_X, REGION_DETECTION_SOURCE_IMG_Y, 1 );
            s.input = test.getFeatures( i );
            s.output = src.output;
            s.debug = src.debug;
            if ( binTest == bin[ n ] )
            {
               ++nbTest;
               s.type = Database::Sample::TESTING;
            } else {
               ++nbLearning;
               s.type = Database::Sample::LEARNING;
            }
            outDat.add( s );
         }
      }
      std::cout << "nbLearning volumes = " << nbLearning << " test=" << nbTest << std::endl;
      return outDat;
   }

   // use the volume database (where each MPR is already computed) and export the result + ground truth
   void testResultVolumeDatabase( TestVolume& test, const std::vector<ui32>& bins, ui32 binToTest )
   {
      typedef Buffer1D<ui8>         Point;
      typedef ClassifierMlp<Point>  Classifier;
      typedef Classifier::Database  Database;

      ui32 nbCases = 0;
      Buffer1D<double> counts( 5 );
      Buffer1D<double> errors( 5 );
      
      // extract the bin that needs to be tested
      std::vector<ui32> idToTest;
      for ( ui32 n = 0; n < bins.size(); ++n )
      {
         if ( bins[ n ] == binToTest )
            idToTest.push_back( n );
      }

      
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );

      Timer t1;
      for ( int index = 0; index < idToTest.size(); ++index )
      {
         ui32 n = idToTest[ index ];

         std::cout << "test case database:" << n << std::endl;
         Database dat;
         dat.read( DATABASE_FULL_CASE( results[ n ].id ) );
         Image<ui8> mprz;
         readBmp( mprz, std::string( PREVIEW_CASE ) + val2str( results[ n ].id ) + ".bmp" );


         TestVolume::Result idresult;
         idresult.sliceIds = std::vector<ui32>( mprz.sizey() );
         idresult.probabilities = std::vector<f64>( mprz.sizey() );

         
         std::cout << "size im=" << mprz.sizey() << " dat=" << dat.size() << std::endl;

         // export ground truth
         std::cout << "write ground truth" << std::endl;
         setColorIntensity( 0, 1 );
         setColorIntensity( 1, 1 );
         setColorIntensity( 2, 1 );
         setColorIntensity( 3, 1 );
         setColorIntensity( 4, 1 );
         for ( ui32 nnn = std::min<ui32>( mprz.sizex(), 10 ); nnn < std::min<ui32>( mprz.sizex(), 20 ); ++nnn )
         {
            ui8* p;
            if ( results[ n ].neckStart > 0 )
            {
               p = mprz.point( nnn, (ui32)results[ n ].neckStart );
               p[ 0 ] = colors[ 1 ][ 0 ];
               p[ 1 ] = colors[ 1 ][ 1 ];
               p[ 2 ] = colors[ 1 ][ 2 ];
            }

            if ( results[ n ].heartStart > 0 )
            {
               p = mprz.point( nnn, (ui32)results[ n ].heartStart );
               p[ 0 ] = colors[ 2 ][ 0 ];
               p[ 1 ] = colors[ 2 ][ 1 ];
               p[ 2 ] = colors[ 2 ][ 2 ];
            }

            if ( results[ n ].lungStart > 0 )
            {
               p = mprz.point( nnn, (ui32)results[ n ].lungStart );
               p[ 0 ] = colors[ 3 ][ 0 ];
               p[ 1 ] = colors[ 3 ][ 1 ];
               p[ 2 ] = colors[ 3 ][ 2 ];
            }

            if ( results[ n ].skullStart > 0 )
            {
               p = mprz.point( nnn, (ui32)results[ n ].skullStart );
               p[ 0 ] = colors[ 4 ][ 0 ];
               p[ 1 ] = colors[ 4 ][ 1 ];
               p[ 2 ] = colors[ 4 ][ 2 ];
            }
         }

         std::cout << "export result" << std::endl;
         for ( ui32 nn = 0; nn < dat.size(); ++nn )
         {
            Image<ui8> i( dat[ nn ].input, REGION_DETECTION_SOURCE_IMG_X, REGION_DETECTION_SOURCE_IMG_Y, 1 );
            Buffer1D<double> f = test.getFeatures( i );

            f64 proba = 0;
            ui32 r = test.rawTest( f, proba );
            idresult.sliceIds[ nn ] = r;
            idresult.probabilities[ nn ] = proba;

            setColorIntensity( 0, proba );
            setColorIntensity( 1, proba );
            setColorIntensity( 2, proba );
            setColorIntensity( 3, proba );

            
            // mark result
            for ( ui32 nnn = 0; nnn < std::min<ui32>( mprz.sizex(), 10 ); ++nnn )
            {
               ui8* p = mprz.point( nnn, nn );
               p[ 0 ] = colors[ r ][ 0 ];
               p[ 1 ] = colors[ r ][ 1 ];
               p[ 2 ] = colors[ r ][ 2 ];
            }
         }

         // compute the final result
         TestVolume::ResultFinal final = test.test( idresult );

         if ( final.neckStart > 0 )
         {
            ++counts[ 1 ];
            errors[ 1 ]  +=  fabs( results[ n ].neckStart - final.neckStart );
         }

         if ( final.heartStart > 0 )
         {
            ++counts[ 2 ];
            errors[ 2 ] += fabs( results[ n ].heartStart - final.heartStart );
         }
         std::cout << "heart found:" << final.heartStart << " truth=" << results[ n ].heartStart << std::endl;

         if ( final.lungStart > 0 )
         {
            ++counts[ 3 ];
            errors[ 3 ]  +=  fabs( results[ n ].lungStart - final.lungStart );
         }

         if ( final.skullStart > 0 )
         {
            ++counts[ 4 ];
            errors[ 4 ]  +=  fabs( results[ n ].skullStart - final.skullStart );
         }
         ++nbCases;

         for ( ui32 nnn = std::min<ui32>( mprz.sizex(), 20 ); nnn < mprz.sizex(); ++nnn )
         {
            ui8* p = 0;
            if ( final.neckStart > 0 && final.neckStart < (int)mprz.sizey() )
            {
               p = mprz.point( nnn, final.neckStart );
               p[ 0 ] = colors[ 1 ][ 0 ];
               p[ 1 ] = colors[ 1 ][ 1 ];
               p[ 2 ] = colors[ 1 ][ 2 ];
            }

            if ( final.heartStart > 0  && final.heartStart < (int)mprz.sizey() )
            {
               p = mprz.point( nnn, final.heartStart );
               p[ 0 ] = colors[ 2 ][ 0 ];
               p[ 1 ] = colors[ 2 ][ 1 ];
               p[ 2 ] = colors[ 2 ][ 2 ];
            }

            if ( final.lungStart > 0  && final.lungStart < (int)mprz.sizey() )
            {
               p = mprz.point( nnn, final.lungStart );
               p[ 0 ] = colors[ 3 ][ 0 ];
               p[ 1 ] = colors[ 3 ][ 1 ];
               p[ 2 ] = colors[ 3 ][ 2 ];
            }

            if ( final.skullStart > 0  && final.skullStart - 1 < (int)mprz.sizey() )
            {
               p = mprz.point( nnn, final.skullStart - 1 );
               p[ 0 ] = colors[ 4 ][ 0 ];
               p[ 1 ] = colors[ 4 ][ 1 ];
               p[ 2 ] = colors[ 4 ][ 2 ];
            }
         }

         writeBmp( mprz, std::string( PREVIEW_CASE_MARK ) + val2str( results[ n ].id ) + ".bmp" );
      }

      std::cout << "mean error in slice:" << std::endl
                << "neck:"  <<( errors[ 1 ] / counts[ 1 ] ) << std::endl
                << "heart:" <<( errors[ 2 ] / counts[ 2 ] ) << std::endl
                << "lung:"  <<( errors[ 3 ]  / counts[ 3 ] ) << std::endl
                << "skull:"  <<( errors[ 4 ]  / counts[ 4 ] ) << std::endl;

      std::cout << "mean time=" << t1.getCurrentTime() / nbCases << std::endl;
   }

   int findIndexFromId( const std::vector<RegionResult::Result>& results, ui32 id )
   {
      for ( size_t n = 0; n < results.size(); ++n )
      {
         if ( results[ n ].id == id )
            return (int)n;
      }
      return -1;
   }

   void registrationExport()
   {
      std::vector<RegionResult::Result> resultsReg = RegionResult::readResults( REGISTRATION_INPUT );
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );

      double errorNeck = 0;
      double errorLung = 0;
      double errorHeart = 0;
      for ( ui32 n = 0; n < resultsReg.size(); ++n )
      {
         Image<ui8> mprz;
         readBmp( mprz, std::string( PREVIEW_CASE ) + val2str( resultsReg[ n ].id ) + ".bmp" );

         
         // result found
         for ( ui32 nnn = 0; nnn < std::min<ui32>( mprz.sizex(), 10 ); ++nnn )
         {
            ui8* p = mprz.point( nnn, (ui32)resultsReg[ n ].neckStart );
            p[ 0 ] = colors[ 1 ][ 0 ];
            p[ 1 ] = colors[ 1 ][ 1 ];
            p[ 2 ] = colors[ 1 ][ 2 ];

            p = mprz.point( nnn, (ui32)resultsReg[ n ].heartStart );
            p[ 0 ] = colors[ 2 ][ 0 ];
            p[ 1 ] = colors[ 2 ][ 1 ];
            p[ 2 ] = colors[ 2 ][ 2 ];

            p = mprz.point( nnn, (ui32)resultsReg[ n ].lungStart );
            p[ 0 ] = colors[ 3 ][ 0 ];
            p[ 1 ] = colors[ 3 ][ 1 ];
            p[ 2 ] = colors[ 3 ][ 2 ];
         }
         
         // ground truth
         ui32 id = findIndexFromId( results, resultsReg[ n ].id );   // look up the ID from the gound thruth
         for ( ui32 nnn = std::min<ui32>( mprz.sizex(), 10 ); nnn < std::min<ui32>( mprz.sizex(), 20 ); ++nnn )
         {
            ui8* p = 0;
            if ( (ui32)results[ id ].neckStart < mprz.sizey() && results[ id ].neckStart >= 0 )
            {
               p = mprz.point( nnn, (ui32)results[ id ].neckStart );
               p[ 0 ] = colors[ 1 ][ 0 ];
               p[ 1 ] = colors[ 1 ][ 1 ];
               p[ 2 ] = colors[ 1 ][ 2 ];
            }

            p = mprz.point( nnn, (ui32)results[ id ].heartStart );
            p[ 0 ] = colors[ 2 ][ 0 ];
            p[ 1 ] = colors[ 2 ][ 1 ];
            p[ 2 ] = colors[ 2 ][ 2 ];

            p = mprz.point( nnn, (ui32)results[ id ].lungStart );
            p[ 0 ] = colors[ 3 ][ 0 ];
            p[ 1 ] = colors[ 3 ][ 1 ];
            p[ 2 ] = colors[ 3 ][ 2 ];
         }

         errorNeck +=  fabs( results[ id ].neckStart - resultsReg[ n ].neckStart );
         errorHeart += fabs( results[ id ].heartStart - resultsReg[ n ].heartStart );
         errorLung +=  fabs( results[ id ].lungStart - resultsReg[ n ].lungStart );
         
         writeBmp( mprz, std::string( PREVIEW_CASE_REG ) + val2str( resultsReg[ n ].id ) + ".bmp" );
      }

      std::cout << "mean error in slice:" << std::endl
                << "neck:"  <<( errorNeck  / resultsReg.size() ) << std::endl
                << "heart:" <<( errorHeart / resultsReg.size() ) << std::endl
                << "lung:"  <<( errorLung  / resultsReg.size() ) << std::endl;
   }
};

TESTER_TEST_SUITE(TestRegion);
 
TESTER_TEST(createDatasets);
//TESTER_TEST(createVolumeDatabase);
//TESTER_TEST(createPreview);
TESTER_TEST(learnSvm);
//TESTER_TEST(learnMlp);


//TESTER_TEST(registrationExport);
//TESTER_TEST(test);
TESTER_TEST_SUITE_END();
