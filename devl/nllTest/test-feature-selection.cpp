#include <nll/nll.h>
#include <tester/register.h>

class TestNllFeatureSelection
{
   typedef nll::core::Buffer1D<nll::f32>        Point;
   typedef nll::algorithm::ClassifierMlp<Point> CMlp;
   typedef CMlp::BaseClassifier                 Classifier;
   typedef CMlp::Database                       Database;

public:
   Database createDatabase() const
   {
      Classifier::Database dat;
      for ( float nx = 0; nx <= 1; nx += 0.01f )
         for ( float ny = 0; ny <= 1; ny += 0.01f )
         {
            nll::ui32 c;
            if ( nx <= 0 && ny <= 0)
               c = 0;
            if ( nx > 0 && ny <= 0)
               c = 1;
            if ( nx <= 0 && ny > 0)
               c = 2;
            if ( nx > 0 && ny > 0)
               c = 3;
            nll::f32 r = ( (nll::f32)rand() ) / RAND_MAX - 0.5;
            dat.add(Classifier::Database::Sample(nll::core::make_buffer1D<nll::f32>(nx, ny, r), c, Classifier::Database::Sample::LEARNING ));
            dat.add(Classifier::Database::Sample(nll::core::make_buffer1D<nll::f32>(nx, ny, r), c, Classifier::Database::Sample::TESTING ));
         }
      return dat;
   }
   
   /*
   void testNllFeatureSelectionPreprocessingUnitBestFirst()
   {
      std::cout << "START" << std::endl;
      typedef nll::core::Buffer1D<float>        Point;
      typedef nll::algorithm::ClassifierMlp<Point> Classifier;

      typedef nll::preprocessing::PreprocessingRawSelect<Point>            Preprocessing1;
      typedef TYPELIST_1(Preprocessing1, Classifier)                       PreprocessingPipeline;

      Classifier::Database dat = createDatabase();
      Classifier* classifier = new Classifier();
      nll::core::Buffer1D<nll::f64> parameters = nll::core::make_buffer1D<nll::f64>(8, 0.5, 1.5);

      nll::core::Buffer1D<bool> selectBuf( 3 );
      selectBuf[ 0 ] = true;
      selectBuf[ 1 ] = true;
      selectBuf[ 2 ] = true;

      Preprocessing1::Options options( 1 );
      options[ 0 ] = Preprocessing1::BEST_FIRST;

      Preprocessing1 select( 2, selectBuf, options );
      PreprocessingPipeline pipeline( select, classifier );

      nll::preprocessing::PreprocessingOptimizationScheme scheme;
      scheme.add( select.getId() );

      std::cout << "optimize start" << std::endl;
      pipeline.optimize( dat, parameters, scheme );
      std::cout << "optimize end" << std::endl;

      Point p( 3 );
      p[ 0 ] = 1;
      p[ 1 ] = 2;
      p[ 2 ] = 3;

      Point presult = pipeline.process( p );
      TESTER_ASSERT( nll::core::equal<nll::f32>(presult[ 0 ], 1) && nll::core::equal<nll::f32>(presult[ 1 ], 2) );
   }*/

   void testNllFeatureSelectionBestFirst()
   {
      srand( 0 );
      Classifier::Database dat = createDatabase();
      Classifier* classifier = new CMlp();
      nll::core::Buffer1D<nll::f64> parameters = nll::core::make_buffer1D<nll::f64>(8, 0.5, 1.5);

      nll::algorithm::FeatureSelectionWrapper<Point>* fs = new nll::algorithm::FeatureSelectionBestFirst<Point>( 2 );
      nll::core::Buffer1D<bool> result = fs->compute( classifier, parameters, dat );
      TESTER_ASSERT( result[ 0 ] && result[ 1 ] && !result[ 2 ] );
   }

   void testNllFeatureSelection()
   {
      Classifier::Database dat = createDatabase();
      Classifier* classifier = new CMlp();
      nll::core::Buffer1D<nll::f64> parameters = nll::core::make_buffer1D<nll::f64>(8, 0.5, 1.5);

      nll::algorithm::FeatureSelectionWrapper<Point>* fs = new nll::algorithm::FeatureSelectionGeneticAlgorithm<Point>( 0.6, 5, 2, 2 );
      nll::core::Buffer1D<bool> result = fs->compute( classifier, parameters, dat );
      TESTER_ASSERT( result[ 0 ] && result[ 1 ] && !result[ 2 ] );
   }

   void testNllFeatureSelectionPearson()
   {
      Classifier::Database dat = createDatabase();
      nll::algorithm::FeatureSelectionFilter<Point>* fs = new nll::algorithm::FeatureSelectionFilterPearson<Point>( 2 );
      nll::core::Buffer1D<bool> result = fs->compute( dat );
      TESTER_ASSERT( result[ 0 ] && result[ 1 ] && !result[ 2 ] );
   }

   void testNllRelieff()
   {
      Classifier::Database dat = createDatabase();
      typedef nll::algorithm::Relieff<Classifier::Database::Sample::Input> Relieff;
      Relieff test;
      Relieff::FeatureRank rank = test.process( dat, 2 );

      TESTER_ASSERT( rank[ 0 ] > rank[ 2 ] );
      TESTER_ASSERT( rank[ 1 ] > rank[ 2 ] );
   }

   void testNllFeatureSelectionRelieff()
   {
      Classifier::Database dat = createDatabase();
      nll::algorithm::FeatureSelectionFilter<Point>* fs = new nll::algorithm::FeatureSelectionFilterRelieff<Point>( 2 );
      nll::core::Buffer1D<bool> result = fs->compute( dat );
      TESTER_ASSERT( result[ 0 ] && result[ 1 ] && !result[ 2 ] );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllFeatureSelection);
# ifndef DONT_RUN_VERY_SLOW_TEST
#  ifndef DONT_RUN_SLOW_TEST
//TESTER_TEST(testNllFeatureSelectionPreprocessingUnitBestFirst);
TESTER_TEST(testNllFeatureSelectionBestFirst);
//TESTER_TEST(testNllFeatureSelection);
#  endif
# endif
//TESTER_TEST(testNllFeatureSelectionPearson);
//TESTER_TEST(testNllFeatureSelectionRelieff);
//TESTER_TEST(testNllRelieff);
TESTER_TEST_SUITE_END();
#endif
