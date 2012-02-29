#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

#define NLL_SECURE

namespace nll
{
namespace algorithm
{
   

   /**
    @brief Stump factory
    */
   template <class DatabaseT>
   class StumpFactory
   {
   public:
      StumpFactory( float nbBinsRatio = 0.5 ) : _nbBinsRatio( nbBinsRatio )
      {}

      WeakClassifierStump<DatabaseT> create() const
      {
         return WeakClassifierStump<DatabaseT>( _nbBinsRatio );
      }

   private:
      float _nbBinsRatio;
   };

   /**
    @brief basic Adaboost implementation for binary classification only

    This version of the algorithm is using resampling of the training data instead of the original version
    (i.e., a specific weak learner using the Dt distribution must be used to weight the learner)

    @note see http://www.site.uottawa.ca/~stan/csi5387/boost-tut-ppr.pdf for implementation details
          see http://www.face-rec.org/algorithms/Boosting-Ensemble/decision-theoretic_generalization.pdf for full proof of the algorihm
    @param WeakClassifier it must be an instance of <code>Classifier</code>
    */
   template <class Database>
   class AdaboostBasic
   {
   public:
      /**
       @brief train the adaboost classifier
       @param data the database to learn. It is assumed binary classification problems
       @param nbWeakClassifiers the number of weakclassifier composing the strong classifier
       @param factory the factory creating a weak classifier
       @note it must be ensured the classification error of the weak classifier is <50%, else it will be discarded
       */
      template <class WeakClassifierFactory>
      void learn( const Database& data, ui32 nbWeakClassifiers, const WeakClassifierFactory& factory )
      {
      }
   };
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
      algorithm::WeakClassifierStump<Database> s1 = stumpFactory.create();
      float error = s1.learn( dat, w );

      ui32 nbGood;
      computeAsStumpResults( dat, 0, s1.getThreshold(), s1.isInfReturningZeroClass(), nbGood );
      TESTER_ASSERT( nbGood == dat.size() );
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
      algorithm::WeakClassifierStump<Database> s1 = stumpFactory.create();
      float error = s1.learn( dat, w );

      ui32 nbGood;
      computeAsStumpResults( dat, 0, s1.getThreshold(), s1.isInfReturningZeroClass(), nbGood );
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
      algorithm::WeakClassifierStump<Database> s1 = stumpFactory.create();
      float error = s1.learn( dat, w );

      ui32 nbGood;
      computeAsStumpResults( dat, 0, s1.getThreshold(), s1.isInfReturningZeroClass(), nbGood );
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
      algorithm::WeakClassifierStump<Database> s1 = stumpFactory.create();
      float error = s1.learn( dat, w );

      ui32 nbGood;
      computeAsStumpResults( dat, 0, s1.getThreshold(), s1.isInfReturningZeroClass(), nbGood );
      TESTER_ASSERT( nbGood == dat.size() );
   }

   void testBoosting1()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>   Adaboost;
      typedef algorithm::StumpFactory<Database>    StumpFactory;

      Database dat;
      Adaboost classifier;
      StumpFactory stumpFactory;
      classifier.learn( dat, 30, stumpFactory );

      algorithm::WeakClassifierStump<Database> s1 = stumpFactory.create();
   }

private:
   template <class Database>
   static std::vector<float> makeWeights( const Database& dat )
   {
      return std::vector<float> ( dat.size(), 1.0 / dat.size() );
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
TESTER_TEST(testStumpInf1);
TESTER_TEST(testStumpInf2);
TESTER_TEST(testStumpInf3);
TESTER_TEST(testStumpSup1);
TESTER_TEST(testBoosting1);
TESTER_TEST_SUITE_END();
#endif