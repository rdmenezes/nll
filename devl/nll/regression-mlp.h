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
   class RegressionMlp : public Regression<Point, TOutput>
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

      // gcc...
      typedef typename Base::Result                   Result;
      typedef typename Base::Database                 Database;
      typedef typename Base::Output                   Output;

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
      RegressionMlp( double weightDecay = 0 ) : Base( buildParameters() ), _weightDecay( weightDecay )
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
