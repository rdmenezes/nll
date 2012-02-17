/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_CLASSIFIER_SVM_LINEAR_H_
# define NLL_CLASSIFIER_SVM_LINEAR_H_

namespace nll
{
namespace algorithm
{
   namespace implementation
   {
      template <class Type>
      class ClassifierSvm;

      template <class T>
      inline algorithm::feature_node* build_svmlinear_input_from_vector(const T& vec, ui32 size)
      {
	      feature_node* node= new feature_node[size + 1];
	      for (ui32 n = 0; n < size; ++n)
	      {
		      node[n].index = n + 1;
		      node[n].value = vec[n];
	      }
	      node[size].index = -1;
	      return node;
      }

      template <class T>
      inline feature_node** build_svmlinear_inputs_from_vectors(const T& vec, ui32 vector_size, ui32 input_vector_size)
      {
	      feature_node** node= new feature_node*[vector_size];
	      for (ui32 n = 0; n < vector_size; ++n)
	      {
		      node[n] = build_svmlinear_input_from_vector(vec[n], input_vector_size);
	      }
	      return node;
      }

      inline static void delete_svm_nodes(feature_node** nodes, ui32 vector_size )
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
   class ClassifierSvmLinear : public Classifier<Point>
   {
      typedef Classifier<Point>  Base;

   public:
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
      enum SolverType { L2R_LR, L2R_L2LOSS_SVC_DUAL, L2R_L2LOSS_SVC, L2R_L1LOSS_SVC_DUAL, MCSVM_CS, L1R_L2LOSS_SVC, L1R_LR, L2R_LR_DUAL };

      // only parameter is C
      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         parameters.push_back( new ParameterOptimizerGaussianLinear( 1, 10000, 60, 30, 15 ) );
         return parameters;
      }

   public:
      /**
       @param createProbabilityModel this enable the probability computation that is used by <code>test(p, probabilities)</code>
              if not set to 1, this will return 0
       */
      ClassifierSvmLinear( SolverType solver = L2R_LR, bool createProbabilityModel = false, bool balanceData = false ) : Base( buildParameters() )
      {
         _solver = solver;
		   _model = 0;
         _vector = 0;
         _nbClasses = 0;
         _createProbabilityModel = createProbabilityModel;
         _balanceData = balanceData;
      }

      virtual void write( const std::string& file ) const
      {
         save_model( file.c_str(), _model );

         std::ofstream f( ( file + ".model").c_str(), std::ios::binary );
         core::write<ui32>( _solver, f );
         core::write<ui32>( _inputSize, f );
         core::write<ui32>( _nbClasses, f );
         core::write<ui32>( _createProbabilityModel, f );
         core::write<bool>( _balanceData, f );
      }

      virtual void read( const std::string& file )
      {
         _model = load_model( file.c_str() );

         std::ifstream f( ( file + ".model").c_str(), std::ios::binary );
         core::read<ui32>( _solver, f );
         core::read<ui32>( _inputSize, f );
         core::read<ui32>( _nbClasses, f );
         core::read<ui32>( _createProbabilityModel, f );
         core::read<bool>( _balanceData, f );
      }

      /**
       @todo implement deepcopy
       */
      virtual typename Base::Classifier* deepCopy() const
      {
         ClassifierSvmLinear<Point>* c = new ClassifierSvmLinear();
         c->_model = 0;
         c->_solver = _solver;
         c->_inputSize = _inputSize;
         c->_vector = 0;
         c->_crossValidationBin = this->_crossValidationBin;
         c->_nbClasses = _nbClasses;
         c->_balanceData = _balanceData;
         return c;
      }

      virtual ~ClassifierSvmLinear()
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

      virtual Class test( const Point& p ) const
      {
         core::Buffer1D<double> pb;
         return test( p, pb );
      }

      virtual Class test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         assert( _model ); // "no svm model loaded"
         assert( _inputSize == p.size() );
         std::auto_ptr<feature_node> i = std::auto_ptr<feature_node>( implementation::build_svmlinear_input_from_vector( p, _inputSize ) );

