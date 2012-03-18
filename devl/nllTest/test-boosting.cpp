#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;


namespace nll
{
namespace algorithm
{
   /**
    @brief Doesn't to be working well...
    */
   template <class DatabaseT>
   class WeakClassifierLinearSvm : public WeakClassifier<DatabaseT>
   {
      struct SvmParams
      {
         SvmParams()
         {
            _model = 0;
            _vector = 0;
         }

         ~SvmParams()
         {
            destroy();
         }

         void destroy()
         {
            if (_model)
			   free_and_destroy_model(&_model);
            if ( _vector )
               implementation::delete_svm_nodes( _vector, _vectorSize );
            _model = 0;
         }

         model*         _model;
	      ui32           _solver;
         ui32           _inputSize;

         feature_node** _vector;
         ui32           _vectorSize;
         ui32           _nbClasses;
         ui32           _createProbabilityModel;
         bool           _balanceData;
         double         _C;
      };

   public:
      typedef DatabaseT    Database;
      typedef float        value_type;
      typedef typename Database::Sample::Input  Point;

      enum SolverType { L2R_LR, L2R_L2LOSS_SVC_DUAL, L2R_L2LOSS_SVC, L2R_L1LOSS_SVC_DUAL, MCSVM_CS, L1R_L2LOSS_SVC, L1R_LR, L2R_LR_DUAL };

      /**
       @param solver the solver to be used
       @param createProbabilityModel if true, the model can be queried to compute posterior probabilities
       @param if true, the classes will be weighted according to their distribution
       */
      WeakClassifierLinearSvm( double C, SolverType solver = L2R_LR, bool createProbabilityModel = true, bool balanceDataLabel = false )
      {
         _params = std::shared_ptr<SvmParams>( new SvmParams() );

         _params->_solver = solver;
		   _params->_model = 0;
         _params->_vector = 0;
         _params->_nbClasses = 0;
         _params->_createProbabilityModel = createProbabilityModel;
         _params->_balanceData = balanceDataLabel;
         _params->_C = C;
      }

      /**
       @brief Learn the weighted database.
       @param database it is assumed all data in <code>database</code> are training data.
       @param weights the weights associated to each data sample. It is assumed sum(weights) = 1
       */
      virtual value_type learn( const Database& dat, const core::Buffer1D<value_type> weights )
      {
         _params->destroy();

		   if ( dat.size() == 0 )
			   return 0;
         _params->_nbClasses = getNumberOfClass( dat );

         _params->_inputSize = static_cast<ui32>( dat[ 0 ].input.size() );
		   std::vector<ui32> learningIndex;
		   for (ui32 n = 0; n < dat.size(); ++n)
            if ( dat[ n ].type == Database::Sample::LEARNING )
				   learningIndex.push_back(n);
		   assert( learningIndex.size() ); // "no learning data in database"

		   int* y = new int[ learningIndex.size() ];
		   for ( ui32 n = 0; n < learningIndex.size(); ++n )
			   y[ n ] = static_cast<int>( dat[ learningIndex[ n ] ].output );

		   Point* inputs = new Point[ learningIndex.size() ];
		   for ( ui32 n = 0; n < learningIndex.size(); ++n )
			   inputs[ n ] = dat[ learningIndex[ n ] ].input;
         feature_node** x = implementation::build_svmlinear_inputs_from_vectors( inputs, static_cast<ui32>( learningIndex.size() ), static_cast<ui32>( dat[ 0 ].input.size() ) );
		   delete [] inputs;

		   problem pb;
		   pb.l = static_cast<ui32>( learningIndex.size() );
         pb.n = _params->_inputSize;
         pb.bias = -1;

		   pb.y = y;
		   pb.x = x;
         
         pb.W = new double[ pb.l ];
         for ( int n = 0; n < pb.l; ++n )
         {
            pb.W[ n ] = static_cast<double>( weights[ n ] );
         }

		   parameter param;
		   param.solver_type = _params->_solver;
         param.eps = 0.01;
         param.C = _params->_C;

         core::Buffer1D<int> labels( _params->_nbClasses );
         core::Buffer1D<double> weights( _params->_nbClasses );
         if ( _params->_balanceData )
         {
            // compute the number of instance for each class
            for ( ui32 n = 0; n < _params->_nbClasses; ++n )
               labels[ n ] = n + 1; // svmlib classes start at 1 by default
            for ( ui32 n = 0; n < learningIndex.size(); ++n )
               ++weights[ dat[ learningIndex[ n ] ].output ];
            double norm = 0;
            for ( ui32 n = 0; n < _params->_nbClasses; ++n )
               norm += weights[ n ];
            for ( ui32 n = 0; n < _params->_nbClasses; ++n )
               weights[ n ] = 1 / ( weights[ n ] / norm );

            // set the penalty factor
            param.nr_weight = _params->_nbClasses;
	         param.weight_label = labels.getBuf();
	         param.weight = weights.getBuf();
         } else {
		      param.nr_weight = 0;
		      param.weight_label = 0;
		      param.weight = 0;
         }

		   const char* err = check_parameter( &pb, &param );
		   ensure( !err, "svm parameter error:" + std::string( err ) ); // error

		   _params->_model = train( &pb, &param );
		   ensure( _params->_model, "error: model not trained" );

         _params->_vector = x;
         _params->_vectorSize = static_cast<ui32>( learningIndex.size() );
		   delete [] y;
         delete [] pb.W;

         return -1;
      }

