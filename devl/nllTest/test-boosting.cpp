#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
   /**
    @brief Generic interface for weak classifiers, designed to be used with <code>Adaboost</code>
    */
   template <class DatabaseT>
   class WeakClassifier
   {
   public:
      typedef DatabaseT    Database;

      /**
       @brief Learn the weighted database.
       @param database it is assumed all data in <code>database</code> are training data. It must contain exactly two classes
       @param weights the weights associated to each data sample
       */
      virtual void learn( const Database& database, const std::vector<float>& weights );
      virtual ui32 test( const typename Database::Input& input ) const;

      virtual ~WeakClassifier()
      {}
   };

   /**
    @brief Weak learner using simple thresholding attribut to compute the class
    */
   template <class DatabaseT>
   class WeakClassifierStump : public WeakClassifier<DatabaseT>
   {
   public:
      /**
       @brief Learn the weighted database.
       @param database it is assumed all data in <code>database</code> are training data
       @param weights the weights associated to each data sample
       */
      virtual void learn( const Database& database, const std::vector<float>& weights )
      {
      }

      virtual ui32 test( const typename Database::Input& input ) const
      {
         return 0;
      }
   };

   /**
    @brief Stump factory
    */
   template <class DatabaseT>
   class StumpFactory
   {
   public:
      WeakClassifierStump<DatabaseT> create() const
      {
         return WeakClassifierStump<DatabaseT>();
      }
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
   void testBoosting1()
   {
      typedef core::Database< std::vector<float> > Database;
      typedef algorithm::AdaboostBasic<Database>   Adaboost;
      typedef algorithm::StumpFactory<Database>    StumpFactory;

      Database dat;
      Adaboost classifier;
      StumpFactory stumpFactory;
      classifier.learn( dat, 30, stumpFactory );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestBoosting);
TESTER_TEST(testBoosting1);
TESTER_TEST_SUITE_END();
#endif