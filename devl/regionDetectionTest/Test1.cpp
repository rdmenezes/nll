#include <tester/register.h>
#include "globals.h"
#include <regionDetection/features.h>
#include <regionDetection/read-result.h>
#include <regionDetection/test.h>

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;
using namespace nll::detect;

ui8 colors[ 4 ][ 3 ] = 
{
   { 0, 0, 0},
   { 255, 255, 255 },
   { 255, 0, 0 },
   { 0, 255, 0 }
};

struct TestRegion
{
   /*
   void normalizeImageTest()
   {
      for ( ui32 n = 1; n < NBCASES; ++n )
      {
         std::cout << "case=" << n << std::endl;
         Volume volume1;
         bool loaded = loadSimpleFlatFile( datasets[ n ], volume1 );
         TESTER_ASSERT( loaded );
         
         std::vector< Image<ui8> > images = normalizeImage( volume1 );


         writeBmp( images[ 0 ], std::string( "c:/tmp/mpr-1-" ) + val2str( n ) + ".bmp" );
         writeBmp( images[ 1 ], std::string( "c:/tmp/mpr-2-" ) + val2str( n ) + ".bmp" );
      }
   }

   void createSourceDataset()
   {
      RegionResult::generateSourceDatabase( DATA_PATH "cases.txt", DATABASE_SOURCE );
   }

   void createPcaDatabase()
   {
      RegionResult::Database dat;
      dat.read( DATABASE_SOURCE );

      typedef RegionResult::Database::Sample Sample;
      typedef Sample::Input                  Point;
      typedef Buffer1D<Point>                Points;

      // create the PCA input
      Points points( dat.size() );
      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         points[ n ] = dat[n].input;
      }
      PrincipalComponentAnalysis<Points> pca( REGION_DETECTION_PCA_SIZE );
      pca.compute( points );
      pca.write( PCA_ENGINE_PATH );

      // transform the database
      RegionResult::Database datPca;
      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         Sample s = dat[ n ];
         s.input = pca.process( s.input );
         datPca.add( s );
      }

      datPca.write( DATABASE_PCA );
   }

   void testLearning()
   {
      srand(time(0));
      RegionResult::Database dat;
      dat.read( DATABASE_PCA );
      std::cout << "dat loaded" << std::endl;

      std::cout << "dim=" << dat[0].output.size() << std::endl;

      typedef Mlp<FunctionSimpleDifferenciableSigmoid> Mlp;
      Mlp mlp( make_vector<ui32>( REGION_DETECTION_PCA_SIZE, 15, 3 ) );

      StopConditionMlpThreshold stopCondition( 1, -1, -1, -1 );
      mlp.learn( dat, stopCondition, 0.05f );
      mlp.write( NN_ENGINE_PATH );
   }


   void createPreview()
   {
      for ( ui32 n = 52; n < NBCASES; ++n )
      {
         std::cout << "generate preview:" << n << std::endl;

         Volume volume1;
         bool loaded = loadSimpleFlatFile( datasets[ n ], volume1 );
         TESTER_ASSERT( loaded );

         Image<ui8> xz, yz;
         
         extractMpr( volume1, xz, yz );


         writeBmp( xz, std::string( PREVIEW_CASE ) + val2str( n ) + ".bmp" );
      }
   }


   void createPreviewMark()
   {
      std::cout << "reading engines..." << std::endl;

      typedef Mlp<FunctionSimpleDifferenciableSigmoid> Mlp;
      Mlp mlp;
      mlp.read( NN_ENGINE_PATH );
      
      RegionResult::Database dat;
      dat.read( DATABASE_PCA );

      const ui8 green[ 3 ] = { 0, 255, 0 };

      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         std::cout << "generate results:" << n << " " << string_from_Buffer1D( dat[ n ].debug ) << std::endl;

         Image<ui8> mpr;
         readBmp( mpr, std::string( PREVIEW_CASE ) + val2str( n ) + ".bmp" );
         const core::Buffer1D<double>& result = mlp.propagate( dat[ n ].input );
         
         ui32 neck = static_cast<ui32>( mpr.sizey() * result[ 0 ] );
         ui32 neckg = static_cast<ui32>( mpr.sizey() * dat[ n ].output[ 0 ] );
         if ( neckg >= mpr.sizey() )
            neckg = mpr.sizey() - 1;
         for ( ui32 nn = 0; nn < mpr.sizex(); ++nn )
         {
            mpr.setPixel( nn, neck, Image<ui8>::white() );
            mpr.setPixel( nn, neckg, green );
         }

         ui32 heart = static_cast<ui32>( mpr.sizey() * result[ 1 ] );
         ui32 heartg = static_cast<ui32>( mpr.sizey() * dat[ n ].output[ 1 ] );
         if ( heartg >= mpr.sizey() )
            heartg = mpr.sizey() - 1;
         for ( ui32 nn = 0; nn < mpr.sizex(); ++nn )
         {
            mpr.setPixel( nn, heart, Image<ui8>::red() );
            mpr.setPixel( nn, heartg, green );
         }

         ui32 lung = static_cast<ui32>( mpr.sizey() * result[ 2 ] );
         ui32 lungg = static_cast<ui32>( mpr.sizey() * dat[ n ].output[ 2 ] );
         if ( lungg >= mpr.sizey() )
            lungg = mpr.sizey() - 1;
         for ( ui32 nn = 0; nn < mpr.sizex(); ++nn )
         {
            mpr.setPixel( nn, lung, Image<ui8>::blue() );
            mpr.setPixel( nn, lungg, green );
         }

         writeBmp( mpr, PREVIEW_CASE_MARK + val2str( n ) + ".bmp" );
      }
   }
   */