      virtual ui32 test( const Point& input ) const
      {
         core::Buffer1D<double> pb; // just discard this param...
         return test( input, pb );
      }

      virtual ui32 test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         ensure( _params->_model, "no svm model loaded" );
         ensure( _params->_inputSize == p.size(), "wrong size" );
         std::auto_ptr<feature_node> i = std::auto_ptr<feature_node>( implementation::build_svmlinear_input_from_vector( p, _params->_inputSize ) );

         core::Buffer1D<double> pb( _params->_nbClasses );
		   f64 res = predict_probability( _params->_model, i.get(), pb.getBuf() );

         // normalize the probability
         f64 sum = 1e-15;
         for ( ui32 n = 0; n < pb.size(); ++n )
            sum += pb[ n ];
         ensure( sum > 0, "error: probability error" );
         for ( ui32 n = 0; n < pb.size(); ++n )
            pb[ n ] /= sum;

         probability = pb;
		   return static_cast<ui32>( res );
      }

   private:
      void _destroy()
      {
         _params = std::shared_ptr<SvmParams>( new SvmParams() );
      }


   private:
      std::shared_ptr<SvmParams> _params; // put all parameters in a shared pointer so we can easily copy the instance
   };


   /**
    @ingroup algorithm
    @brief Linear SVM factory
    */
   template <class DatabaseT>
   class WeakClassifierLinearSvmFactory
   {
   public:
      typedef WeakClassifierLinearSvm<DatabaseT> LinearSvm;
      typedef typename LinearSvm::SolverType     SolverType;

      WeakClassifierLinearSvmFactory( double C, SolverType solver = LinearSvm::L2R_LR, bool createProbabilityModel = true, bool balanceDataLabel = false ) : _C( C ), _solver( solver ), _createProbabilityModel( createProbabilityModel ), _balanceDataLabel( balanceDataLabel )
      {}

      std::shared_ptr<LinearSvm> create() const
      {
         return std::shared_ptr<LinearSvm>( new LinearSvm( _C, _solver, _createProbabilityModel, _balanceDataLabel ) );
      }

   private:
      double      _C;
      SolverType  _solver;
      bool        _createProbabilityModel;
      bool        _balanceDataLabel;
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

      core::Buffer1D<float> w = makeWeights( dat );

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

