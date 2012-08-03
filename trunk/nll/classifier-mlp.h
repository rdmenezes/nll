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
    @note the features must have a [0..1] normalization

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

      // for gcc...
      typedef typename Base::Result                   Result;
      typedef typename Base::Database                 Database;
      typedef typename Base::Class                    Class;

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

      virtual ClassifierMlp* deepCopy() const
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

      virtual Class test( const Point& p ) const
      {
         core::Buffer1D<double> pb;
         return test( p, pb );
      }

      virtual Class test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         assert( p.size() == _pmc.getInputSize() );
         core::Buffer1D<double> i( p.size() );
         for ( size_t n = 0; n < _pmc.getInputSize(); ++n )
            i[ n ] = static_cast<double>( p[ n ] );
         core::Buffer1D<double> buf = _pmc.propagate( i );
         double maxp = INT_MIN;
         size_t index = 0;
         double sum = 1e-6;
         for ( size_t n = 0; n < _pmc.getOutputSize(); ++n )
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
         for ( size_t n = 0; n < _pmc.getOutputSize(); ++n )
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
      virtual void learn( const Database& dat, const core::Buffer1D<f64>& parameters )
      {
         if ( !dat.size() )
            return;
         ensure( parameters.size() == this->_parametersPrototype.size(), "Incorrect parameters." );
         MlpDatabase pmcDat = _computePmcDatabase( dat );
         std::vector<size_t> layerDesc = core::make_vector<size_t>( 
            static_cast<size_t>( pmcDat[0].input.size() ),
            static_cast<size_t>( parameters[ 0 ] ),
            static_cast<size_t>( pmcDat[0].output.size() ) );
         _pmc.createNetwork( layerDesc );
         
         StopConditionMlpThreshold stopCondition( parameters[ 2 ], -10, -10, -10 );
         _pmc.learn( pmcDat, stopCondition, parameters[ 1 ], 0.1, _weightDecay );
      }

   private:
      // we recreate a new database as the neural network only understand for its output an arrayof doubles
      inline MlpDatabase _computePmcDatabase( const Database& dat )
      {
         MlpDatabase pmcDatabase;
         size_t nbOfclass = core::getNumberOfClass( dat );
         for ( size_t n = 0; n < dat.size(); ++n)
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
