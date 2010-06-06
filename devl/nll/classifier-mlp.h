#ifndef CLASSIFIER_MLP_H_
# define CLASSIFIER_MLP_H_

# define NLL_MLP_EVALUATE_NB_INSTANCE  5

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief multi-layered neural network with backpropagation for learning.

    @note classifier:database:input: needs size() and operator[], constructor(size) implemented
          <code>Mlp</code> for more information on neural networks
    */
   template <class Point>
   class ClassifierMlp : public Classifier<Point>
   {
      typedef Classifier<Point>  Base;
      typedef core::Database< core::ClassificationSample< Point, std::vector<double> > > MlpDatabase;

   public:
      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

   public:
      /**
        Create the parameter specification
             - parameters[ 0 ] = nb neur intermediate layer
             - parameters[ 1 ] = learning rate
             - parameters[ 2 ] = timeout (in sec)
       */
      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         parameters.push_back( new ParameterOptimizerGaussianInteger( 1, 30, 8, 4, 1 ) );
         parameters.push_back( new ParameterOptimizerGaussianGeometric( 0.001, 10, 0.1, 2, 1.25, 0.75  ) );
         parameters.push_back( new ParameterOptimizerGaussianLinear( 0.1, 15, 2, 1, 1  ) );
         return parameters;
      }

   public:
      /**
       @param defines the weight decay to be used
       */
      ClassifierMlp( double weightDecay = 0 ) : Base( buildParameters() ), _weightDecay( weightDecay )
      {}

      virtual typename Base::Classifier* deepCopy() const
      {
         ClassifierMlp* c = new ClassifierMlp();
         c->_pmc = _pmc;
         c->_crossValidationBin = this->_crossValidationBin;
         c->_weightDecay = _weightDecay;
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
         core::Buffer1D<double> pb;
         return test( p, pb );
      }

      virtual Output test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         assert( p.size() == _pmc.getInputSize() );
         core::Buffer1D<double> i( p.size() );
         for ( ui32 n = 0; n < _pmc.getInputSize(); ++n )
            i[ n ] = static_cast<double>( p[ n ] );
         core::Buffer1D<double> buf = _pmc.propagate( i );
         double maxp = INT_MIN;
         ui32 index = 0;
         double sum = 1e-6;
         for ( ui32 n = 0; n < _pmc.getOutputSize(); ++n )
         {
            if ( buf[ n ] > maxp )
            {
               maxp = buf[ n ];
               index = n;
            }
            sum += buf[ n ];
         }

         probability = core::Buffer1D<double>( _pmc.getOutputSize() );
         ensure( sum > 0, "error: probability error" );
         for ( ui32 n = 0; n < _pmc.getOutputSize(); ++n )
         {
            probability[ n ] = buf[ n ] / sum;
         }
         assert( !core::equal<double>( INT_MIN, maxp ) );
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
         if ( !dat.size() )
            return;
         ensure( parameters.size() == this->_parametersPrototype.size(), "Incorrect parameters." );
         MlpDatabase pmcDat = _computePmcDatabase( dat );
         std::vector<ui32> layerDesc = core::make_vector<ui32>( 
            static_cast<ui32>( pmcDat[0].input.size() ),
            static_cast<ui32>( parameters[ 0 ] ),
            static_cast<ui32>( pmcDat[0].output.size() ) );
         _pmc.createNetwork( layerDesc );
         
         StopConditionMlpThreshold stopCondition( parameters[ 2 ], -10, -10, -10 );
         _pmc.learn( pmcDat, stopCondition, parameters[ 1 ], 0.1, _weightDecay );
      }

   private:
      // we recreate a new database as the neural network only understand for its output an arrayof doubles
      inline MlpDatabase _computePmcDatabase( const typename Base::Database& dat )
      {
         MlpDatabase pmcDatabase;
         ui32 nbOfclass = core::getNumberOfClass( dat );
         for ( ui32 n = 0; n < dat.size(); ++n)
         {
            typename MlpDatabase::Sample s;
            s.input = dat[ n ].input;
            s.output = typename MlpDatabase::Sample::Output( nbOfclass );
            s.output[ dat[ n ].output ] = 1.0f;
            s.type = (typename MlpDatabase::Sample::Type)dat[ n ].type;
            pmcDatabase.add( s );
         }
         return pmcDatabase;
      }
   private:
      Mlp<FunctionSimpleDifferenciableSigmoid>   _pmc;
      double                                     _weightDecay;
   };
}
}

#endif
