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
   
/*
   void testExtractSlice()
   {
      
      for ( ui32 n = 0; n < 5; ++n )
      {
         std::cout << "case=" << n << std::endl;
         Volume volume;
         bool loaded = loadSimpleFlatFile( datasets[ n ], volume );
         TESTER_ASSERT( loaded );
         
         vector3f center = volume.indexToPosition( vector3f( 0, 0, static_cast<f32>( volume.size()[ 2 ] ) / 2 ) );
         Image<ui8> image = extractSlice( volume, center[ 2 ] );


         core::extend( image, 3 );
         writeBmp( image, std::string( "c:/tmp/mpr-1-" ) + val2str( n ) + ".bmp" );
      }
   }*/



   void createDatasets()
   {
      typedef Buffer1D<double>      Point;
      typedef ClassifierMlp<Point>  Classifier;
      typedef Classifier::Database  Database;

      RegionResult::generateSourceDatabase( CASES_DESC, DATABASE_SOURCE );
      RegionResult::generateFeatureDatabase();


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
            dat.add( Database::Sample( mpr_xy, 0, Database::Sample::LEARNING ) );
         }

         dat.write( DATABASE_FULL_CASE( results[ n ].id ) );
      }
   }

   static std::vector<ui32> createBins( ui32& nbBins )
   {
      nbBins = 10;
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

      return bins;
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

   ClassifierSvm< Buffer1D<double> >::Database createLearningDatabase( const std::vector<ui32>& bin, ui32 binTest )
   {
      typedef Buffer1D<double>      Point;
      typedef ClassifierSvm<Point>  Classifier;
      typedef Classifier::Database  Database;
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      ensure( results.size() == bin.size(), "must be the same database..." );


      Database dat;
      dat.read( HAAR_SELECTION_DATABASE ); // HAAR_SELECTION_DATABASE
      std::cout << "number of slice in database=" << dat.size() << std::endl;

      ui32 currentCaseId = 0;
      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         // update the current caseId
         std::string id = string_from_Buffer1D( dat[ n ].debug );
         if ( id != "" && id != "noname" )
         {
            currentCaseId = str2val<ui32>( id );
         }

         // assign the learning/testing
         int datid = findIndexFromId( results, currentCaseId );
         ensure( datid != -1, "should always find it" );
         if ( bin[ datid ] != binTest )
         {
            dat[ n ].type = Database::Sample::LEARNING;
         } else {
            dat[ n ].type = Database::Sample::TESTING;
         }
      }
      return dat;
   }

   void learnSvm()
   {
      typedef Buffer1D<double>      Point;
      typedef ClassifierSvm<Point>  Classifier;
      typedef Classifier::Database  Database;

      ui32 nbBins = 0;
      std::vector<ui32> bins = createBins( nbBins );

      Buffer1D<double> counts( NB_CLASS );
      Buffer1D<double> errors( NB_CLASS );
      for ( ui32 n = 0; n < nbBins; ++n )
      {
         Database selectedHaarDatabaseNormalized = createLearningDatabase( bins, n );

         Classifier classifier( 1, true );
         classifier.learn( selectedHaarDatabaseNormalized, make_buffer1D<double>( 1, 100 ) );
         classifier.test( selectedHaarDatabaseNormalized );

         //testResult( &classifier );
         testResultVolumeDatabase( &classifier, bins, n, counts, errors );
      }

      std::cout << "TOTAL mean error in slice:" << std::endl
                << "neck:"  <<( errors[ 1 ]  / counts[ 1 ] ) << std::endl
                << "heart:" <<( errors[ 2 ]  / counts[ 2 ] ) << std::endl
                << "lung:"  <<( errors[ 3 ]  / counts[ 3 ] ) << std::endl
                << "skull:" <<( errors[ 4 ]  / counts[ 4 ] ) << std::endl;
   }
/*
   void learnMlp()
   {
      srand( (unsigned)time(0) );
      typedef Buffer1D<double>      Point;
      typedef ClassifierMlp<Point>  Classifier;
      typedef Classifier::Database  Database;

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

   // use the volume database (where each MPR is already computed) and export the result + ground truth
   void testResultVolumeDatabase( Classifier< Buffer1D<double> >* classifier, const std::vector<ui32>& bins, ui32 binTest, Buffer1D<double>& totalCounts, Buffer1D<double>& totalErrors )
   {
      typedef Buffer1D<ui8>         Point;
      typedef ClassifierMlp<Point>  Classifier;
      typedef Classifier::Database  Database;

      ui32 nbCases = 0;
      Buffer1D<double> counts( NB_CLASS );
      Buffer1D<double> errors( NB_CLASS );
      

      TestVolume test( classifier, HAAR_FEATURES, PREPROCESSING_HAAR, HAAR_SELECTION );
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );

      Timer t1;
      ensure( bins.size() == results.size(), "should be the same database" );
      for ( int n = 0; n < results.size(); ++n )
      {
         std::cout << bins[ n ] << " " << binTest << std::endl;
         if ( bins[ n ] != binTest )
            continue;   // we just test the testing samples
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
         for ( ui32 i = 0; i < NB_CLASS; ++i )
         {
            setColorIntensity( i, 1 );
         }
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

            for ( ui32 i = 0; i < NB_CLASS; ++i )
            {
               setColorIntensity( i, proba );
            }
            
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

         for ( ui32 i = 0; i < NB_CLASS; ++i )
         {
            setColorIntensity( i, 1 );
         }
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

            if ( final.skullStart > 0  && final.skullStart < (int)mprz.sizey() )
            {
               p = mprz.point( nnn, final.skullStart );
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

      for ( ui32 n = 0; n < NB_CLASS; ++n )
      {
         totalCounts[ n ] += counts[ n ];
         totalErrors[ n ] += errors[ n ];
      }
   }

   /*
   void testResult( Classifier< Buffer1D<double> >* classifier )
   {
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      TestVolume test( classifier, HAAR_FEATURES, PREPROCESSING_HAAR, HAAR_SELECTION );
      for ( ui32 n = 0; n < results.size(); ++n )
      {
         Volume v;
         bool loaded = loadSimpleFlatFile( DATA_PATH "case" +val2str( results[ n ].id ) + ".mf2", v );
         TESTER_ASSERT( loaded );

         test.rawTest( v );
      }
   }*/


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
 
//TESTER_TEST(createDatasets);
//TESTER_TEST(createVolumeDatabase);

//TESTER_TEST(createPreview);
TESTER_TEST(learnSvm);
//TESTER_TEST(learnMlp);


//TESTER_TEST(registrationExport);
//TESTER_TEST(test);
TESTER_TEST_SUITE_END();
