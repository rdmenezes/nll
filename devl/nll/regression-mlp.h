#ifndef NLL_REGRESSION_MLP_H_
# define NLL_REGRESSION_MLP_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Regression using multi-layered neural network
    */
   template <class Point, class TOutput>
   class RegressionMlp : public nll::algorithm::Regression<Point, TOutput>
   {
   public:
      typedef Regression<Point, TOutput>  Base;
      typedef core::Database< core::ClassificationSample< Point, std::vector<double> > > MlpDatabase;

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
      static nll::algorithm::ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         parameters.push_back( new ParameterOptimizerGaussianInteger( 1, 30, 8, 4, 1 ) );
         parameters.push_back( new ParameterOptimizerGaussianGeometric( 0.001, 10, 0.1, 2, 1.25, 0.75  ) );
         parameters.push_back( new ParameterOptimizerGaussianLinear( 0.1, 15, 2, 1, 1  ) );
         return parameters;
      }

   public:
      RegressionMlp( double weightDecay = 0 ) : Regression( buildParameters() ), _weightDecay( weightDecay )
      {}

      virtual RegressionMlp* deepCopy() const
      {
         RegressionMlp* c = new RegressionMlp();
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

      virtual Output test( const Point& p ) const
      {
         assert( p.size() == _pmc.getInputSize() );

         core::Buffer1D<double> i( static_cast<ui32>( p.size() ) );
         for ( ui32 n = 0; n < _pmc.getInputSize(); ++n )
            i[ n ] = static_cast<double>( p[ n ] );
         core::Buffer1D<double> buf = _pmc.propagate( i );

         Output r( _pmc.getOutputSize() );
         for ( ui32 n = 0; n < _pmc.getOutputSize(); ++n )
         {
            r[ n ] = static_cast<typename Output::value_type>( buf[ n ] );
         }
         return r;
      }

      virtual void learn( const Database& dat, const nll::core::Buffer1D<nll::f64>& parameters )
      {
         if ( !dat.size() )
            return;
         ensure( parameters.size() == this->_parametersPrototype.size(), "Incorrect parameters." );
         std::vector<ui32> layerDesc = core::make_vector<ui32>( 
            static_cast<ui32>( dat[0].input.size() ),
            static_cast<ui32>( parameters[ 0 ] ),
            static_cast<ui32>( dat[0].output.size() ) );
         _pmc.createNetwork( layerDesc );

         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            for ( ui32 nn = 0; nn < dat[ n ].output.size(); ++nn )
            {
               // check the database'output is correctly normalized...
               assert( dat[ n ].output[ nn ] >= -0.1 && dat[ n ].output[ nn ] <= 1.1 );
            }
         }
         
         StopConditionMlpThreshold stopCondition( parameters[ 2 ], -10, -10, -10 );
         _pmc.learn( dat, stopCondition, parameters[ 1 ], 0.1, _weightDecay );
      }

   private:
      Mlp<FunctionSimpleDifferenciableSigmoid>   _pmc;
      double                                     _weightDecay;
   };
}
}

#endif