      core::Buffer1D<float> w = makeWeights( dat );

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

      core::Buffer1D<float> w = makeWeights( dat );

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

      core::Buffer1D<float> w = makeWeights( dat );

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
      dat.add( Database::Sample( core::make_vector<float>( 50 ), 0, Database::Sample::LEARNING ) );

      Adaboost classifier;
      StumpFactory stumpFactory( 10 );
      classifier.learn( dat, 6, stumpFactory );

      TESTER_ASSERT( getTrainingError( dat, classifier ) <= 0 );
   }

   void testBoosting2()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>   Adaboost;
      typedef algorithm::StumpFactory<Database>    StumpFactory;

      srand(1);
      const ui32 nbPoints = 2000;
      const double mean = 0;
      const double var = 1;

      Database dat;
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         const float x = (float)core::generateGaussianDistributionStddev( mean, var );
         const float y = (float)core::generateGaussianDistributionStddev( mean, var );
         const float d = sqrt( x * x + y * y );
         dat.add( Database::Sample( core::make_vector<float>( x, y ), d < 1, Database::Sample::LEARNING ) );
      }

      Adaboost classifier;
      StumpFactory stumpFactory( 10 );
      classifier.learn( dat, 100, stumpFactory );
      const double e = getTrainingError( dat, classifier );
      std::cout << "Error=" << e << std::endl;
      TESTER_ASSERT( e <= 0.01 );
   }

   void testBoostingMlp()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>   Adaboost;
      typedef algorithm::WeakClassifierMlpFactory<Database>    Factory;

      srand(10);
      const ui32 nbPoints = 200;
      const double mean = 0;
      const double var = 0.5;

      Database dat;
      ui32 nbClassOne = 0;
      float max = 0;
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         const float x = (float)core::generateGaussianDistributionStddev( mean, var );
         const float y = (float)core::generateGaussianDistributionStddev( mean, var );
         const ui32 d = sqrt( x * x + y * y ) < 0.6f;
         dat.add( Database::Sample( core::make_vector<float>( x, y ), d, Database::Sample::LEARNING ) );

         max = std::max( fabs( x ), max );
         max = std::max( fabs( y ), max );
         if ( d )
         {
            ++nbClassOne;
         }
      }

      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         for ( ui32 nn = 0; nn < dat[ n ].input.size(); ++nn )
         {
            dat[ n ].input[ nn ] /= max;
         }
      }

      std::cout << "One=" << nbClassOne << " other=" << dat.size() - nbClassOne << std::endl;

      Adaboost classifier;
      std::shared_ptr<algorithm::StopConditionMlpCycleThreshold> stop( new algorithm::StopConditionMlpCycleThreshold( 500 ) );
      std::vector<ui32> layout = core::make_vector<ui32>( 2, 2, 2 ); // the NN is sufficiently weak with average error 0.3 but combined, it is much better

      Factory factory( layout, stop, 1.0, 0 );
      classifier.learn( dat, 10, factory );
      const double e = getTrainingError( dat, classifier );
      std::cout << "Error=" << e << std::endl;
      TESTER_ASSERT( e <= 0.07 );
   }

   void testDecisionTree2()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::WeakClassifierDecisionTreeFactory<Database>    Factory;
      typedef Factory::value_type Classifier;
      
      {
         Database dat;

         dat.add( Database::Sample( core::make_vector<float>( 0, 0 ), 1, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, 1 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, 2 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, -1 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, -2 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 1, 0 ), 0, Database::Sample::LEARNING ) );

         core::Buffer1D<float> ws = makeWeights( dat );

         Classifier c( 1 );
         c.learn( dat, ws );

         const Classifier::Classifier& cc = c.getTree();
         TESTER_ASSERT( cc.getNodes().size() == 2 );

         const Classifier::Classifier& cc1 = c.getTree().getNodes()[ 0 ];
         const Classifier::Classifier& cc2 = c.getTree().getNodes()[ 0 ];
         TESTER_ASSERT( cc1.getNodes().size() == 0 );
         TESTER_ASSERT( cc2.getNodes().size() == 0 );
         const float error = getTrainingError( dat, c );
         TESTER_ASSERT( core::equal( error, 1.0f / 6, 0.001f ) ); // without weight, we just classify everything as 0, so we expect to have only one error
      }


      {
         // do the same as before, but with the weight this time!
         Database dat;

         dat.add( Database::Sample( core::make_vector<float>( 0, 0 ), 1, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, 1 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, 2 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, -1 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, -2 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 1, 0 ), 0, Database::Sample::LEARNING ) );

         core::Buffer1D<float> ws = core::make_buffer1D<float>( 10, 1, 1, 1, 1, 1 );

         Classifier c( 1 );
         c.learn( dat, ws );

         const Classifier::Classifier& cc = c.getTree();
         TESTER_ASSERT( cc.getNodes().size() == 2 );

         const Classifier::Classifier& cc1 = c.getTree().getNodes()[ 0 ];
         const Classifier::Classifier& cc2 = c.getTree().getNodes()[ 0 ];
         TESTER_ASSERT( cc1.getNodes().size() == 0 );
         TESTER_ASSERT( cc2.getNodes().size() == 0 );
         const float error = getTrainingError( dat, c );
         TESTER_ASSERT( core::equal( error, 3.0f / 6, 0.001f ) ); // without weight, we just classify everything as 0, so we expect to have only one error
      }
   }


   void testPerceptronWeights()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::WeakClassifierMarginPerceptronFactory<Database>    Factory;
      typedef Factory::value_type Classifier;
      
      
      {
         Database dat;

         dat.add( Database::Sample( core::make_vector<float>( 0, 0 ), 1, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, 1 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, 2 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, -1 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 0, -2 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 1, 0 ), 0, Database::Sample::LEARNING ) );

         core::Buffer1D<float> ws = makeWeights( dat );

         Classifier c( 10, 1, 0 );
         c.learn( dat, ws );

         const float error = getTrainingError( dat, c );
         TESTER_ASSERT( core::equal( error, 1.0f / 6, 0.001f ) ); // without weight, we just classify everything as 0, so we expect to have only one error
      }


      {
         // do the same as before, but with the weight this time!
         Database dat;

         dat.add( Database::Sample( core::make_vector<float>( 1, 0 ), 1, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 1, 1 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 1, 2 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 1, -1 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 1, -2 ), 0, Database::Sample::LEARNING ) );
         dat.add( Database::Sample( core::make_vector<float>( 2, 0 ), 0, Database::Sample::LEARNING ) );

         core::Buffer1D<float> ws = core::make_buffer1D<float>( 10, 1, 1, 1, 1, 1 );

         Classifier c( 10, 1, 0 );
         c.learn( dat, ws );

         const float error = getTrainingError( dat, c );
         TESTER_ASSERT( core::equal( error, 2.0f / 6, 0.001f ) ); // without weight, we just classify everything as 0, so we expect to have only one error
      }
   }

   void testDecisionTree()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>         Adaboost;
      typedef algorithm::WeakClassifierDecisionTreeFactory<Database>    Factory;


      const ui32 nbPoints = 200;
      const double mean = 10;
      const double var = 5;
      const double d = 7;


      Database dat;
      ui32 nbOne = 0;
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         const float x = (float)core::generateGaussianDistributionStddev( mean, var );
         const float y = (float)core::generateGaussianDistributionStddev( mean, var );
         const bool isInside = std::sqrt( core::sqr( x - mean ) + core::sqr( y - mean ) ) < d;
         dat.add( Database::Sample( core::make_vector<float>( x, y ), isInside ? 1 : 0, Database::Sample::LEARNING ) );
         if ( isInside ) ++nbOne;
      }

      std::cout << "class distrib: 1=" << nbOne << " -1=" << (dat.size() - nbOne ) << std::endl;

      Adaboost classifier; 
      Factory factory( 2, 15 );
      classifier.learn( dat, 30, factory );

      core::Image<ui8> out;
      for ( size_t n = 0; n < classifier.getClassifiers().size(); ++n )
      {
         print( core::vector2i( -50, -50 ), core::vector2i( 50, 50 ), *classifier.getClassifiers()[ n ].classifier, dat, out );
         core::writeBmp( out, "c:/tmp/decision-tree-" + core::val2str( n ) + ".bmp" );
      }

      print( core::vector2i( -50, -50 ), core::vector2i( 50, 50 ), classifier, dat, out );
      core::writeBmp( out, "c:/tmp/decision-tree-single-" + core::val2str( 0 ) + ".bmp" );
      std::cout << "ErrorSingle="  << getTrainingError( dat, classifier ) << std::endl;

      std::cout << "Error="  << getTrainingError( dat, classifier ) << std::endl;
      TESTER_ASSERT( getTrainingError( dat, classifier ) <= 0.0 );
   }

   void testBoostingLinearSvm()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>      Adaboost;
      typedef algorithm::WeakClassifierLinearSvmFactory<Database>   WeakClassifierLinearSvmFactory;


      const ui32 nbPoints = 200;
      const double mean = 0;
      const double var = 1;

      Database dat;
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         const float x = (float)core::generateGaussianDistributionStddev( mean, var );
         const float y = (float)core::generateGaussianDistributionStddev( mean, var );
         const float d = sqrt( x * x + y * y );
         dat.add( Database::Sample( core::make_vector<float>( x, y ), d < 1, Database::Sample::LEARNING ) );
      }

      Adaboost classifier; 
      WeakClassifierLinearSvmFactory svmFactory( 1, WeakClassifierLinearSvmFactory::LinearSvm::L2R_LR );
      classifier.learn( dat, 5, svmFactory );

      std::cout << "Error="  << getTrainingError( dat, classifier ) << std::endl;
      TESTER_ASSERT( getTrainingError( dat, classifier ) <= 0 );
   }

   void testPerceptron()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::MarginPerceptron Classifier;

      Database dat;
      dat.add( Database::Sample( core::make_vector<float>( -11000 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( -10 ), 0, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 11 ), 1, Database::Sample::LEARNING ) );
      dat.add( Database::Sample( core::make_vector<float>( 12 ), 1, Database::Sample::LEARNING ) );

      Classifier classifier;
      classifier.learn( dat, 1000, 0.5, 1 );
      const double error = getTrainingError( dat, classifier );
      std::cout << "error=" << error << std::endl;

      TESTER_ASSERT( error <= 0.25 );
   }

   void testPerceptron2()
   {
      srand( 1 );
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>      Adaboost;
      typedef algorithm::WeakClassifierMarginPerceptronFactory<Database>   Factory;


      const ui32 nbPoints = 200;
      const double mean = 20;
      const double var = 10;

      const double mean2x = 0;
      const double mean2y = 20;
      const double var2 = 10;

      Database dat;
      ui32 nbOne = 0;
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         {
            const float x = (float)core::generateGaussianDistributionStddev( mean, var );
            const float y = (float)core::generateGaussianDistributionStddev( mean, var );
            dat.add( Database::Sample( core::make_vector<float>( x, y ), 0, Database::Sample::LEARNING ) );
         }

         {
            const float x = (float)core::generateGaussianDistributionStddev( mean2x, var2 );
            const float y = (float)core::generateGaussianDistributionStddev( mean2y, var2 );
            dat.add( Database::Sample( core::make_vector<float>( x, y ), 1, Database::Sample::LEARNING ) );
            ++nbOne;
         }
      }

      std::cout << "class distrib: 1=" << nbOne << " -1=" << (dat.size() - nbOne ) << std::endl;

      Adaboost classifier; 
      Factory factory( 10, 0.1, 0.1 );
      classifier.learn( dat, 100, factory );

      core::Image<ui8> out;
      for ( size_t n = 0; n < classifier.getClassifiers().size(); ++n )
      {
         print( core::vector2i( -50, -50 ), core::vector2i( 50, 50 ), *classifier.getClassifiers()[ n ].classifier, dat, out );
         core::writeBmp( out, "c:/tmp/decision2-" + core::val2str( n ) + ".bmp" );
      }

      std::cout << "Error="  << getTrainingError( dat, classifier ) << std::endl;
      TESTER_ASSERT( getTrainingError( dat, classifier ) <= 0.17 );
   }

   void testPerceptron3()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>      Adaboost;
      typedef algorithm::WeakClassifierMarginPerceptronFactory<Database>   Factory;

      srand( 3 );
      for ( ui32 iter = 0; iter < 50; )
      {
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "testPerceptron3, iter=" + core::val2str( iter ) );
         const ui32 nbPoints = 200;
         const double mean = core::generateUniformDistribution( -60, 60 );
         const double var = 10;

         const double mean2x = core::generateUniformDistribution( -60, 60 );
         const double mean2y = core::generateUniformDistribution( -60, 60 );
         const double var2 = 10;

         if ( std::sqrt( core::sqr( mean - mean2x ) + core::sqr( mean - mean2y ) ) < 25 )
            continue;

         Database dat;
         ui32 nbOne = 0;
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            {
               const float x = (float)core::generateGaussianDistributionStddev( mean, var );
               const float y = (float)core::generateGaussianDistributionStddev( mean, var );
               dat.add( Database::Sample( core::make_vector<float>( x, y ), 0, Database::Sample::LEARNING ) );
            }

            {
               const float x = (float)core::generateGaussianDistributionStddev( mean2x, var2 );
               const float y = (float)core::generateGaussianDistributionStddev( mean2y, var2 );
               dat.add( Database::Sample( core::make_vector<float>( x, y ), 1, Database::Sample::LEARNING ) );
               ++nbOne;
            }
         }

         std::cout << "class distrib: 1=" << nbOne << " -1=" << (dat.size() - nbOne ) << std::endl;
         algorithm::MarginPerceptron classifier;
         classifier.learn( dat, 1000, 1, 0 );

         core::Image<ui8> out;
         print( core::vector2i( -100, -100 ), core::vector2i( 100, 100 ), classifier, dat, out );
         core::writeBmp( out, "c:/tmp/decision" + core::val2str( iter ) + ".bmp" );


         std::cout << "Error="  << getTrainingError( dat, classifier ) << std::endl;
         TESTER_ASSERT( getTrainingError( dat, classifier ) <= 0.1 );

         ++iter;
      }
   }

   void testBoostingLinearSvm2()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>         Adaboost;
      typedef algorithm::WeakClassifierLinearSvmFactory<Database>   Factory;


      const ui32 nbPoints = 200;
      const double mean = 10;
      const double var = 5;
      const double d = 6;


      Database dat;
      ui32 nbOne = 0;
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         const float x = (float)core::generateGaussianDistributionStddev( mean, var );
         const float y = (float)core::generateGaussianDistributionStddev( mean, var );
         const bool isInside = std::sqrt( core::sqr( x - mean ) + core::sqr( y - mean ) ) < d;
         dat.add( Database::Sample( core::make_vector<float>( x, y ), isInside ? 1 : 0, Database::Sample::LEARNING ) );
         if ( isInside ) ++nbOne;
      }

      std::cout << "class distrib: 1=" << nbOne << " -1=" << (dat.size() - nbOne ) << std::endl;

      Adaboost classifier; 
      Factory factory( 1 );
      classifier.learn( dat, 100, factory );

      core::Image<ui8> out;
      for ( size_t n = 0; n < classifier.getClassifiers().size(); ++n )
      {
         print( core::vector2i( -50, -50 ), core::vector2i( 50, 50 ), *classifier.getClassifiers()[ n ].classifier, dat, out );
         core::writeBmp( out, "c:/tmp/decision-stum4-" + core::val2str( n ) + ".bmp" );
      }

      print( core::vector2i( -50, -50 ), core::vector2i( 50, 50 ), classifier, dat, out );
      core::writeBmp( out, "c:/tmp/decision-stum4-single-" + core::val2str( 0 ) + ".bmp" );
      std::cout << "ErrorSingle="  << getTrainingError( dat, classifier ) << std::endl;

      std::cout << "Error="  << getTrainingError( dat, classifier ) << std::endl;
      TESTER_ASSERT( getTrainingError( dat, classifier ) <= 0 );
   }

   void testStumpInf4()
   {
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>         Adaboost;
      typedef algorithm::StumpFactory<Database>   Factory;


      const ui32 nbPoints = 200;
      const double mean = 10;
      const double var = 5;
      const double d = 7;


      Database dat;
      ui32 nbOne = 0;
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         const float x = (float)core::generateGaussianDistributionStddev( mean, var );
         const float y = (float)core::generateGaussianDistributionStddev( mean, var );
         const bool isInside = std::sqrt( core::sqr( x - mean ) + core::sqr( y - mean ) ) < d;
         dat.add( Database::Sample( core::make_vector<float>( x, y ), isInside ? 1 : 0, Database::Sample::LEARNING ) );
         if ( isInside ) ++nbOne;
      }

      std::cout << "class distrib: 1=" << nbOne << " -1=" << (dat.size() - nbOne ) << std::endl;

      Adaboost classifier; 
      Factory factory( 1000 );
      classifier.learn( dat, 100, factory );

      core::Image<ui8> out;
      for ( size_t n = 0; n < classifier.getClassifiers().size(); ++n )
      {
         print( core::vector2i( -50, -50 ), core::vector2i( 50, 50 ), *classifier.getClassifiers()[ n ].classifier, dat, out );
         core::writeBmp( out, "c:/tmp/decision-stum4-" + core::val2str( n ) + ".bmp" );
      }

      print( core::vector2i( -50, -50 ), core::vector2i( 50, 50 ), classifier, dat, out );
      core::writeBmp( out, "c:/tmp/decision-stum4-single-" + core::val2str( 0 ) + ".bmp" );
      std::cout << "ErrorSingle="  << getTrainingError( dat, classifier ) << std::endl;

      std::cout << "Error="  << getTrainingError( dat, classifier ) << std::endl;
      TESTER_ASSERT( getTrainingError( dat, classifier ) <= 0 );
   }

   void testPerceptron4()
   {
      srand( 0 );
      typedef core::Database< core::ClassificationSample< std::vector<float>, ui32 > > Database;
      typedef algorithm::AdaboostBasic<Database>      Adaboost;
      typedef algorithm::WeakClassifierMarginPerceptronFactory<Database>   Factory;


      const ui32 nbPoints = 200;
      const double mean = 20;
      const double var = 10;
      const double d = 10;


      Database dat;
      ui32 nbOne = 0;
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         const float x = (float)core::generateGaussianDistributionStddev( mean, var );
         const float y = (float)core::generateGaussianDistributionStddev( mean, var );
         const bool isInside = std::sqrt( core::sqr( x - mean ) + core::sqr( y - mean ) ) < d;
         dat.add( Database::Sample( core::make_vector<float>( x, y ), isInside ? 1 : 0, Database::Sample::LEARNING ) );
         if ( isInside ) ++nbOne;
      }

      std::cout << "class distrib: 1=" << nbOne << " -1=" << (dat.size() - nbOne ) << std::endl;

      Adaboost classifier; 
      Factory factory( 100, 1, 0 );
      classifier.learn( dat, 30, factory );

      core::Image<ui8> out;
      for ( size_t n = 0; n < classifier.getClassifiers().size(); ++n )
      {
         print( core::vector2i( -50, -50 ), core::vector2i( 50, 50 ), *classifier.getClassifiers()[ n ].classifier, dat, out );
         core::writeBmp( out, "c:/tmp/decision4-" + core::val2str( n ) + ".bmp" );
      }

      {
         print( core::vector2i( -50, -50 ), core::vector2i( 50, 50 ), classifier, dat, out );
         core::writeBmp( out, "c:/tmp/decision4-single-" + core::val2str( 0 ) + ".bmp" );
         std::cout << "ErrorSingle="  << getTrainingError( dat, classifier ) << std::endl;
      }

      std::cout << "Error="  << getTrainingError( dat, classifier ) << std::endl;
      TESTER_ASSERT( getTrainingError( dat, classifier ) <= 0.006 ); 
   }