         core::Buffer1D<double> pb( _nbClasses );
		   f64 res = predict_probability( _model, i.get(), pb.getBuf() );

         // normalize the probability
         f64 sum = 1e-15;
         for ( ui32 n = 0; n < pb.size(); ++n )
            sum += pb[ n ];
         ensure( sum > 0, "error: probability error" );
         for ( ui32 n = 0; n < pb.size(); ++n )
            pb[ n ] /= sum;

         probability = pb;
		   return static_cast<ui32>( res - 1 );
      }

      /**
       @param parameters :
               - parameters[ 0 ] = gamma
               - parameters[ 1 ] = marginCost
       */
      virtual void learn( const Database& dat, const core::Buffer1D<f64>& parameters )
      {
         ensure( parameters.size() == this->_parametersPrototype.size(), "expected parameters and parameters don't match" );

         _learn( dat, parameters[ 0 ] );
      }

   private:
      void _learn(const Database& dat, double C)
	   {
		   _destroy();

		   if ( dat.size() == 0 )
			   return;
         _nbClasses = getNumberOfClass( dat );

         _inputSize = dat[ 0 ].input.size();
		   std::vector<ui32> learningIndex;
		   for (ui32 n = 0; n < dat.size(); ++n)
            if ( dat[ n ].type == Base::Database::Sample::LEARNING )
				   learningIndex.push_back(n);
		   assert( learningIndex.size() ); // "no learning data in database"

		   int* y = new int[ learningIndex.size() ];
		   for ( ui32 n = 0; n < learningIndex.size(); ++n )
			   y[ n ] = static_cast<int>( dat[ learningIndex[ n ] ].output ) + 1;

		   Point* inputs = new Point[ learningIndex.size() ];
		   for ( ui32 n = 0; n < learningIndex.size(); ++n )
			   inputs[ n ] = dat[ learningIndex[ n ] ].input;
         feature_node** x = implementation::build_svmlinear_inputs_from_vectors( inputs, static_cast<ui32>( learningIndex.size() ), dat[ 0 ].input.size() );
		   delete [] inputs;

		   problem pb;
		   pb.l = static_cast<ui32>( learningIndex.size() );
         pb.n = _inputSize;

		   pb.y = y;
		   pb.x = x;

		   parameter param;
		   param.solver_type = _solver;
         param.eps = 1e-3;
         param.C = C;

         core::Buffer1D<int> labels( _nbClasses );
         core::Buffer1D<double> weights( _nbClasses );
         if ( _balanceData )
         {
            // compute the number of instance for each class
            for ( ui32 n = 0; n < _nbClasses; ++n )
               labels[ n ] = n + 1; // svmlib classes start at 1 by default
            for ( ui32 n = 0; n < learningIndex.size(); ++n )
               ++weights[ dat[ learningIndex[ n ] ].output ];
            double norm = 0;
            for ( ui32 n = 0; n < _nbClasses; ++n )
               norm += weights[ n ];
            for ( ui32 n = 0; n < _nbClasses; ++n )
               weights[ n ] = 1 / ( weights[ n ] / norm );

            // set the penalty factor
            param.nr_weight = _nbClasses;
	         param.weight_label = labels.getBuf();
	         param.weight = weights.getBuf();
         } else {
		      param.nr_weight = 0;
		      param.weight_label = 0;
		      param.weight = 0;
         }

		   const char* err = check_parameter( &pb, &param );
		   ensure( !err, "svm parameter error" ); // error

		   _model = train( &pb, &param );
		   assert( _model ); // "error: model not trained"

         _vector = x;
         _vectorSize = static_cast<ui32>( learningIndex.size() );
		   delete [] y;
	   }

   private:
      void _destroy()
      {
         if (_model)
			   free_and_destroy_model(&_model);
         if ( _vector )
            implementation::delete_svm_nodes( _vector, _vectorSize );
         _model = 0;
      }

   private:
	   model*         _model;
	   ui32           _solver;
      ui32           _inputSize;

      feature_node** _vector;
      ui32           _vectorSize;
      ui32           _nbClasses;
      ui32           _createProbabilityModel;
      bool           _balanceData;
   };
}
}

#endif