   void testExtractSlice()
   {
      /*
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
      }*/
   }

   void createDatasets()
   {
      //RegionResult::generateSourceDatabase( CASES_DESC, DATABASE_SOURCE );
      RegionResult::generateFeatureDatabase();
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

   void learnMlp()
   {
      typedef Buffer1D<double>      Point;
      typedef ClassifierMlp<Point>  Classifier;
      typedef Classifier::Database  Database;

      Database haarDatabaseNormalized;
      haarDatabaseNormalized.read( NORMALIZED_HAAR );

      Classifier classifier;
      classifier.learn( haarDatabaseNormalized, make_buffer1D<double>( 20, 0.05, 15 ) );
      classifier.test( haarDatabaseNormalized );
   }

   void learnSvm()
   {
      typedef Buffer1D<double>      Point;
      typedef ClassifierSvm<Point>  Classifier;
      typedef Classifier::Database  Database;

      Database haarDatabaseNormalized;
      haarDatabaseNormalized.read( NORMALIZED_HAAR );

      Classifier classifier;
      classifier.learn( haarDatabaseNormalized, make_buffer1D<double>( 0.002, 100 ) );
      classifier.test( haarDatabaseNormalized );

      //testResult( &classifier );
      testResultVolumeDatabase( &classifier );
   }

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
   void testResultVolumeDatabase( Classifier< Buffer1D<double> >* classifier )
   {
      typedef Buffer1D<ui8>         Point;
      typedef ClassifierMlp<Point>  Classifier;
      typedef Classifier::Database  Database;
      

      TestVolume test( classifier, HAAR_FEATURES, PREPROCESSING_HAAR );
      std::vector<RegionResult::Result> results = RegionResult::readResults( CASES_DESC );
      for ( ui32 n = 0; n < results.size(); ++n )
      {
         std::cout << "test case database:" << n << std::endl;
         Database dat;
         dat.read( DATABASE_FULL_CASE( results[ n ].id ) );
         Image<ui8> mprz;
         readBmp( mprz, std::string( PREVIEW_CASE ) + val2str( results[ n ].id ) + ".bmp" );

         
         std::cout << "size im=" << mprz.sizey() << " dat=" << dat.size() << std::endl;

         // export ground truth
         std::cout << "write ground truth" << std::endl;
         for ( ui32 nnn = std::min<ui32>( mprz.sizex(), 10 ); nnn < std::min<ui32>( mprz.sizex(), 20 ); ++nnn )
         {
            ui8* p = mprz.point( nnn, (ui32)results[ n ].neckStart );
            p[ 0 ] = colors[ 1 ][ 0 ];
            p[ 1 ] = colors[ 1 ][ 1 ];
            p[ 2 ] = colors[ 1 ][ 2 ];

            p = mprz.point( nnn, (ui32)results[ n ].heartStart );
            p[ 0 ] = colors[ 2 ][ 0 ];
            p[ 1 ] = colors[ 2 ][ 1 ];
            p[ 2 ] = colors[ 2 ][ 2 ];

            p = mprz.point( nnn, (ui32)results[ n ].lungStart );
            p[ 0 ] = colors[ 3 ][ 0 ];
            p[ 1 ] = colors[ 3 ][ 1 ];
            p[ 2 ] = colors[ 3 ][ 2 ];
         }

         std::cout << "export result" << std::endl;
         for ( ui32 nn = 0; nn < dat.size(); ++nn )
         {
            Image<ui8> i( dat[ nn ].input, REGION_DETECTION_SOURCE_IMG_X, REGION_DETECTION_SOURCE_IMG_Y, 1 );
            Buffer1D<double> f = test.getFeatures( i );
            ui32 r = classifier->test( f );


            // mark result
            for ( ui32 nnn = 0; nnn < std::min<ui32>( mprz.sizex(), 10 ); ++nnn )
            {
               ui8* p = mprz.point( nnn, nn );
               p[ 0 ] = colors[ r ][ 0 ];
               p[ 1 ] = colors[ r ][ 1 ];
               p[ 2 ] = colors[ r ][ 2 ];
            }
         }

         writeBmp( mprz, std::string( PREVIEW_CASE_MARK ) + val2str( results[ n ].id ) + ".bmp" );
      }
   }

   // for all test, create a XZ MPR displaying the results
   void testResult( Classifier< Buffer1D<double> >* classifier )
   {
      TestVolume test( classifier, HAAR_FEATURES, PREPROCESSING_HAAR );
      for ( ui32 n = static_cast<ui32>( NBCASES * 0.8 ); n < NBCASES; ++n )
      {
         Volume v;
         bool loaded = loadSimpleFlatFile( datasets[ n ], v );
         TESTER_ASSERT( loaded );

         const std::string exportNmae = "c:/tmp/result-" + val2str( n ) + ".bmp";
         Image<ui8> i = test.exportTest( v );
/*
         ui8 colors[ 4 ][ 3 ] = 
         {
            { 0, 0, 0},
            { 255, 255, 255 },
            { 255, 0, 0 },
            { 0, 255, 0 }
         };

         ui32 min = std::min<ui32>( sliceTfm.sizex(), 10 );
         ui32 max = std::min<ui32>( sliceTfm.sizex(), 20 );
         for ( ui32 n = min; n < max; ++n )
         {
            ui8* p = i.point( nn, n );
            p[ 0 ] = colors[ r[ n ] ][ 0 ];
            p[ 1 ] = colors[ r[ n ] ][ 1 ];
            p[ 2 ] = colors[ r[ n ] ][ 2 ];
         }
         */
         writeBmp( i, exportNmae );
      }
   }
};

TESTER_TEST_SUITE(TestRegion);
//TESTER_TEST(createDatasets);
TESTER_TEST(learnSvm);
//TESTER_TEST(createVolumeDatabase);
//TESTER_TEST(createPreview);

//TESTER_TEST(learnMlp);
//TESTER_TEST(testExtractSlice);

/*
//TESTER_TEST(normalizeImageTest);
//TESTER_TEST(createSourceDataset);
TESTER_TEST(createPcaDatabase);
TESTER_TEST(testLearning);
//TESTER_TEST(createPreview);
TESTER_TEST(createPreviewMark);
*/
TESTER_TEST_SUITE_END();
