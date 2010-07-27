#include <tester/register.h>
#include "globals.h"
#include <regionDetection/features.h>
#include <regionDetection/read-result.h>
#include <regionDetection/test.h>
#include <regionDetection/correction.h>
#include <regionDetection/correction2.h>
#include <regionDetection/correction3.h>
#include <regionDetection/correction4.h>
#include <regionDetection/regionDetection.h>

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;
using namespace nll::detect;


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
      srand( 0 );
      RegionResult::generateFeatureDatabase();


      std::cout << "haar selection..." << std::endl;
      Database haarDatabaseNormalized;
      haarDatabaseNormalized.read( NORMALIZED_HAAR );
      //FeatureTransformationPca<Point> pearson( FEATURE_SELECTION_SIZE );
      FeatureTransformationPca<Point> pearson;
      pearson.compute( haarDatabaseNormalized, FEATURE_SELECTION_SIZE );
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
      nbBins = LEARNING_NB_BINS;
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<ui32> bins( results.size() );
      for ( ui32 n = 0; n < results.size(); ++n )
      {
         bins[ n ] = (ui32)( n / ( (f32)results.size() / ( nbBins ) + 0.001 ) );
      }
      core::randomize( bins, 2.0f );
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

   int findIndexFromId( const std::vector<RegionResult::Measure>& results, ui32 id )
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

   struct   ErrorReporting
   {
      ui32  id;
      float errorInSlice;
      ui32  label;

      ErrorReporting( ui32 i, float e, ui32 l ) : id( i ), errorInSlice( e ), label( l )
      {}
   };

   void analyseResults( const std::vector<ErrorReporting>& reporting, const std::vector<RegionResult::Measure>& measures, const std::vector<RegionResult::Result>& results )
   {
      // do the results' analysis
      ensure( results.size() == measures.size(), "must be the same" );

      std::vector<float> sumSliceError( NB_CLASS );
      std::vector< std::vector< float > > errorInMM( NB_CLASS );
      
      for ( size_t n = 0; n < reporting.size(); ++n )
      {
         int id = findIndexFromId( results, reporting[ n ].id );
         ensure( id != -1, "error: can't find the case in the measures" );
         float spacing = measures[ id ].height / (float)measures[ id ].numberOfSlices;

         errorInMM[ reporting[ n ].label ].push_back(  reporting[ n ].errorInSlice * spacing );
         sumSliceError[ reporting[ n ].label ] += reporting[ n ].errorInSlice;
      }

      const float moreThanThresholdMm = 10.0f; // 10mm
      std::vector<float> means( NB_CLASS );
      std::vector<float> stddev( NB_CLASS );
      std::vector<float> max( NB_CLASS );
      std::vector<ui32>  moreThan( NB_CLASS );

      for ( ui32 n = 1; n < NB_CLASS; ++n )
      {
         for ( ui32 nn = 0; nn < errorInMM[ n ].size(); ++nn )
         {
            float errorMm = errorInMM[ n ][ nn ];
            max[ n ] = std::max<float>( max[ n ], errorMm );
            means[ n ] += errorMm;
            if ( errorMm > moreThanThresholdMm )
               ++moreThan[ n ];
         }
         means[ n ] /= errorInMM[ n ].size();
      }

      for ( ui32 n = 1; n < NB_CLASS; ++n )
      {
         for ( ui32 nn = 0; nn < errorInMM[ n ].size(); ++nn )
         {
            float errorMm = errorInMM[ n ][ nn ];
            stddev[ n ] += sqr( errorMm - means[ n ] );
         }
         stddev[ n ] = sqrt( stddev[ n ] / errorInMM[ n ].size() );
      }

      std::cout << "TOTAL mean error in slice:" << std::endl
                << "neck:"  <<( sumSliceError[ 1 ]  / errorInMM[ 1 ].size() ) << std::endl
                << "heart:" <<( sumSliceError[ 2 ]  / errorInMM[ 2 ].size() ) << std::endl
                << "lung:"  <<( sumSliceError[ 3 ]  / errorInMM[ 3 ].size() ) << std::endl
                << "skull:" <<( sumSliceError[ 4 ]  / errorInMM[ 4 ].size() ) << std::endl
                << "hips:" << ( sumSliceError[ 5 ]  / errorInMM[ 5 ].size() ) << std::endl;

      std::cout << "Total mean error in mm" << std::endl
                << "neck:"  <<( means[ 1 ] ) << std::endl
                << "heart:" <<( means[ 2 ] ) << std::endl
                << "lung:"  <<( means[ 3 ] ) << std::endl
                << "skull:" <<( means[ 4 ] ) << std::endl
                << "hips:"  <<( means[ 5 ] ) << std::endl;

      std::cout << "Total ROIs more than " << moreThanThresholdMm << " mm" << std::endl
                << "neck:"  <<( moreThan[ 1 ] ) << std::endl
                << "heart:" <<( moreThan[ 2 ] ) << std::endl
                << "lung:"  <<( moreThan[ 3 ] ) << std::endl
                << "skull:" <<( moreThan[ 4 ] ) << std::endl
                << "hips:"  <<( moreThan[ 5 ] ) << std::endl;

      std::cout << "Max error in mm " << moreThanThresholdMm << " mm" << std::endl
                << "neck:"  <<( max[ 1 ] ) << std::endl
                << "heart:" <<( max[ 2 ] ) << std::endl
                << "lung:"  <<( max[ 3 ] ) << std::endl
                << "skull:" <<( max[ 4 ] ) << std::endl
                << "hips:" << ( max[ 5 ] ) << std::endl;

      std::cout << "stddev in mm " << moreThanThresholdMm << " mm" << std::endl
                << "neck:"  <<( stddev[ 1 ] ) << std::endl
                << "heart:" <<( stddev[ 2 ] ) << std::endl
                << "lung:"  <<( stddev[ 3 ] ) << std::endl
                << "skull:" <<( stddev[ 4 ] ) << std::endl
                << "hips:"  <<( stddev[ 5 ] ) << std::endl;

      const float step = 5.0f;         // in mm
      const float maxstep  = 60.0f;    // in mm
      std::cout << "data distribution:" << std::endl;
      for ( ui32 n = 1; n < NB_CLASS; ++n )
      {
         std::cout << " class:" << n << std::endl;
         for ( float interval = step; interval < maxstep; interval += step )
         {
            ui32 nberrors = 0;
            for ( ui32 nn = 0; nn < reporting.size(); ++nn )
            {
               int caseid = findIndexFromId( measures, reporting[ nn ].id );
               ensure( caseid >= 0, "error:" + val2str( reporting[ nn ].id ) );
               float errorMM = reporting[ nn ].errorInSlice * measures[ caseid ].height / measures[ caseid ].numberOfSlices;
               if ( reporting[ nn ].label == n && errorMM >= ( interval - step ) && errorMM < interval )
                  ++nberrors;
            }
            std::cout << "  error[" << interval - step << "|" << interval << "]=" << nberrors << std::endl;
         }

         ui32 nberrors = 0;
         for ( ui32 nn = 0; nn < reporting.size(); ++nn )
         {
            int caseid = findIndexFromId( measures, reporting[ nn ].id );
            ensure( caseid >= 0, "error" );
            float errorMM = reporting[ nn ].errorInSlice * measures[ caseid ].height / measures[ caseid ].numberOfSlices;
            if ( reporting[ nn ].label == n && errorMM >= maxstep )
               ++nberrors;
         }
         std::cout << "  error[>=" << maxstep << "]=" << nberrors << std::endl;
      }
   }

   void learnSvm()
   {
      srand( 1 );

      typedef Buffer1D<double>      Point;
      typedef ClassifierSvm<Point>  Classifier;
      typedef Classifier::Database  Database;

      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );

      ui32 nbBins = 0;
      std::vector<ui32> bins = createBins( nbBins );
      Buffer1D<double> params = make_buffer1D<double>( 0.4, 100 );

      std::vector<ErrorReporting> reporting;
      for ( ui32 n = 0; n < nbBins; ++n )
      {
         Database selectedHaarDatabaseNormalized = createLearningDatabase( bins, n );

         Classifier classifier( 1, true );
         classifier.learn( selectedHaarDatabaseNormalized, params );
         classifier.test( selectedHaarDatabaseNormalized );

         //testResult( &classifier );
         testResultVolumeDatabase( &classifier, bins, n, reporting );
      }

      // do the results' analysis
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      ensure( results.size() == measures.size(), "must be the same" );
      analyseResults( reporting, measures, results );

      // learn the full classifier
      Classifier classifier( 1, true );
      classifier.learnAllDatabase( createLearningDatabase( bins, 0 ), params );
      classifier.write( FINAL_SVM_CLASSIFIER );
   }

   void learnKnn()
   {
      srand( 1 );

      typedef Buffer1D<double>                                  Point;
      typedef nll::algorithm::MetricEuclidian<Point>            MetricEuclidian;
      typedef ClassifierNearestNeighbor<Point, MetricEuclidian> Classifier;
      typedef Classifier::Database                              Database;

      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );
      MetricEuclidian euclidian;

      ui32 nbBins = 0;
      std::vector<ui32> bins = createBins( nbBins );
      Buffer1D<double> params = make_buffer1D<double>( 1 );

      std::vector<ErrorReporting> reporting;
      for ( ui32 n = 0; n < nbBins; ++n )
      {
         Database selectedHaarDatabaseNormalized = createLearningDatabase( bins, n );

         Classifier classifier( &euclidian );
         classifier.learn( selectedHaarDatabaseNormalized, params );
         classifier.test( selectedHaarDatabaseNormalized );

         //testResult( &classifier );
         testResultVolumeDatabase( &classifier, bins, n, reporting );
      }

      // do the results' analysis
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      ensure( results.size() == measures.size(), "must be the same" );
      analyseResults( reporting, measures, results );

      // learn the full classifier
      Classifier classifier( &euclidian );
      classifier.learnAllDatabase( createLearningDatabase( bins, 0 ), params );
      classifier.write( FINAL_SVM_CLASSIFIER );
   }

   void learnMlp()
   {
      srand( 1 );

      typedef Buffer1D<double>      Point;
      typedef ClassifierMlp<Point>  Classifier;
      typedef Classifier::Database  Database;

      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );

      ui32 nbBins = 0;
      std::vector<ui32> bins = createBins( nbBins );
      Buffer1D<double> params = make_buffer1D<double>( 30, 0.5, 80 );

      std::vector<ErrorReporting> reporting;
      for ( ui32 n = 0; n < nbBins; ++n )
      {
         Database selectedHaarDatabaseNormalized = createLearningDatabase( bins, n );

         Classifier classifier;
         classifier.learn( selectedHaarDatabaseNormalized, params );
         classifier.test( selectedHaarDatabaseNormalized );

         //testResult( &classifier );
         testResultVolumeDatabase( &classifier, bins, n, reporting );
      }

      // do the results' analysis
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      ensure( results.size() == measures.size(), "must be the same" );
      analyseResults( reporting, measures, results );

      // learn the full classifier
      Classifier classifier;
      classifier.learnAllDatabase( createLearningDatabase( bins, 0 ), params );
      classifier.write( FINAL_SVM_CLASSIFIER );
   }

   // do the full process from an actual volume on an independent dataset
   void testValidationDataSvm()
   {
      typedef Buffer1D<double>      Point;
      typedef ClassifierSvm<Point>  Classifier;
      typedef Classifier::Database  Database;

      //std::vector<RegionResult::Result> results = RegionResult::readResults( VALIDATION_CASES_DESC );
      std::vector<RegionResult::Result> results = RegionResult::readResults( VALIDATION_OVERWEIGHT );
      //std::vector<RegionResult::Result> results = RegionResult::readResults( VALIDATION_COMAPRE );
      std::vector<RegionResult::Measure> measures;

      Classifier classifier( 1, true );
      classifier.read( FINAL_SVM_CLASSIFIER );
      TestVolume test( &classifier, HAAR_FEATURES, PREPROCESSING_HAAR, HAAR_SELECTION );

      std::vector<RegionResult::Measure> measuresTraining;
      measuresTraining = RegionResult::readMeasures( DATABASE_MEASURES_CORRECTION );
      CorrectPosition2 corrector( measuresTraining );

      std::vector<ErrorReporting> reporting;
      for ( ui32 nn = 0; nn < results.size(); ++nn )
      {
         std::cout << "test validation case=" << results[ nn ].id << std::endl;
         // read volume
         Volume volume;
         bool loaded = loadSimpleFlatFile( DATA_PATH "case" + val2str( results[ nn ].id ) + ".mf2", volume );
         TESTER_ASSERT( loaded );
         measures.push_back( RegionResult::Measure( results[ nn ].id, volume.size()[ 2 ], volume.size()[ 2 ] * volume.getSpacing()[ 2 ] ) );

         // vizualise the results
         Image<ui8> mprz = extractXZ( volume );
         extend( mprz, 3 );

         // compute the locations
         TestVolume::Result pbs;
         TestVolume::ResultFinal final = test.test( volume, pbs );

         // results
         previewLabel( mprz, 40, mprz.sizex() / 2, make_buffer1D<ui32>( 0, final.neckStart, final.heartStart, final.lungStart, final.skullStart, final.hipsStart ) );
         Buffer1D<float> labelsmm = make_buffer1D<float>( 0, final.neckStart * volume.getSpacing()[ 2 ],
                                                             final.heartStart * volume.getSpacing()[ 2 ],
                                                             final.lungStart * volume.getSpacing()[ 2 ],
                                                             final.skullStart * volume.getSpacing()[ 2 ],
                                                             final.hipsStart * volume.getSpacing()[ 2 ] );
         corrector.correct( labelsmm );

         corrector.annotateProbability( labelsmm, mprz, volume.getSpacing()[ 2 ] );

         
         // uncomment to display label probabilities
         for ( ui32 y = 0; y < mprz.sizey(); ++y )
         {
            Buffer1D<double> f = test.getFeatures( volume, y );

            core::Buffer1D<double> probas( NB_CLASS );
            test.rawTest( f, probas );
            probas.print(std::cout);

            for ( ui32 i = 0; i < NB_CLASS; ++i )
            {
               setColorIntensity( i, probas[ i ] );
            }

            for ( ui32 label = 0; label < NB_CLASS; ++label )
            {
               const ui32 size = 3;
               ui32 start = label * size + 20;
               ui32 end = start + size;
               for ( ui32 nnn = start; nnn < end; ++nnn )
               {
                  setColorIntensity( label, probas[ label ] );
                  ui8* p = mprz.point( nnn, y );
                  p[ 0 ] = colors[ label ][ 0 ];
                  p[ 1 ] = colors[ label ][ 1 ];
                  p[ 2 ] = colors[ label ][ 2 ];
               }
            }
         }

         for ( ui32 i = 1; i < NB_CLASS; ++i )
            labelsmm[ i ] /= volume.getSpacing()[ 2 ];
         final.neckStart  = labelsmm[ 1 ];
         final.heartStart = labelsmm[ 2 ];
         final.lungStart  = labelsmm[ 3 ];
         final.skullStart = labelsmm[ 4 ];
         final.hipsStart  = labelsmm[ 5 ];
         for ( ui32 i = 0; i < NB_CLASS; ++i )
         {
            setColorIntensity( i, 1 );
         }
         previewLabel( mprz, mprz.sizex() / 2, mprz.sizex(), make_buffer1D<ui32>( 0, final.neckStart, final.heartStart, final.lungStart, final.skullStart, final.hipsStart ) );

         for ( ui32 i = 0; i < NB_CLASS; ++i )
            setColorIntensity( i, 1 );
         
         for ( ui32 z = 0; z < volume.size()[ 2 ]; ++z )
         {
            for ( ui32 i = 0; i < NB_CLASS; ++i )
               setColorIntensity( pbs.sliceIds[ z ], pbs.probabilities[ z ] );

            for ( ui32 n = 0; n < 10; ++n )
            {
               ui8* p;

               p = mprz.point( n, z );
               p[ 0 ] = colors[ pbs.sliceIds[ z ] ][ 0 ];
               p[ 1 ] = colors[ pbs.sliceIds[ z ] ][ 1 ];
               p[ 2 ] = colors[ pbs.sliceIds[ z ] ][ 2 ];
            }
         }

         for ( ui32 i = 0; i < NB_CLASS; ++i )
            setColorIntensity( i, 1 );

         // ground truth
         previewLabel( mprz, 10, 20, make_buffer1D<ui32>( 0, results[ nn ].neckStart, results[nn ].heartStart, results[ nn ].lungStart, results[ nn ].skullStart, results[ nn ].hipsStart ) );
         
         // reporting
         if ( results[ nn ].neckStart > 0 && final.neckStart > 0 )
         {
            reporting.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].neckStart - final.neckStart ), 1 ) );
         }
         if ( results[ nn ].heartStart > 0 && final.heartStart > 0 )
         {
            reporting.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].heartStart - final.heartStart ), 2 ) );
         }

         if ( results[ nn ].lungStart > 0 && final.lungStart > 0 )
         {
            reporting.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].lungStart - final.lungStart ), 3 ) );
         }

         if ( results[ nn ].skullStart > 0 && final.skullStart > 0 )
         {
            reporting.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].skullStart - final.skullStart ), 4 ) );
         }

         if ( results[ nn ].hipsStart > 0 && final.hipsStart > 0 )
         {
            reporting.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].hipsStart - final.hipsStart ), 5 ) );
         }
         writeBmp( mprz, std::string( PREVIEW_CASE_VALIDATION ) + val2str( results[ nn ].id ) + ".bmp" );
      }

      analyseResults( reporting, measures, results );
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
   void testResultVolumeDatabase( Classifier< Buffer1D<double> >* classifier, const std::vector<ui32>& bins, ui32 binTest, std::vector<ErrorReporting>& reporting  )
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
      for ( int n = 0; n < (int)results.size(); ++n )
      {
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
         previewLabel( mprz, 10, 20, make_buffer1D<ui32>( 0, results[ n ].neckStart, results[ n ].heartStart, results[ n ].lungStart, results[ n ].skullStart, results[ n ].hipsStart ) );

         std::cout << "export result" << std::endl;
         for ( ui32 nn = 0; nn < dat.size(); ++nn )
         {
            Image<ui8> i( dat[ nn ].input, REGION_DETECTION_SOURCE_IMG_X, REGION_DETECTION_SOURCE_IMG_Y, 1 );
            Buffer1D<double> f = test.getFeatures( i );

            f64 proba = 0;
            core::Buffer1D<double> probas( NB_CLASS );
            ui32 r = test.rawTest( f, proba );
            test.rawTest( f, probas );
            probas.print(std::cout);

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

            for ( ui32 label = 0; label < NB_CLASS; ++label )
            {
               const ui32 size = 3;
               ui32 start = label * size + 20;
               ui32 end = start + size;
               for ( ui32 nnn = start; nnn < end; ++nnn )
               {
                  setColorIntensity( label, probas[ label ] );
                  ui8* p = mprz.point( nnn, nn );
                  p[ 0 ] = colors[ label ][ 0 ];
                  p[ 1 ] = colors[ label ][ 1 ];
                  p[ 2 ] = colors[ label ][ 2 ];
               }
            }
         }

         // compute the final result
         TestVolume::ResultFinal final = test.test( idresult );

         if ( final.neckStart > 0 && results[ n ].neckStart > 0 )
         {
            float err = fabs( results[ n ].neckStart - final.neckStart );
            reporting.push_back( ErrorReporting( results[ n ].id, err, 1 ) );
            ++counts[ 1 ];
            errors[ 1 ]  +=  err;
         }

         if ( final.heartStart > 0 && results[ n ].heartStart > 0 )
         {
            float err = fabs( results[ n ].heartStart - final.heartStart );
            reporting.push_back( ErrorReporting( results[ n ].id, err, 2 ) );
            ++counts[ 2 ];
            errors[ 2 ] += err;
         }
         std::cout << "heart found:" << final.heartStart << " truth=" << results[ n ].heartStart << std::endl;

         if ( final.lungStart > 0 && results[ n ].lungStart > 0 )
         {
            float err = fabs( results[ n ].lungStart - final.lungStart );
            ++counts[ 3 ];
            reporting.push_back( ErrorReporting( results[ n ].id, err, 3 ) );
            errors[ 3 ]  +=  err;
         }

         if ( final.skullStart > 0 && results[ n ].skullStart > 0 )
         {
            float err = fabs( results[ n ].skullStart - final.skullStart );
            ++counts[ 4 ];
            reporting.push_back( ErrorReporting( results[ n ].id, err, 4 ) );
            errors[ 4 ]  +=  err;
         }

         if ( final.hipsStart > 0 && results[ n ].hipsStart > 0 )
         {
            float err = fabs( results[ n ].hipsStart - final.hipsStart );
            ++counts[ 5 ];
            reporting.push_back( ErrorReporting( results[ n ].id, err, 5 ) );
            errors[ 5 ]  +=  err;
         }
         ++nbCases;

         for ( ui32 i = 0; i < NB_CLASS; ++i )
         {
            setColorIntensity( i, 1 );
         }

         previewLabel( mprz, 20, mprz.sizex(), make_buffer1D<ui32>( 0, final.neckStart, final.heartStart, final.lungStart, final.skullStart, final.hipsStart ) );
         writeBmp( mprz, std::string( PREVIEW_CASE_MARK ) + val2str( results[ n ].id ) + ".bmp" );
      }

      std::cout << "mean error in slice:" << std::endl
                << "neck:"  <<( errors[ 1 ]  / counts[ 1 ] ) << std::endl
                << "heart:" <<( errors[ 2 ]  / counts[ 2 ] ) << std::endl
                << "lung:"  <<( errors[ 3 ]  / counts[ 3 ] ) << std::endl
                << "skull:" <<( errors[ 4 ]  / counts[ 4 ] ) << std::endl
                << "hips:"  <<( errors[ 5 ]  / counts[ 5 ] ) << std::endl;

      std::cout << "mean time=" << t1.getCurrentTime() / nbCases << std::endl;
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

   void extractXZFullResolution()
   {
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      for ( ui32 n = 0; n < results.size(); ++n )
      {
         Volume volume;
         bool loaded = loadSimpleFlatFile( DATA_PATH "case" + val2str( results[ n ].id ) + ".mf2", volume );
         TESTER_ASSERT( loaded );

         for ( ui32 nn = 0; nn < volume.size()[ 2 ]; ++nn )
         {
            Image<ui8> xz = extractSliceFull( volume, nn );
            extend( xz, 3 );
            writeBmp( xz, std::string( "c:/tmp/full/case" ) + val2str( results[ n ].id ) + "-slice-" + val2str( nn + 1 ) + ".bmp" );   // +1 as we do it 1-based numerotation, as the reader app to be consistant!
         }
      }
   }


   void registrationExport()
   {
      std::vector<ErrorReporting> reporting;
      std::vector<RegionResult::Result> resultsReg = RegionResult::readResults( REGISTRATION_INPUT );
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );

      for ( ui32 n = 0; n < resultsReg.size(); ++n )
      {
         Image<ui8> mprz;
         readBmp( mprz, std::string( PREVIEW_CASE ) + val2str( resultsReg[ n ].id ) + ".bmp" );

         
         // result found
         for ( ui32 nnn = 0; nnn < std::min<ui32>( mprz.sizex(), 10 ); ++nnn )
         {
            ui8* p;
            if ( resultsReg[ n ].neckStart > 0 && resultsReg[ n ].neckStart < mprz.sizey() )
            {
               p = mprz.point( nnn, (ui32)resultsReg[ n ].neckStart );
               p[ 0 ] = colors[ 1 ][ 0 ];
               p[ 1 ] = colors[ 1 ][ 1 ];
               p[ 2 ] = colors[ 1 ][ 2 ];
            }

            if ( resultsReg[ n ].heartStart > 0 && resultsReg[ n ].heartStart < mprz.sizey() )
            {
               p = mprz.point( nnn, (ui32)resultsReg[ n ].heartStart );
               p[ 0 ] = colors[ 2 ][ 0 ];
               p[ 1 ] = colors[ 2 ][ 1 ];
               p[ 2 ] = colors[ 2 ][ 2 ];
            }

            if ( resultsReg[ n ].lungStart > 0 && resultsReg[ n ].lungStart < mprz.sizey() )
            {
               p = mprz.point( nnn, (ui32)resultsReg[ n ].lungStart );
               p[ 0 ] = colors[ 3 ][ 0 ];
               p[ 1 ] = colors[ 3 ][ 1 ];
               p[ 2 ] = colors[ 3 ][ 2 ];
            }
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

         float eNeck =  fabs( results[ id ].neckStart - resultsReg[ n ].neckStart );
         float eHeart = fabs( results[ id ].heartStart - resultsReg[ n ].heartStart );
         float eLung = fabs( results[ id ].lungStart - resultsReg[ n ].lungStart );

         reporting.push_back( ErrorReporting( resultsReg[ n ].id, eNeck, 1 ) );
         reporting.push_back( ErrorReporting( resultsReg[ n ].id, eHeart, 2 ) );
         reporting.push_back( ErrorReporting( resultsReg[ n ].id, eLung, 3 ) );
         
         writeBmp( mprz, std::string( PREVIEW_CASE_REG ) + val2str( resultsReg[ n ].id ) + ".bmp" );
      }
      analyseResults( reporting, measures, results );
   }

   static void previewLabel( Image<ui8>& mprz, ui32 min, ui32 max, const Buffer1D<f32>& labelPosition )
   {
      Buffer1D<ui32> l( labelPosition.size() );
      for ( ui32 n = 0; n < labelPosition.size(); ++n )
         l[ n ] = (ui32)labelPosition[ n ];
      previewLabel( mprz, min, max, l );
   }

   static void previewLabel( Image<ui8>& mprz, ui32 min, ui32 max, const Buffer1D<ui32>& labelPosition )
   {
      for ( ui32 nnn = std::max<ui32>( 0, min ); nnn < std::min<ui32>( mprz.sizex(), max ); ++nnn )
      {
         for ( ui32 n = 1; n < NB_CLASS; ++n )
         {
            if ( labelPosition[ n ] > 0 && labelPosition[ n ] < (int)mprz.sizey() )
            {
               ui8* p = mprz.point( nnn, labelPosition[ n ] );
               p[ 0 ] = colors[ n ][ 0 ];
               p[ 1 ] = colors[ n ][ 1 ];
               p[ 2 ] = colors[ n ][ 2 ];
            }
         }
      }
   }

   // read the measures, split them in a test & learning set, then add gaussian error on the testing set
   // finally, try to detect & fix the error
   void testSimilarity()
   {
      std::vector<ErrorReporting> reportingCorrected;
      srand(18);

      const float testingRatio = 0.15f;
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );
      std::vector<RegionResult::Measure> measuresTraining;
      std::vector<RegionResult::Measure> measuresTest;

      // split the database
      const ui32 nbTesting = static_cast<ui32>( testingRatio * measures.size() );
      for ( ui32 n = 0; n < measures.size(); ++n )
         if ( n >= nbTesting )
            measuresTraining.push_back( measures[ n ] );
         else
         {
            //measuresTraining.push_back( measures[ n ] );
            measuresTest.push_back( measures[ n ] );
         }

      // create new templates by deforming the existing ones
         /*
      const ui32 nbTraining = (ui32)measuresTraining.size();
      for ( ui32 n = 0; n < nbTraining; ++n )
      {
         for ( float r = 0.8f; r < 1.2f; r += 0.1f )
         {
            RegionResult::Measure m = measuresTraining[ n ];
            m.heightNeck  *= r;
            m.heightHeart *= r;
            m.heightLung  *= r;
            m.heightSkull *= r;
            m.heightHips  *= r;
            measuresTraining.push_back( m );
         }
      }*/



      // training
      CorrectPosition correct( measuresTraining );

      // set the sampling statistics
      const double means[ NB_CLASS ] =
      {
         0, 6.23f, 6.87f, 3.38f, 10.4f, 7.0f
      };

      const double vars[ NB_CLASS ] =
      {
         0, 8.16f, 6.55f, 3.74f, 15.88f, 10.0f
      };
      const double probaMissing         = 0.1f;
      const double probaBigDeviation    = 0.1f;
      
      const double meanBigDeviation     = 0;
      const double varBigDeviation      = 150;


      // create test data
      const ui32 nbSamplePerMeasure = 20;
      std::vector<RegionResult::Measure> measuresTestSampled;
      for ( ui32 n = 0; n < measuresTest.size(); ++n )
      {
         std::cout << "case=" << measuresTest[ n ].id << std::endl;
         for ( ui32 sample = 0; sample < nbSamplePerMeasure; ++sample )
         {
            // sampling
            core::Buffer1D<float> labels = measuresTest[ n ].toArray();
            for ( ui32 nn = 1; nn < NB_CLASS; ++nn )
            {
               if ( labels[ nn ] < 0 )
                  continue;   // skip, we don't bother with it as we can't compare gound truth
               double p = core::generateUniformDistribution( 0, 1 );
               if ( p <= probaMissing )
               {
                  labels[ nn ] = -1;
                  continue;
               } else if ( p > probaMissing && ( p - probaMissing ) <= probaBigDeviation )
               {
                  labels[ nn ] += (float)generateGaussianDistribution( meanBigDeviation * core::generateSign(), varBigDeviation );
               } else {
                  labels[ nn ] += (float)generateGaussianDistribution( means[ nn ] * core::generateSign(), vars[ nn ] );
               }
            }

           // if ( n != 9 || sample != 1 )
           //    continue;

            Image<ui8> preview( std::string( PREVIEW_CASE ) + val2str( measuresTest[ n ].id ) + ".bmp" );
            Buffer1D<float> previewRef( NB_CLASS );
            Buffer1D<float> previewRes( NB_CLASS );
            for ( ui32 nn = 1; nn < NB_CLASS; ++nn )
            {
               // convert to MM
               previewRef[ nn ] = measuresTest[ n ].toArray()[ nn ] / ( measuresTest[ n ].height / measuresTest[ n ].numberOfSlices );
               previewRes[ nn ] = labels[ nn ] / ( measuresTest[ n ].height / measuresTest[ n ].numberOfSlices );
            }

            previewLabel( preview, 20, preview.sizex() / 2, previewRes );
            previewLabel( preview, 0, 20, previewRef  );

            //
            //
            //TODO update the method to compute error...
            correct.correct( labels );
            //
            //

            Buffer1D<float> previewCorrected( NB_CLASS );
            for ( ui32 nn = 1; nn < NB_CLASS; ++nn )
            {
               const float spacing = ( measuresTest[ n ].height / measuresTest[ n ].numberOfSlices );
               previewCorrected[ nn ] = labels[ nn ] / spacing;
            }
            if ( previewCorrected[ 4 ] > preview.sizey() )     // we need to clamp the skull: we choose a model with full skull, but it is not in the test volume
               previewCorrected[ 4 ] = (f32)preview.sizey() - 1;


            previewCorrected.print( std::cout );
            previewLabel( preview, preview.sizex() / 2, preview.sizex(), previewCorrected );
            writeBmp( preview, std::string( PREVIEW_CASE_CORRECTION ) + val2str( measuresTest[ n ].id )+ "-sample-" + val2str( sample ) + ".bmp" );

            std::cout << "case done" << std::endl;

            // reporting
            int nn = findIndexFromId( results, measuresTest[ n ].id );
            ensure( nn >= 0, "error" );
            if ( results[ nn ].neckStart > 0 && previewCorrected[ 1 ] > 0 )
            {
               reportingCorrected.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].neckStart - previewCorrected[ 1 ] ), 1 ) );
            }
            if ( results[ nn ].heartStart > 0 && previewCorrected[ 2 ] > 0 )
            {
               reportingCorrected.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].heartStart - previewCorrected[ 2 ] ), 2 ) );
            }

            if ( results[ nn ].lungStart > 0 && previewCorrected[ 3 ] > 0 )
            {
               reportingCorrected.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].lungStart - previewCorrected[ 3 ] ), 3 ) );
            }

            if ( results[ nn ].skullStart > 0 && previewCorrected[ 4 ] > 0 )
            {
               reportingCorrected.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].skullStart - previewCorrected[ 4 ] ), 4 ) );
            }

            if ( results[ nn ].hipsStart > 0 && previewCorrected[ 5 ] > 0 )
            {
               reportingCorrected.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].hipsStart - previewCorrected[ 5 ] ), 5 ) );
            }
         }
      }

      analyseResults( reportingCorrected, measures, results );
   }

   // check that the selected template to correst ROI position is good
   void testSelectedTemplate()
   {
      std::vector<ErrorReporting> reportingCorrected;
      srand(24);

      const float testingRatio = 0.15f;
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );
      std::vector<RegionResult::Measure> measuresTraining;
      std::vector<RegionResult::Measure> measuresTest;

      // split the database
      const ui32 nbTesting = static_cast<ui32>( testingRatio * measures.size() );
      for ( ui32 n = 0; n < measures.size(); ++n )
      {
         if ( n >= nbTesting )
         {
            ui32 nb = 0;
            Buffer1D<float> f = measures[ n ].toArray();
            for ( ui32 nn = 0; nn < f.size(); ++nn )
               nb += f[ nn ] > 0;
            if ( nb == 5 )
               measuresTraining.push_back( measures[ n ] );
         }
         else
         {
            //measuresTraining.push_back( measures[ n ] );
            measuresTest.push_back( measures[ n ] );
         }
      }

      CorrectPosition2 corrector( measuresTraining );
      
      // set the sampling statistics
      /*
      const double means[ NB_CLASS ] =
      {
         0, 6.23f, 6.87f, 3.38f, 10.4f, 7.0f
      };

      const double vars[ NB_CLASS ] =
      {
         0, 8.16f, 6.55f, 3.74f, 15.88f, 10.0f
      };
      */
      const double means[ NB_CLASS ] =
      {
         0, 0, 0, 0, 0, 0
      };

      const double vars[ NB_CLASS ] =
      {
         0, 0, 0, 0, 0, 0
      };

      const double probaMissing         = 0.2f;
      const double probaBigDeviation    = 0.0f;
      
      const double meanBigDeviation     = 0;
      const double varBigDeviation      = 150;


      // create test data
      const ui32 nbSamplePerMeasure = 20;
      std::vector<RegionResult::Measure> measuresTestSampled;
      for ( ui32 n = 0; n < measuresTest.size(); ++n )
      {
         std::cout << "case=" << measuresTest[ n ].id << std::endl;
         for ( ui32 sample = 0; sample < nbSamplePerMeasure; ++sample )
         {
            // sampling
            core::Buffer1D<float> labels = measuresTest[ n ].toArray();
            for ( ui32 nn = 1; nn < NB_CLASS; ++nn )
            {
               if ( labels[ nn ] < 0 )
                  continue;   // skip, we don't bother with it as we can't compare gound truth
               double p = core::generateUniformDistribution( 0, 1 );
               if ( p <= probaMissing )
               {
                  labels[ nn ] = -1;
                  continue;
               } else if ( p > probaMissing && ( p - probaMissing ) <= probaBigDeviation )
               {
                  labels[ nn ] += (float)generateGaussianDistribution( meanBigDeviation * core::generateSign(), varBigDeviation );
               } else {
                  labels[ nn ] += (float)generateGaussianDistribution( means[ nn ] * core::generateSign(), vars[ nn ] );
               }
            }

            //if ( n != 3 || sample != 2 )
            //   continue;

            Image<ui8> preview( std::string( PREVIEW_CASE ) + val2str( measuresTest[ n ].id ) + ".bmp" );
            Buffer1D<float> previewRef( NB_CLASS );
            Buffer1D<float> previewRes( NB_CLASS );
            for ( ui32 nn = 1; nn < NB_CLASS; ++nn )
            {
               // convert to MM
               previewRef[ nn ] = measuresTest[ n ].toArray()[ nn ] / ( measuresTest[ n ].height / measuresTest[ n ].numberOfSlices );
               previewRes[ nn ] = labels[ nn ] / ( measuresTest[ n ].height / measuresTest[ n ].numberOfSlices );
            }

            previewLabel( preview, 20, preview.sizex() / 2, previewRes );
            previewLabel( preview, 0, 20, previewRef  );

            //
            //
            //TODO update the method to compute error...
            labels.print( std::cout );
            corrector.correct( labels );
            //
            //

            Buffer1D<float> previewCorrected( NB_CLASS );
            for ( ui32 nn = 1; nn < NB_CLASS; ++nn )
            {
               const float spacing = ( measuresTest[ n ].height / measuresTest[ n ].numberOfSlices );
               previewCorrected[ nn ] = labels[ nn ] / spacing;
            }
            if ( previewCorrected[ 4 ] > preview.sizey() )     // we need to clamp the skull: we choose a model with full skull, but it is not in the test volume
               previewCorrected[ 4 ] = (f32)preview.sizey() - 1;


            previewCorrected.print( std::cout );
            previewLabel( preview, preview.sizex() / 2, preview.sizex(), previewCorrected );
            writeBmp( preview, std::string( PREVIEW_CASE_CORRECTION ) + val2str( measuresTest[ n ].id )+ "-sample-" + val2str( sample ) + ".bmp" );

            std::cout << "case done" << std::endl;

            // reporting
            int nn = findIndexFromId( results, measuresTest[ n ].id );
            ensure( nn >= 0, "error" );
            if ( results[ nn ].neckStart > 0 && previewCorrected[ 1 ] > 0 )
            {
               reportingCorrected.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].neckStart - previewCorrected[ 1 ] ), 1 ) );
            }
            if ( results[ nn ].heartStart > 0 && previewCorrected[ 2 ] > 0 )
            {
               reportingCorrected.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].heartStart - previewCorrected[ 2 ] ), 2 ) );
            }

            if ( results[ nn ].lungStart > 0 && previewCorrected[ 3 ] > 0 )
            {
               reportingCorrected.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].lungStart - previewCorrected[ 3 ] ), 3 ) );
            }

            if ( results[ nn ].skullStart > 0 && previewCorrected[ 4 ] > 0 )
            {
               reportingCorrected.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].skullStart - previewCorrected[ 4 ] ), 4 ) );
            }

            if ( results[ nn ].hipsStart > 0 && previewCorrected[ 5 ] > 0 )
            {
               reportingCorrected.push_back( ErrorReporting( results[ nn ].id, fabs( results[ nn ].hipsStart - previewCorrected[ 5 ] ), 5 ) );
            }
         }
      }

      RegionResult::writeMeasures( measuresTraining, DATABASE_MEASURES_CORRECTION );
      analyseResults( reportingCorrected, measures, results );
   }

   void displaySpacingStatistics()
   {
      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );
      std::map<float, int> r;
      for ( ui32 n = 0; n < measures.size(); ++n )
      {
         float spacing = measures[ n ].height / measures[ n ].numberOfSlices;
         ++r[ spacing ];
      }

      for ( std::map<float, int>::iterator it = r.begin(); it != r.end(); ++it )
         std::cout << "spacing=" << it->first << " nb=" << it->second << std::endl;
   }

   

   void testCenter()
   {
      Image<ui8> i( "C:/tmp/s3/TEST2.bmp" );
      decolor( i );
      centerImage( i );
      extend( i, 3 );
      writeBmp( i, "C:/tmp/s3/TEST.bmp" );
   }

   void createCorrectorDatabase( std::vector<RegionResult::Measure>& measuresTraining, std::vector<RegionResult::Measure>& measuresTest, float testingRatio = 0.15f )
   {
      measuresTraining.clear();
      measuresTest.clear();

      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );

      // split the database
      const ui32 nbTesting = static_cast<ui32>( testingRatio * measures.size() );
      for ( ui32 n = 0; n < measures.size(); ++n )
      {
         if ( n >= nbTesting )
         {
            ui32 nb = 0;
            Buffer1D<float> f = measures[ n ].toArray();
            for ( ui32 nn = 0; nn < f.size(); ++nn )
               nb += f[ nn ] > 0;
            if ( nb == 5 )
               measuresTraining.push_back( measures[ n ] );
            else
               measuresTest.push_back( measures[ n ] );
         }
         else
         {
            //measuresTraining.push_back( measures[ n ] );
            measuresTest.push_back( measures[ n ] );
         }
      }

      RegionResult::writeMeasures( measuresTraining, DATABASE_MEASURES_CORRECTION );
   }

   // test the position estimation
   void testPositionEstimation()
   {
      std::vector<ErrorReporting> reportingCorrected;

      // prepare the database
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );

      std::vector<RegionResult::Measure> training;
      std::vector<RegionResult::Measure> test;
      createCorrectorDatabase( training, test );

      CorrectPosition2 corrector( training );

      // for all test case, we estimate one ROI position knowing all other ROI and compare to the gound truth
      for ( ui32 n = 0; n < test.size(); ++n )
      {
         core::Buffer1D<float> roiPos = test[ n ].toArray();
         core::Buffer1D<float> roiPosOrig = test[ n ].toArray();

         Image<ui8> preview( std::string( PREVIEW_CASE ) + val2str( test[ n ].id ) + ".bmp" );
         ensure( preview.sizey() == test[ n ].numberOfSlices, "the size must match..." );
         const float spacing = test[ n ].height / test[ n ].numberOfSlices;

         // compute the error
         for ( ui32 roi = 1; roi < NB_CLASS; ++roi )
         {
            if ( roiPosOrig[ roi ] >= 0 )
            {
               corrector.estimate( roiPos, spacing, roi, test[ n ].numberOfSlices );
               if ( roiPos[ roi ] >= 0 )
               {
                  float error = fabs( roiPosOrig[ roi ] - roiPos[ roi ] ) / spacing;
                  reportingCorrected.push_back( ErrorReporting( test[ n ].id, error, roi ) );
               }
            }
         }

         // annotate the image
         corrector.annotateProbability( roiPosOrig, preview, spacing );

         // convert to slice for display...
         for ( ui32 roi = 1; roi < NB_CLASS; ++roi )
            roiPosOrig[ roi ] /= spacing;
         previewLabel( preview, 0, preview.sizex(), roiPosOrig  );
         writeBmp( preview, std::string( PREVIEW_CASE_CORRECTION )+ "-estmator-" + val2str( test[ n ].id )+ ".bmp" );
      }

      analyseResults( reportingCorrected, measures, results );
      RegionResult::writeMeasures( training, DATABASE_MEASURES_CORRECTION );
   }


   // test the position estimation with one ROI having error
   void testPositionEstimationRobustness()
   {
      const double meanBigDeviation     = 0;
      const double varBigDeviation      = 150;
      std::vector<ErrorReporting> reportingCorrected;

      // prepare the database
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );

      std::vector<RegionResult::Measure> training;
      std::vector<RegionResult::Measure> test;
      createCorrectorDatabase( training, test );

      CorrectPosition2 corrector( training );

      // for all test case, we estimate one ROI position knowing all other ROI and compare to the gound truth
      for ( ui32 n = 0; n < test.size(); ++n )
      {
         // generate one ROI with error
         ui32 roiErrorId = rand() % ( NB_CLASS - 1 ) + 1;
         float roiError = (float)generateGaussianDistribution( meanBigDeviation * core::generateSign(), varBigDeviation );

         core::Buffer1D<float> roiPos = test[ n ].toArray();
         core::Buffer1D<float> roiPosOrig = test[ n ].toArray();

         // update the ROI position according the error generated
         roiPos[ roiErrorId ] += roiError;
         roiPosOrig[ roiErrorId ] = roiPos[ roiErrorId ];

         Image<ui8> preview( std::string( PREVIEW_CASE ) + val2str( test[ n ].id ) + ".bmp" );
         ensure( preview.sizey() == test[ n ].numberOfSlices, "the size must match..." );
         const float spacing = test[ n ].height / test[ n ].numberOfSlices;

         // compute the error
         for ( ui32 roi = 1; roi < NB_CLASS; ++roi )
         {
            if ( roi != roiErrorId )
            {
               if ( roiPosOrig[ roi ] >= 0 ) // we don't want to estimate the ROI with error!
               {
                  corrector.estimate( roiPos, spacing, roi, test[ n ].numberOfSlices );
                  if ( roiPos[ roi ] >= 0 )
                  {
                     float error = fabs( roiPosOrig[ roi ] - roiPos[ roi ] ) / spacing;
                     reportingCorrected.push_back( ErrorReporting( test[ n ].id, error, roi ) );
                  }
               }
            }
         }

         // annotate the image
         corrector.annotateProbability( roiPosOrig, preview, spacing );

         // convert to slice for display...
         core::Buffer1D<float> roiPosOrigOrig = test[ n ].toArray();
         for ( ui32 roi = 1; roi < NB_CLASS; ++roi )
         {
            roiPosOrig[ roi ] /= spacing;
            roiPos[ roi ] /= spacing;
            roiPosOrigOrig[ roi ] /= spacing;
         }
         previewLabel( preview, 0, 10, roiPosOrigOrig );
         previewLabel( preview, 10, preview.sizex() / 2, roiPosOrig  );
         ++roiPosOrig[ roiErrorId ];
         previewLabel( preview, 10, preview.sizex() / 2, roiPosOrig  );
         ++roiPosOrig[ roiErrorId ];
         previewLabel( preview, 10, preview.sizex() / 2, roiPosOrig  );

         previewLabel( preview, preview.sizex() / 2, preview.sizex(), roiPos  );

         writeBmp( preview, std::string( PREVIEW_CASE_CORRECTION )+ "-estmator-" + val2str( test[ n ].id )+ ".bmp" );
      }

      analyseResults( reportingCorrected, measures, results );
   }

   void testOutlierDetection()
   {
      srand(6);
      std::vector<ErrorReporting> reportingCorrected;

      // prepare the database
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );

      std::vector<RegionResult::Measure> training;
      std::vector<RegionResult::Measure> test;
      createCorrectorDatabase( training, test );

      CorrectPosition2 corrector( training );

      const double meanBigDeviation     = 0;       // mean of the generated error in mm
      const double varBigDeviation      = 250;     // variance of the generated error in mm
      const double outlierThreshold     = 30;      // threshold indicating it should be detected as outlier, between smallThreshold-outlierThreshold is considered ok to detect it as outlier or miss it...
      const ui32 nbSamplePerTest        = 10;

      Buffer1D<ui32> nbFalsePositives( NB_CLASS );
      Buffer1D<ui32> nbFalseNegatives( NB_CLASS );
      Buffer1D<ui32> nbTruePositives( NB_CLASS );
      Buffer1D<ui32> nbExpectedTP( NB_CLASS );
      ui32 nbCasesWithError = 0;
      ui32 nbCasesFailingCorrecly = 0;
      ui32 nbCasesFailingIncorrecly = 0;

      const double means[ NB_CLASS ] =
      {
         0, 6.89f, 7.29f, 2.65f, 8.95f, 4.74f
      };

      const double vars[ NB_CLASS ] =
      {
         0, sqrt( 9.56f ), sqrt( 6.00f ), sqrt( 4.22f ), sqrt( 9.89f ), sqrt( 5.68f )
      };

      ui32 randseed = 10;
      for ( ui32 testid = 0; testid < test.size(); ++testid )
      {
         for ( ui32 sample = 0; sample < nbSamplePerTest; ++sample )
         {
            srand( randseed++ );
            ui32 roi;
            Buffer1D<float> distances = test[ testid ].toArray();
            Buffer1D<float> distancesOrig = test[ testid ].toArray();

            ui32 nbDist = 0;
            for ( ui32 n = 1; n < NB_CLASS; ++n )
            {
               if ( distances[ n ] > 0 )
                  ++nbDist;
            }
            if ( nbDist < 3 )
               continue;

            while ( 1 )
            {
               // select randomly a ROI
               roi = ( rand() % (NB_CLASS - 1) ) + 1;
               if ( distances[ roi ] > 0 )
                  break;
            }

            // generate a default error to test robustness
            for ( ui32 n = 1; n < NB_CLASS; ++n )
               if ( n != roi && distances[ n ] > 0 )
               {
                  const float err = (float)generateGaussianDistribution( means[ n ] * core::generateSign(), vars[ n ] );
                  distances[ n ] += err;
               }

            // generate an error, and detect the outlier
            float error = (float)generateGaussianDistribution( meanBigDeviation * core::generateSign(), varBigDeviation );
            distances[ roi ] += error;
            for ( ui32 n = 1; n < NB_CLASS; ++n )
            {
               if ( distancesOrig[ n ] > 0 )
               {
                  if ( fabs( distancesOrig[ n ] - distances[ n ] ) > outlierThreshold  )
                  {
                     ++nbCasesWithError;
                  }
               }
            }

            const float spacing = test[ testid ].height / test[ testid ].numberOfSlices;
            //std::set<ui32> outliers = corrector.detectOutliers( distances, spacing, test[ testid ].numberOfSlices );
            std::set<ui32> outliers = corrector.detectOutliers( distances );

            for ( ui32 n = 1; n < NB_CLASS; ++n )
               std::cout << "error roi=" << n << " " << fabs( distancesOrig[ n ] - distances[ n ] ) << std::endl;
            std::cout << "sample:" << sample << " ROI=" << roi << " error=" << error << std::endl;

            bool hasError = false;
            for ( ui32 n = 1; n < NB_CLASS; ++n )
            {
               if ( distancesOrig[ n ] > 0 )
               {
                  if ( fabs( distancesOrig[ n ] - distances[ n ] ) > outlierThreshold  )
                  {
                     ++nbExpectedTP[ n ];
                     hasError = true;
                  }
                  if ( fabs( distancesOrig[ n ] - distances[ n ] ) > outlierThreshold && outliers.find( n ) == outliers.end() )
                     ++nbFalseNegatives[ n ];
               }
            }
            if ( !hasError && outliers.size() )
               ++nbCasesFailingIncorrecly;

            bool failed = false;
            for ( std::set<ui32>::iterator it = outliers.begin(); it != outliers.end(); ++it )
            {
               if ( distancesOrig[ * it ] > 0 )
               {
                  if ( fabs( distancesOrig[ *it ] - distances[ *it ] ) < outlierThreshold  )
                  {
                     ++nbFalsePositives[ *it ];
                     std::cout << "FP:" << *it << " dorig=" << distancesOrig[ *it ] << " d=" << distances[ *it ] << std::endl;
                  }

                  if ( fabs( distancesOrig[ *it ] - distances[ *it ] ) >= outlierThreshold  )
                  {
                     ++nbTruePositives[ *it ];
                     failed = true;
                  }
               } else exit(-1);  // problem!
            }

            if ( failed )
               ++nbCasesFailingCorrecly;
         }
      }

      std::cout << "result:" << std::endl;
      for ( ui32 n = 1; n < NB_CLASS; ++n )
      {
         std::cout << "label:" << n << std::endl
                   << " TP=" << nbTruePositives[ n ] << " expected:" << nbExpectedTP[ n ] << std::endl
                   << " FP=" << nbFalsePositives[ n ] << std::endl
                   << " FN=" << nbFalseNegatives[ n ] << std::endl;
      }

      std::cout << "nbCases=" << test.size() * nbSamplePerTest << std::endl
                << "nbCaseWithOutliers=" << nbCasesWithError << std::endl
                << "case failing as expected=" << nbCasesFailingCorrecly << std::endl
                << "FP case=" << nbCasesFailingIncorrecly << std::endl;
                

   }

   // measure the detection rate of an incorrect configuration with at least 3 ROI
   void testOutlierDetection0Wrong()
   {
      std::vector<ErrorReporting> reportingCorrected;

      // prepare the database
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );

      std::vector<RegionResult::Measure> training;
      std::vector<RegionResult::Measure> test;
      createCorrectorDatabase( training, test );

      CorrectPosition2 corrector( training );

      const double meanBigDeviation     = 0;       // mean of the generated error in mm
      const double varBigDeviation      = 250;     // variance of the generated error in mm
      const double smallThreshold       = 30;      // threshold indicating we don't want any outlier detection within this range
      const double outlierThreshold     = 30;      // threshold indicating it should be detected as outlier, between smallThreshold-outlierThreshold is considered ok to detect it as outlier or miss it...
      const ui32 nbSamplePerTest        = 50;

      ui32 nbCasesWithError = 0;
      ui32 nbCasesFailingCorrecly = 0;
      ui32 nbCasesFailingIncorrecly = 0;

      const double means[ NB_CLASS ] =
      {
         0, 6.89f, 7.29f, 2.65f, 8.95f, 4.74f
      };

      const double vars[ NB_CLASS ] =
      {
         0, sqrt( 9.56f ), sqrt( 6.00f ), sqrt( 4.22f ), sqrt( 9.89f ), sqrt( 5.68f )
      };

      for ( ui32 testid = 0; testid < test.size(); ++testid )
      {
         for ( ui32 sample = 0; sample < nbSamplePerTest; ++sample )
         {
            ui32 roi;
            Buffer1D<float> distances = test[ testid ].toArray();
            Buffer1D<float> distancesOrig = test[ testid ].toArray();

            ui32 nbDist = 0;
            for ( ui32 n = 1; n < NB_CLASS; ++n )
            {
               if ( distances[ n ] > 0 )
                  ++nbDist;
            }
            if ( nbDist < 3 )
               continue;

            while ( 1 )
            {
               // select randomly a ROI
               roi = ( rand() % (NB_CLASS - 1) ) + 1;
               if ( distances[ roi ] > 0 )
                  break;
            }

            // generate a default error to test robustness
            for ( ui32 n = 1; n < NB_CLASS; ++n )
               if ( n != roi && distances[ n ] > 0 )
               {
                  const float err = (float)generateGaussianDistribution( means[ n ] * core::generateSign(), vars[ n ] );
                  std::cout << " add err=" << err << std::endl;
                  distances[ n ] += err;
               }

            // generate an error, and detect the outlier
            float error = (float)generateGaussianDistribution( meanBigDeviation * core::generateSign(), varBigDeviation );
            distances[ roi ] += error;
            for ( ui32 n = 1; n < NB_CLASS; ++n )
            {
               if ( distancesOrig[ n ] > 0 )
               {
                  if ( fabs( distancesOrig[ n ] - distances[ n ] ) > outlierThreshold  )
                  {
                     ++nbCasesWithError;
                  }
               }
            }

            std::cout << "sample:" << sample << " ROI=" << roi << " error=" << error << std::endl;
            const float spacing = test[ testid ].height / test[ testid ].numberOfSlices;
            //std::set<ui32> outliers = corrector.detectOutliers( distances, spacing, test[ testid ].numberOfSlices );
            bool hasOutliers = corrector.isIncorrectConfiguration( distances );

            bool hasError = false;
            for ( ui32 n = 1; n < NB_CLASS; ++n )
            {
               if ( distancesOrig[ n ] > 0 )
               {
                  if ( fabs( distancesOrig[ n ] - distances[ n ] ) > outlierThreshold  )
                  {
                     hasError = true;
                  }
               }
            }
            if ( !hasError && hasOutliers )
               ++nbCasesFailingIncorrecly;

            if ( hasError && hasOutliers )
               ++nbCasesFailingCorrecly;
         }
      }

      std::cout << "result:" << std::endl;
      std::cout << "nbCases=" << test.size() * nbSamplePerTest << std::endl
                << "nbCaseWithOutliers=" << nbCasesWithError << std::endl
                << "case failing as expected=" << nbCasesFailingCorrecly << std::endl
                << "FP case=" << nbCasesFailingIncorrecly << std::endl;
   }

   void trainOutlierDetection()
   {
      // prepare the database
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<RegionResult::Measure> test = RegionResult::readMeasures( DATABASE_MEASURES );

      
      CorrectPosition3 corrector;
      corrector.learn( test );
   }

   void testOutlierDetection0Wrong_3()
   {
      std::vector<ErrorReporting> reportingCorrected;

      // prepare the database
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      std::vector<RegionResult::Measure> measures = RegionResult::readMeasures( DATABASE_MEASURES );

      CorrectPosition4 corrector;
      corrector.learn( measures );

      std::vector<RegionResult::Measure> training;
      std::vector<RegionResult::Measure> test;
      createCorrectorDatabase( training, test );


      const double meanBigDeviation     = 0;       // mean of the generated error in mm
      const double varBigDeviation      = 200;     // variance of the generated error in mm
      const double outlierThreshold     = 30;      // threshold indicating it should be detected as outlier, between smallThreshold-outlierThreshold is considered ok to detect it as outlier or miss it...
      const ui32 nbSamplePerTest        = 100;

      ui32 nbCasesWithError = 0;
      ui32 nbCasesFailingCorrecly = 0;

      const double means[ NB_CLASS ] =
      {
         0, 6.89f, 7.29f, 2.65f, 8.95f, 4.74f
      };

      const double vars[ NB_CLASS ] =
      {
         0, sqrt( 9.56f ), sqrt( 6.00f ), sqrt( 4.22f ), sqrt( 9.89f ), sqrt( 5.68f )
      };

      ui32 fp = 0;
      for ( ui32 testid = 0; testid < test.size(); ++testid )
      {
         for ( ui32 sample = 0; sample < nbSamplePerTest; ++sample )
         {
            ui32 roi;
            Buffer1D<float> distances = test[ testid ].toArray();
            Buffer1D<float> distancesOrig = test[ testid ].toArray();

            while ( 1 )
            {
               // select randomly a ROI
               roi = ( rand() % (NB_CLASS - 1) ) + 1;
               if ( distances[ roi ] > 0 )
                  break;
            }

            // generate a default error to test robustness
            for ( ui32 n = 1; n < NB_CLASS; ++n )
               if ( n != roi && distances[ n ] > 0 )
               {
                  const float err = (float)generateGaussianDistribution( means[ n ] * core::generateSign(), vars[ n ] );
                  std::cout << " add err=" << err << std::endl;
                  distances[ n ] += err;
               }

            // generate an error, and detect the outlier
            float error = (float)generateGaussianDistribution( meanBigDeviation * core::generateSign(), varBigDeviation );
            distances[ roi ] += error;
            

            std::cout << "sample:" << sample << " ROI=" << roi << " error=" << error << std::endl;
            const float spacing = test[ testid ].height / test[ testid ].numberOfSlices;
            bool outliers = corrector.detectOutliers( distances );

            float maxError = 0;
            for ( ui32 n = 1; n < NB_CLASS; ++n )
            {
               maxError = std::max( maxError, fabs( distancesOrig[ n ] - distances[ n ] ) );
            }

            if ( maxError > outlierThreshold )
            {
               ++nbCasesWithError;
               if ( outliers )
                  ++nbCasesFailingCorrecly;
            } else {
               if ( outliers )
                  ++fp;
            }
         }
      }

      std::cout << "expected failures:" << nbCasesWithError << " failure correct=" << nbCasesFailingCorrecly << std::endl;
      std::cout << "fp=" << fp << std::endl;
   }
}; 