private:
   template <class Database>
   static core::Buffer1D<float> makeWeights( const Database& dat )
   {
      core::Buffer1D<float> w( dat.size() );
      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         w[ n ] = 1.0f / dat.size();
      }
      return w;
   }

   template <class Database, class Classifier>
   float getTrainingError( const Database& dat, const Classifier& c )
   {
      ui32 nbErrors = 0;
      Database learning = core::filterDatabase( dat, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );
      for ( ui32 n = 0; n < learning.size(); ++n )
      {
         const ui32 cc = c.test( learning[ n ].input );
         if ( cc != learning[ n ].output )
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

   template <class Classifier, class Database>
   static void print( const core::vector2i& min, const core::vector2i& max, const Classifier& c, const Database& dat, core::Image<ui8>& out )
   {
      ui32 sx = max[ 0 ] - min[ 0 ];
      ui32 sy = max[ 1 ] - min[ 1 ];
      out = core::Image<ui8>( sx, sy, 3 );

      ui8 red[] = {255, 0, 0};
      ui8 green[] = {0, 255, 0};
      for ( int y = 0; y < sy; ++y )
      {
         for ( int x = 0; x < sx; ++x )
         {
            std::vector<float> p( 2 );
            p[ 0 ] = min[ 0 ] + x;
            p[ 1 ] = min[ 1 ] + y;

            ui32 cc = c.test( p );
            ui8* color = ( cc == 1 ) ? green : red;

            out( x, y, 0 ) = color[ 0 ];
            out( x, y, 1 ) = color[ 1 ];
            out( x, y, 2 ) = color[ 2 ];
         }
      }

      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         int px = dat[ n ].input[ 0 ] - min[ 0 ];
         int py = dat[ n ].input[ 1 ] - min[ 1 ];

         if ( px < 0 || px >= out.sizex() ||
              py < 0 || py >= out.sizey() )
         {
            std::cout << "skipeed=" << px << " " << py << std::endl;
            continue;
         }

         ui8* color = dat[ n ].output == 1 ? green : red;

         out( px, py, 0 ) = color[ 0 ] * 0.8;
         out( px, py, 1 ) = color[ 1 ] * 0.8;
         out( px, py, 2 ) = color[ 2 ] * 0.8;
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
TESTER_TEST(testBoosting2);
TESTER_TEST(testBoostingMlp);
TESTER_TEST(testStumpInf4);
TESTER_TEST(testPerceptron3);
TESTER_TEST(testPerceptron);
TESTER_TEST(testPerceptron2);

TESTER_TEST(testStumpInf4);
TESTER_TEST(testDecisionTree);
TESTER_TEST(testDecisionTree2);
TESTER_TEST(testPerceptronWeights);
TESTER_TEST(testPerceptron4);

// not working.. samples need to be in range [0..1], but still...
//TESTER_TEST(testBoostingLinearSvm);
//TESTER_TEST(testBoostingLinearSvm2);
TESTER_TEST_SUITE_END();
#endif