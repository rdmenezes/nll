#include <tester/register.h>
#include "globals.h"
#include <regionDetection/features.h>
#include <regionDetection/read-result.h>
#include <regionDetection/test.h>

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;
using namespace nll::detect;

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
      classifier.learn( haarDatabaseNormalized, make_buffer1D<double>( 0.005, 100 ) );
      classifier.test( haarDatabaseNormalized );

      testResult( &classifier );
   }

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
//TESTER_TEST(testExtractSlice);
TESTER_TEST(createDatasets);
//TESTER_TEST(learnMlp);
TESTER_TEST(learnSvm);

/*
//TESTER_TEST(normalizeImageTest);
//TESTER_TEST(createSourceDataset);
TESTER_TEST(createPcaDatabase);
TESTER_TEST(testLearning);
//TESTER_TEST(createPreview);
TESTER_TEST(createPreviewMark);
*/
TESTER_TEST_SUITE_END();