TESTER_TEST_SUITE(TestRegion);

// input: cases, mf2 volumes, output: haar features, normalization parameters, learning database
//TESTER_TEST(createDatasets);

// input: cases, mf2 volumes, output: XZ slice in preview directory // this is needed for all volume-database related visualization
//TESTER_TEST(createPreview);

// input: cases, mf2 volumes, output: a database for all volumes of all slices
//TESTER_TEST(createVolumeDatabase);

// input: cases, haar features, normalization parameters, learning database, output: svm
//TESTER_TEST(learnSvm);
//TESTER_TEST(learnKnn);
//TESTER_TEST(learnMlp);

// input: SVM model, validation-cases, validation volumes mf2
//TESTER_TEST(testValidationDataSvm);

// test the ROI position estimation
//TESTER_TEST(testPositionEstimation); // input: measures, results, output: the template database
//TESTER_TEST(testPositionEstimationRobustness)   // input: measures, results,

// test the ROI outlier detection // input:template database
//TESTER_TEST(testOutlierDetection0Wrong); // measure the incorrect configuration detection rate
//TESTER_TEST(testOutlierDetection); 


//
// deprecated:
//
//TESTER_TEST(extractXZFullResolution);
//TESTER_TEST(testCenter);
//TESTER_TEST(testSelectedTemplate);

// with SVM
//TESTER_TEST(trainOutlierDetection);
//TESTER_TEST(testOutlierDetection0Wrong_3);   // only 0 outlier

// for registration based method: display registration findings
//TESTER_TEST(registrationExport);
TESTER_TEST_SUITE_END();