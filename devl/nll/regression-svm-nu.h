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

#ifndef NLL_REGRESSION_SVM_NU_H_
# define NLL_REGRESSION_SVM_NU_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm

    @brief Support Vector Machine algorithm
    */
   template <class Point, class TOutput>
   class RegressionSvmNu : public Regression<Point, TOutput>
   {
      typedef Regression<Point, TOutput>  Base;

   public:
      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

      // gcc...
      typedef typename Base::Result                   Result;
      typedef typename Base::Database                 Database;
      typedef typename Base::Output                   Output;

   public:
      // TODO ONLY RBF kernel is handled
      enum KernelType { LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED };

      /**
       @brief Expecting:
       - parameters[ 0 ] = gamma          - default 0.001 - 20
       - parameters[ 1 ] = marginCost     - default 20 - 100
       - parameters[ 2 ] = NU             - default 0.2 - 1
       */
      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         parameters.push_back( new ParameterOptimizerGaussianGeometric( 0.000001, 20, 0.001, 0.0001, 5, 1.0f / 8 ) );
         parameters.push_back( new ParameterOptimizerGaussianLinear( 1, 200, 60, 30, 50 ) );
         parameters.push_back( new ParameterOptimizerGaussianLinear( 0.05, 1, 0.2, 0.5, 0.1 ) );
         return parameters;
      }

   public:
      RegressionSvmNu() : Base( buildParameters() )
      {
         _kernelType = RBF;
		   _model = 0;
         _vector = 0;
      }

      /**
       @todo implement deepcopy
       */
      virtual RegressionSvmNu* deepCopy() const
      {
         RegressionSvmNu* c = new RegressionSvmNu();
         c->_model = 0;
         c->_kernelType = _kernelType;
         c->_inputSize = _inputSize;
         c->_vector = 0;
         c->_crossValidationBin = this->_crossValidationBin;
         return c;
      }

      virtual ~RegressionSvmNu()
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

      virtual Output test( const Point& p ) const
      {
         assert( _model ); // "no svm model loaded"
         assert( _inputSize == p.size() );
         svm_node* i = implementation::build_svm_input_from_vector( p, _inputSize );
		   f64 res = svm_predict( _model, i );
		   delete [] i;
         Output out( 1 );
         out[ 0 ] = static_cast<typename Output::value_type>( res );
		   return out;
      }

      /**
       @param parameters :
               - parameters[ 0 ] = gamma
               - parameters[ 1 ] = marginCost
       */
      virtual void learn( const typename Base::Database& dat, const core::Buffer1D<f64>& parameters )
      {
         if ( !dat.size() )
            return;
         ensure( dat[ 0 ].output.size() == 1, "SVR-NU handles only 1 regression value TODO: automatically create one SVR for each..." );
         assert( parameters.size() == this->_parametersPrototype.size() );

         _learnRbfKernel( dat, parameters[ 0 ], parameters[ 1 ], parameters[ 2 ] );
      }

   private:
      void _learnRbfKernel(const typename Base::Database& dat, double gamma, double marginCost, double nu )
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
			   y[  n ] = dat[ learningIndex[ n ] ].output[ 0 ];

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
		   param.svm_type = NU_SVR;
		   param.kernel_type = RBF;
		   param.cache_size = 100;
         param.eps = 0.1;

         // parameters
   		param.gamma = gamma;
		   param.C = marginCost;
         param.nu = nu;

		   param.shrinking = 1;
		   param.probability = 0;

		   param.weight_label = NULL;
		   param.weight = NULL;

         param.degree = 0;

		   const char* err = svm_check_parameter( &pb, &param );
         ensure( !err, "svm parameter error:" + std::string( err ) ); // error

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
   };
}
}

#endif
