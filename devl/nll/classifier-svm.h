#ifndef NLL_CLASSIFIER_SVM_H_
# define NLL_CLASSIFIER_SVM_H_

namespace nll
{
namespace algorithm
{
   namespace implementation
   {
      template <class Type>
      class ClassifierSvm;

      template <class T>
      inline svm_node* build_svm_input_from_vector(const T& vec, ui32 size)
      {
	      svm_node* node= new svm_node[size + 1];
	      for (ui32 n = 0; n < size; ++n)
	      {
		      node[n].index = n + 1;
		      node[n].value = vec[n];
	      }
	      node[size].index = -1;
	      return node;
      }

      template <class T>
      inline svm_node** build_svm_inputs_from_vectors(const T& vec, ui32 vector_size, ui32 input_vector_size)
      {
	      svm_node** node= new svm_node*[vector_size];
	      for (ui32 n = 0; n < vector_size; ++n)
	      {
		      node[n] = build_svm_input_from_vector(vec[n], input_vector_size);
	      }
	      return node;
      }

      inline static void delete_svm_nodes(svm_node** nodes, ui32 vector_size )
      {
	      for (ui32 n = 0; n < vector_size; ++n)
         {
			   delete nodes[n];
         }
	      delete [] nodes;
      }
   }

   /**
    @ingroup algorithm

    @brief Support Vector Machine algorithm
    */
   template <class Point>
   class ClassifierSvm : public Classifier<Point>
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
      // TODO ONLY RBF kernel is handled
      enum KernelType { LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED };

      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         parameters.push_back( new ParameterOptimizerGaussianGeometric( 0.000001, 20, 0.001, 0.0001, 5, 1.0f / 8 ) );
         parameters.push_back( new ParameterOptimizerGaussianLinear( 1, 200, 60, 30, 15 ) );
         return parameters;
      }

   public:
      ClassifierSvm() : Base( buildParameters() )
      {
         _kernelType = RBF;
		   _model = 0;
         _vector = 0;
      }

      /**
       @todo implement deepcopy
       */
      virtual typename Base::Classifier* deepCopy() const
      {
         ClassifierSvm<Point>* c = new ClassifierSvm();
         c->_model = 0;
         c->_kernelType = _kernelType;
         c->_inputSize = _inputSize;
         c->_vector = 0;
         c->_crossValidationBin = this->_crossValidationBin;
         return c;
      }

      virtual ~ClassifierSvm()
	   {
		   _destroy();
	   }

      /**
       @todo implement read
       */
      virtual void read( std::istream& /*i*/ )
      {
         unreachable("TODO implement");
      }

      /**
       @todo implement write
       */
      virtual void write( std::ostream& /*o*/ ) const
      {
         unreachable("TODO implement");
      }

      virtual typename Base::Class test( const Point& p ) const
      {
         assert( _model ); // "no svm model loaded"
         assert( _inputSize == p.size() );
         svm_node* i = implementation::build_svm_input_from_vector( p, _inputSize );
		   f64 res = svm_predict( _model, i );
		   delete [] i;
		   return static_cast<ui32>( res - 1 );
      }

      /**
       @param parameters :
               - parameters[ 0 ] = gamma
               - parameters[ 1 ] = marginCost
       */
      virtual void learn( const typename Base::Database& dat, const core::Buffer1D<f64>& parameters )
      {
         assert( parameters.size() == this->_parametersPrototype.size() );

         _learnRbfKernel( dat, parameters[ 0 ], parameters[ 1 ] );
      }

   private:
      void _learnRbfKernel(const typename Base::Database& dat, double gamma, double marginCost)
	   {
		   _destroy();

		   if ( dat.size() == 0 )
			   return;
         _inputSize = dat[ 0 ].input.size();
		   std::vector<ui32> learningIndex;
		   for (ui32 n = 0; n < dat.size(); ++n)
            if ( dat[ n ].type == Base::Database::Sample::LEARNING )
				   learningIndex.push_back(n);
		   assert( learningIndex.size() ); // "no learning data in database"

		   f64* y = new f64[ learningIndex.size() ];
		   for ( ui32 n = 0; n < learningIndex.size(); ++n )
			   y[  n ] = dat[ n ].output + 1;

		   Point* inputs = new Point[ learningIndex.size() ];
		   for ( ui32 n = 0; n < learningIndex.size(); ++n )
			   inputs[ n ] = dat[ n ].input;
         svm_node** x = implementation::build_svm_inputs_from_vectors( inputs, static_cast<ui32>( learningIndex.size() ), dat[ 0 ].input.size() );
		   delete [] inputs;

		   svm_problem pb;
		   pb.l = static_cast<ui32>( learningIndex.size() );
		   pb.y = y;
		   pb.x = x;

		   svm_parameter param;
		   param.svm_type = C_SVC;
		   param.kernel_type = RBF;
		   param.degree = 3;
   		
		   param.gamma = gamma;

		   param.coef0 = 0;
		   param.nu = 0.5;
		   param.cache_size = 100;
   		
		   param.C = marginCost;

		   param.eps = 1e-3;
		   param.p = 0.1;
		   param.shrinking = 1;
		   param.probability = 0;
		   param.nr_weight = 0;
		   param.weight_label = NULL;
		   param.weight = NULL;

		   const char* err = svm_check_parameter( &pb, &param );
		   ensure( !err, "svm parameter error" ); // error

		   _model = svm_train( &pb, &param );
		   assert( _model ); // "error: model not trained"

         _vector = x;
         _vectorSize = static_cast<ui32>( learningIndex.size() );
		   delete [] y;
	   }

   private:
      void _destroy()
      {
         if (_model)
			   svm_destroy_model(_model);
         if ( _vector )
            implementation::delete_svm_nodes( _vector, _vectorSize );
         _model = 0;
      }

   private:
	   svm_model*  _model;
	   ui32        _kernelType;
      ui32        _inputSize;

      svm_node**  _vector;
      ui32        _vectorSize;
   };
}
}

#endif
