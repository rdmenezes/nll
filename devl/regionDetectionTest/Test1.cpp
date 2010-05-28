#include <tester/register.h>
#include "globals.h"
#include <regionDetection/compute-barycentre.h>
#include <regionDetection/extract-mpr.h>
#include <regionDetection/create-features.h>
#include <regionDetection/read-result.h>

typedef nll::imaging::VolumeSpatial<float>         Volume;
typedef nll::imaging::LookUpTransformWindowingRGB  Lut;

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;
using namespace nll::utility;

struct TestRegion
{
   void normalizeImageTest()
   {
      for ( ui32 n = 1; n < 52; ++n )
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
      RegionResult::generateSourceDatabase( DATA_PATH "cases.txt", DATA_PATH "source-data.dat" );
   }

   void createPcaDatabase()
   {
      RegionResult::Database dat;
      dat.read( DATA_PATH "source-data.dat" );

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
      pca.write( DATA_PATH "pca-engine.dat" );

      // transform the database
      RegionResult::Database datPca;
      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         Sample s = dat[ n ];
         s.input = pca.process( s.input );
         datPca.add( s );
      }

      datPca.write( DATA_PATH "pca-data.dat" );
   }

   void testLearning()
   {
      srand(time(0));
      RegionResult::Database dat;
      dat.read( DATA_PATH "pca-data.dat" );
      std::cout << "dat loaded" << std::endl;

      std::cout << "dim=" << dat[0].output.size() << std::endl;

      typedef Mlp<FunctionSimpleDifferenciableSigmoid> Mlp;
      Mlp mlp( make_vector<ui32>( REGION_DETECTION_PCA_SIZE, 10, 3 ) );

      StopConditionMlpThreshold stopCondition( 3, -1, -1, -1 );
      mlp.learn( dat, stopCondition, 0.2f );

      // test
      Volume volume1;
      bool loaded = loadSimpleFlatFile( "D:/Devel/sandbox/nllTest/data/medical/ct.mf2", volume1 );
      TESTER_ASSERT( loaded );

      Buffer1D<double> points = createFeatures( volume1 );

      PrincipalComponentAnalysis< Buffer1D< Buffer1D<double> > > pca( 1 );
      pca.read( DATA_PATH "pca-engine.dat" );

      Buffer1D<double> input = pca.process( points );
      const core::Buffer1D<double>& result = mlp.propagate( input );
      std::cout << "result=";
      result.print( std::cout );

      vector3f head = volume1.indexToPosition( vector3f( volume1.size()[ 0 ] / 2, volume1.size()[ 1 ] / 2, ( result[ 0 ] ) * volume1.size()[ 2 ] ) );
      vector3f heart = volume1.indexToPosition( vector3f( volume1.size()[ 0 ] / 2, volume1.size()[ 1 ] / 2, ( result[ 1 ] ) * volume1.size()[ 2 ] ) );
      vector3f lung = volume1.indexToPosition( vector3f( volume1.size()[ 0 ] / 2, volume1.size()[ 1 ] / 2, ( result[ 2 ] ) * volume1.size()[ 2 ] ) );
      head.print( std::cout );
      heart.print( std::cout );
      lung.print( std::cout );
   }
};

TESTER_TEST_SUITE(TestRegion);
//TESTER_TEST(normalizeImageTest);
TESTER_TEST(createSourceDataset);
TESTER_TEST(createPcaDatabase);
TESTER_TEST(testLearning);
TESTER_TEST_SUITE_END();
