#ifndef CLASSIFIER_RBF_H_
# define CLASSIFIER_RBF_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief multi-layered neural network with backpropagation for learning.
    */
   template <class Point>
   class ClassifierRbf : public Classifier<Point>
   {
      typedef Classifier<Point>  Base;
      typedef core::Database< core::ClassificationSample< Point, std::vector<double> > > RbfDatabase;

   public:
      typedef RadialBasisNetwork<FunctionSimpleDifferenciableSigmoid> Rbf;

      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

      // for gcc...
      typedef typename Base::Result                   Result;
      typedef typename Base::Database                 Database;
      typedef typename Base::Class                    Class;

   public:
      /**
        Create the parameter specification
             - parameters[ 0 ] = nb RBF
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
      ClassifierRbf(  ) : Base( buildParameters() )
      {}

      virtual typename Base::Classifier* deepCopy() const
      {
         ClassifierRbf* c = new ClassifierRbf();
         c->_rbf = _rbf;
         c->_crossValidationBin = this->_crossValidationBin;
         return c;
      }

      virtual void read( std::istream& i )
      {
         _rbf.read( i );
      }

      virtual void write( std::ostream& o ) const
      {
         _rbf.write( o );
      }

      virtual Class test( const Point& p ) const
      {
         core::Buffer1D<double> pb;
         return test( p, pb );
      }

      /**
       @brief set a monitoring structure. <monitor> will be called everytime the log every <reportTimeIntervalInSec>
       @note a reference is taken on the monitoring structure, so it must be alive when <learn> is called
       */
      void setMonitoring( const Rbf::RadialBasisNetworkMonitoring& monitoring )
      {
         _rbf.setMonitoring( &monitoring );
      }

      virtual Class test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         Point result = _rbf.evaluate( p );

         double maxp = INT_MIN;
         ui32 index = 0;
         double sum = 1e-6;
         for ( ui32 n = 0; n < (ui32)result.size(); ++n )
         {
            if ( result[ n ] > maxp )
            {
               maxp = result[ n ];
               index = n;
            }
            sum += result[ n ];
         }

         probability = core::Buffer1D<double>( (ui32)result.size() );
         ensure( sum > 0, "error: probability error" );
         for ( ui32 n = 0; n < (ui32)result.size(); ++n )
         {
            probability[ n ] = result[ n ] / sum;
         }
         assert( !core::equal<double>( INT_MIN, maxp ) );
         return index;
      }

      /**
       param parameters:
             - parameters[ 0 ] = nb RBF
             - parameters[ 1 ] = learning rate
             - parameters[ 2 ] = timeout (in sec)
       */
      virtual void learn( const Database& dat, const core::Buffer1D<f64>& parameters )
      {
         if ( !dat.size() )
            return;
         ensure( parameters.size() == this->_parametersPrototype.size(), "Incorrect parameters." );
         RbfDatabase pmcDat = _computePmcDatabase( dat );
         
         _rbf.learn( pmcDat, (ui32)parameters[ 0 ], parameters[ 1 ], parameters[ 1 ], parameters[ 1 ], 0, parameters[ 2 ] );
      }

      const Rbf& getRbfImpl() const
      {
         return _rbf
      }

   private:
      // we recreate a new database as the neural network only understand for its output an arrayof doubles
      inline RbfDatabase _computePmcDatabase( const Database& dat )
      {
         RbfDatabase pmcDatabase;
         ui32 nbOfclass = core::getNumberOfClass( dat );
         for ( ui32 n = 0; n < dat.size(); ++n)
         {
            typename RbfDatabase::Sample s;
            s.input = dat[ n ].input;
            s.output = typename RbfDatabase::Sample::Output( nbOfclass );
            s.output[ dat[ n ].output ] = 1.0f;
            s.type = (typename RbfDatabase::Sample::Type)dat[ n ].type;
            pmcDatabase.add( s );
         }
         return pmcDatabase;
      }
   private:
      Rbf   _rbf;
   };
}
}

#endif
