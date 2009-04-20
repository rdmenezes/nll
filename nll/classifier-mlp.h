#ifndef CLASSIFIER_MLP_H_
# define CLASSIFIER_MLP_H_

# include "multi-layered-perceptron.h"

# define NLL_MLP_EVALUATE_NB_INSTANCE  5

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief multi-layered neural network with backpropagation for learning.

    @note memory optimised for Point = Buffer1D<float>
    @note classifier:database:input: needs size() and operator[] implemented
    */
   template <class Point>
   class ClassifierMlp : public Classifier<Point>
   {
      typedef Classifier<Point>  Base;

   public:
      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

   public:
      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         parameters.push_back( new ParameterOptimizerGaussianInteger( 1, 30, 8, 4, 1 ) );
         parameters.push_back( new ParameterOptimizerGaussianGeometric( 0.001, 10, 0.1, 2, 1.25, 0.75  ) );
         parameters.push_back( new ParameterOptimizerGaussianLinear( 0.1, 15, 2, 1, 1  ) );
         return parameters;
      }

   public:
      ClassifierMlp() : Base( buildParameters() )
      {}
      virtual typename Base::Classifier* deepCopy() const
      {
         ClassifierMlp* c = new ClassifierMlp();
         c->_pmc = _pmc;
         return c;
      }

      virtual void read( std::istream& i )
      {
         bool res = _pmc.read( i );
         assert( res );
      }

      virtual void write( std::ostream& o ) const
      {
         bool res = _pmc.write( o );
         assert( res );
      }

      virtual typename Base::Class test( const Point& p ) const
      {
         assert( _pmc.isWorking() ); // no model
         assert( p.size() );
         pmc::Database::Sample::Input i( p.size() );
         for ( ui32 n = 0; n < _pmc.getInputSize(); ++n )
            i[ n ] = static_cast<f32>( p[ n ] );
         const float * buf = _pmc.calculate( i.getBuf() );
         f32 maxp = INT_MIN;
         ui32 index = 0;
         for ( ui32 n = 0; n < _pmc.getOutputSize(); ++n )
            if ( buf[ n ] > maxp )
            {
               maxp = buf[ n ];
               index = n;
            }
         assert( !core::equal<f32>( INT_MIN, maxp ) );
         return index;
      }

      /**
       param parameters:
             - parameters[ 0 ] = nb neur intermediate layer
             - parameters[ 1 ] = learning rate
             - parameters[ 2 ] = timeout (in sec)
       */
      virtual void learn( const typename Base::Database& dat, const core::Buffer1D<f64>& parameters )
      {
         _pmc.destroy();
         if ( !dat.size() )
            return;
         assert( parameters.size() == this->_parametersPrototype.size() );
         pmc::Database pmcDat = _computePmcDatabase( dat, core::Val2Type<core::Equal<core::Buffer1D<f32>, Point>::value>() );
         ui32 layerDesc[] = { static_cast<ui32>( pmcDat[0].input.size() ),
                              static_cast<ui32>( parameters[ 0 ] ),
                              static_cast<ui32>( pmcDat[0].output.size() ) };
         _pmc.create( 3, layerDesc );
         _pmc.setLearningRate( (float)parameters[ 1 ] );
         _pmc.learn( pmcDat, (float)0, (float)0, (float)parameters[ 2 ] );
      }

   private:
      // specialisation in case the type is the same that the internal buffer type used by the MLP classifier
      inline pmc::Database _computePmcDatabase( const typename Base::Database& dat, core::Val2Type<true> /* isPointDatabaseMatchToBuffer1DF32 */ )
      {
         pmc::Database pmcDatabase;
         ui32 nbOfclass = core::getNumberOfClass( dat );
         for ( ui32 n = 0; n < dat.size(); ++n)
         {
            pmc::Database::Sample s;
            s.input = dat[ n ].input;
            s.output = pmc::Database::Sample::Output( nbOfclass );
            s.output[ dat[ n ].output ] = 1.0f;
            s.type = (pmc::Database::Sample::Type)dat[ n ].type;
            pmcDatabase.add( s );
         }
         return pmcDatabase;
      }

      // specialisation in case the type is the different that the internal buffer type used by the MLP classifier
      // convert it to the same type
      inline pmc::Database _computePmcDatabase( const typename Base::Database& dat, core::Val2Type<false> /* isPointDatabaseMatchToBuffer1DF32 */ )
      {
         pmc::Database pmcDatabase;
         ui32 nbOfclass = core::getNumberOfClass( dat );
         for ( ui32 n = 0; n < dat.size(); ++n)
         {
            pmc::Database::Sample s;
            s.input = pmc::Database::Sample::Input( dat[ n ].input.size() );
            for ( ui32 nn = 0; nn < dat[ n ].input.size(); ++nn)
               s.input[ nn ] = static_cast<f32>( dat[ n ].input[ nn ] );
            s.output = pmc::Database::Sample::Output( nbOfclass );
            s.output[ dat[ n ].output ] = 1.0f;
            s.type = (pmc::Database::Sample::Type)dat[ n ].type;
            pmcDatabase.add( s );
         }
         return pmcDatabase;
      }

      /**
       @todo: should it be averaged? or not that necesary if we do crossvalidation?
       @brief Evaluate the quality of the classifier with the specified parameters and database.

       Computes the mean learning error and mean testing error and computation time. The score is a pondered
       sum of these 3 variables, according a much greater importance on the testing error.
       */

      /*
      virtual double evaluate( const core::Buffer1D<nll::f64>& parameters, const typename Base::Database& dat ) const
      {
         double meanTestError = 0;
         double meanLearnError = 0;

         core::Timer timer;
         for ( ui32 n = 0; n < NLL_MLP_EVALUATE_NB_INSTANCE; ++n )
         {
            typename Base::Classifier* classifier = new ClassifierMlp();
            classifier->learn( dat, parameters );
            typename Base::Classifier::Result r = classifier->test( dat );
            delete classifier;

            meanTestError += r.testingError;
            meanLearnError += r.learningError;
         }
         timer.end();

         return 1 / ( ( meanTestError + meanLearnError * 0.1 + timer.getTime() * 0.0001) / NLL_MLP_EVALUATE_NB_INSTANCE );
      } */
   private:
      pmc   _pmc;
   };
}
}

#endif
