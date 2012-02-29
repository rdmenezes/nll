#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;


namespace nll
{
namespace algorithm
{
   
}
}

class TestBoosting
{
public:
   void testStumpInf1()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::StumpFactory<Database>    StumpFactory;

      Database dat;
      dat.add( Database::Sample( core::make_vector<float>( -500 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -400 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -300 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 50 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 60 ), 1, Database::Sample::LEARNING ) );

      std::vector<float> w = makeWeights( dat );

      StumpFactory stumpFactory( 1.0 );
      std::shared_ptr<algorithm::WeakClassifierStump<Database>> s1 = stumpFactory.create();
      float error = s1->learn( dat, w );
      TESTER_ASSERT( error <= 0 );

      ui32 nbGood;
      computeAsStumpResults( dat, 0, s1->getThreshold(), s1->isInfReturningZeroClass(), nbGood );
      TESTER_ASSERT( nbGood == dat.size() );

      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         TESTER_ASSERT( s1->test( dat[ n ].input ) == dat[ n ].output );
      }
   }

   void testStumpInf2()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::StumpFactory<Database>    StumpFactory;

      Database dat;
      dat.add( Database::Sample( core::make_vector<float>( -500 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -400 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -300 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 50 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 60 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 50 ), 0, Database::Sample::LEARNING ) );

      std::vector<float> w = makeWeights( dat );

      StumpFactory stumpFactory( 1.0 );
      std::shared_ptr<algorithm::WeakClassifierStump<Database>> s1 = stumpFactory.create();
      s1->learn( dat, w );

      ui32 nbGood;
      computeAsStumpResults( dat, 0, s1->getThreshold(), s1->isInfReturningZeroClass(), nbGood );
      TESTER_ASSERT( nbGood == 5 );
   }

   void testStumpInf3()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::StumpFactory<Database>    StumpFactory;

      Database dat;
      dat.add( Database::Sample( core::make_vector<float>( -550 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -500 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -400 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -300 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -100 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 60 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 50 ), 0, Database::Sample::LEARNING ) );

      std::vector<float> w = makeWeights( dat );

      StumpFactory stumpFactory( 1.0 );
      std::shared_ptr<algorithm::WeakClassifierStump<Database>> s1 = stumpFactory.create();
      s1->learn( dat, w );

      ui32 nbGood;
      computeAsStumpResults( dat, 0, s1->getThreshold(), s1->isInfReturningZeroClass(), nbGood );
      TESTER_ASSERT( nbGood == 5 );
   }

   void testStumpSup1()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::StumpFactory<Database>    StumpFactory;

      Database dat;
      dat.add( Database::Sample( core::make_vector<float>( -500 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -400 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -300 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 50 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 60 ), 0, Database::Sample::LEARNING ) );

      std::vector<float> w = makeWeights( dat );

      StumpFactory stumpFactory( 1.0 );
      std::shared_ptr<algorithm::WeakClassifierStump<Database>> s1 = stumpFactory.create();
      s1->learn( dat, w );

      ui32 nbGood;
      computeAsStumpResults( dat, 0, s1->getThreshold(), s1->isInfReturningZeroClass(), nbGood );
      TESTER_ASSERT( nbGood == dat.size() );
   }

   void testBoosting1()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>   Adaboost;
      typedef algorithm::StumpFactory<Database>    StumpFactory;

      Database dat;
      dat.add( Database::Sample( core::make_vector<float>( -550 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -500 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -400 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -300 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -100 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 60 ), 1, Database::Sample::LEARNING ) );
      //dat.add( Database::Sample( core::make_vector<float>( 50 ), 0, Database::Sample::LEARNING ) );

      Adaboost classifier;
      StumpFactory stumpFactory( 1 );
      classifier.learn( dat, 20, stumpFactory );

      std::cout << "ERROR=" << getTrainingError( dat, classifier ) << std::endl;
   }

private:
   template <class Database>
   static std::vector<float> makeWeights( const Database& dat )
   {
      return std::vector<float> ( dat.size(), 1.0f / dat.size() );
   }

   template <class Database>
   float getTrainingError( const Database& dat, algorithm::AdaboostBasic<Database>& c )
   {
      ui32 nbErrors = 0;
      Database learning = core::filterDatabase( dat, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );
      for ( ui32 n = 0; n < learning.size(); ++n )
      {
         if ( c.test( learning[ n ].input ) != learning[ n ].output )
            ++nbErrors;
      }

      return static_cast<float>( nbErrors ) / learning.size();
   }

   // simulate a sump and compute its expected results on the database
   template <class Database>
   static void computeAsStumpResults( const Database& dat, ui32 feature, float threshold, bool isClassInfZero, ui32& nbRight )
   {
      nbRight = 0;
      const ui32 nbSamples = dat.size();
      for ( ui32 n = 0; n < nbSamples; ++n )
      {
         ui32 classout;
         if ( dat[ n ].input[ feature ] < threshold )
         {
            if ( isClassInfZero )
            {
               classout = 0;
            } else {
               classout = 1;
            }
         } else {
            if ( isClassInfZero )
            {
               classout = 1;
            } else {
               classout = 0;
            }
         }

         if ( dat[ n ].output == classout )
         {
            ++nbRight;
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestBoosting);
/*
TESTER_TEST(testStumpInf1);
TESTER_TEST(testStumpInf2);
TESTER_TEST(testStumpInf3);
TESTER_TEST(testStumpSup1);
*/
TESTER_TEST(testBoosting1);
TESTER_TEST_SUITE_END();
#endif