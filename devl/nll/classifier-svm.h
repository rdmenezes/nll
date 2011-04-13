/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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

      // for gcc...
      typedef typename Base::Result                   Result;
      typedef typename Base::Database                 Database;
      typedef typename Base::Class                    Class;

   public:
      // TODO ONLY RBF kernel is handled
      enum KernelType { LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED };

      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         parameters.push_back( new ParameterOptimizerGaussianGeometric( 0.00000001, 100, 0.001, 0.0001, 5, 1.0f / 8 ) );
         parameters.push_back( new ParameterOptimizerGaussianLinear( 1, 200, 60, 30, 15 ) );
         return parameters;
      }

   public:
      /**
       @param createProbabilityModel this enable the probability computation that is used by <code>test(p, probabilities)</code>
              if not set to 1, this will return 0
       */
      ClassifierSvm( bool createProbabilityModel = false, bool balanceData = false ) : Base( buildParameters() )
      {
         _kernelType = RBF;
		   _model = 0;
         _vector = 0;
         _nbClasses = 0;
         _createProbabilityModel = createProbabilityModel;
         _balanceData = balanceData;
      }

      virtual void write( const std::string& file ) const
      {
         svm_save_model( file.c_str(), _model );

         std::ofstream f( ( file + ".model").c_str(), std::ios::binary );
         core::write<ui32>( _kernelType, f );
         core::write<ui32>( _inputSize, f );
         core::write<ui32>( _nbClasses, f );
         core::write<ui32>( _createProbabilityModel, f );
         core::write<bool>( _balanceData, f );
      }

      virtual void read( const std::string& file )
      {
         _model = svm_load_model( file.c_str() );

         std::ifstream f( ( file + ".model").c_str(), std::ios::binary );
         core::read<ui32>( _kernelType, f );
         core::read<ui32>( _inputSize, f );
         core::read<ui32>( _nbClasses, f );
         core::read<ui32>( _createProbabilityModel, f );
         core::read<bool>( _balanceData, f );
         if ( !_model )
         {
            throw std::runtime_error( "cannot read classifier from file:" + file );
         }
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
         c->_nbClasses = _nbClasses;
         c->_balanceData = _balanceData;
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

      virtual Class test( const Point& p ) const
      {
         core::Buffer1D<double> pb;
         return test( p, pb );
      }

      virtual Class test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         assert( _model ); // "no svm model loaded"
         assert( _inputSize == p.size() );
         std::auto_ptr<svm_node> i = std::auto_ptr<svm_node>( implementation::build_svm_input_from_vector( p, _inputSize ) );

         core::Buffer1D<double> pb( _nbClasses );
		   f64 res = svm_predict_probability( _model, i.get(), pb.getBuf() );

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
         assert( parameters.size() == this->_parametersPrototype.size() );

         _learnRbfKernel( dat, parameters[ 0 ], parameters[ 1 ] );
      }

   private:
      void _learnRbfKernel(const Database& dat, double gamma, double marginCost)
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

		   f64* y = new f64[ learningIndex.size() ];
		   for ( ui32 n = 0; n < learningIndex.size(); ++n )
			   y[ n ] = dat[ learningIndex[ n ] ].output + 1;

		   Point* inputs = new Point[ learningIndex.size() ];
		   for ( ui32 n = 0; n < learningIndex.size(); ++n )
			   inputs[ n ] = dat[ learningIndex[ n ] ].input;
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
		   param.probability = _createProbabilityModel;

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
		      param.weight_label = NULL;
		      param.weight = NULL;
         }

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
			   svm_free_and_destroy_model(&_model);
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
      ui32        _nbClasses;
      ui32        _createProbabilityModel;
      bool        _balanceData;
   };
}
}

#endif
