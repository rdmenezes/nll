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

#ifndef NLL_ALGORITHM_PMC_H_
# define NLL_ALGORITHM_PMC_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief defines a sigmoid function
    */
   class FunctionSimpleDifferenciableSigmoid
   {
   public:
      inline double evaluate( double val ) const
      {
         return 1 / ( 1 + exp( -val ) );
      }

      inline double evaluateDerivative( double val ) const
      {
         const double v = evaluate( val );
         return v * ( 1 - v );
      }
   };

   /**
    @ingroup algorithm
    @brief Defines the stop condition of the neural network
    */
   class StopConditionMlp
   {
   public:
      /**
       @return true if given these errors, the training can be stopped
       */
      virtual bool stop( size_t nbCycles, double errorTraining, double errorValidation, double errorTest ) const = 0;

      virtual ~StopConditionMlp()
      {}
   };

   /**
    @ingroup algorithm
    @brief stop condition if one of the error is below the threshold
    */
   class StopConditionMlpThreshold : public StopConditionMlp
   {
   public:
      StopConditionMlpThreshold( double timer, double minTraining = 0, double minValidation = 0, double minTest = 0 )
      {
         _maxTime = timer;
         _errorTraining = minTraining;
         _errorValidation = minValidation;
         _errorTest = minTest;
      }

      virtual bool stop( size_t /*nbCycles*/, double errorTraining, double errorValidation, double errorTest ) const
      {
         double time = _timer.getCurrentTime();
         return errorTraining < _errorTraining     ||
                errorValidation < _errorValidation ||
                errorTest < _errorTest             ||
                time > _maxTime;
      }

   private:
      double _errorTraining;
      double _errorValidation;
      double _errorTest;
      double _maxTime;
      core::Timer _timer;
   };

   /**
    @ingroup algorithm
    @brief stop condition if enough cycles
    */
   class StopConditionMlpCycleThreshold : public StopConditionMlp
   {
   public:
      StopConditionMlpCycleThreshold( size_t nbCycles )
      {
         _nbCycles = nbCycles;
      }

      virtual bool stop( size_t nbCycles, double /*errorTraining*/, double /*errorValidation*/, double /*errorTest*/ ) const
      {
         return nbCycles >= _nbCycles;
      }

   private:
      size_t _nbCycles;
   };

   /**
    @ingroup algorithm
    @brief Defines a multi layered neural network

    We defines sk = sum_j ( wjk * yj ), yk = f( sk )
    One extra unit is added to each layer except the last layer to simulate the bias. These units have always y = 1
    */
   template <class FunctionSimpleDifferenciable>
   class Mlp
   {
      typedef core::Buffer1D<size_t>  Vectori;
      typedef std::vector<double>   Vectorf;

      struct Unit
      {
         // these variables are mutable as they dont define the network, but simply a temporary state
         mutable double  s;      /// defined as sum_j ( wjk^p * yj^p )
         mutable double  y;      /// defined as f( sk^p )
         mutable double  grad;   /// dE^p/dsk^p, p a pattern
         Vectorf         weights;/// defines the connections. No connection to the bias units
         mutable Vectorf olddw;  /// previsous weight modification value

         bool write( std::ostream& f ) const
         {
            core::write<Vectorf>( weights, f );
            return true;
         }

         bool read( std::istream& f )
         {
            core::read<Vectorf>( weights, f );
            olddw = Vectorf( weights.size() );
            return true;
         }
      };
      typedef std::vector<Unit>  Layer;
      typedef std::vector<Layer> Layers;

   public:
      struct Result
      {
         double learningError;
         double validationError;
         double testerror;
      };

   public:
      /**
       @brief Construct a new neural network with the specified layout

       Vector must define operator[](unsigned) and size().
       @todo add C++0x Concept
       */
      template <class Vector>
      Mlp( const Vector& layers )
      {
         createNetwork( layers );
      }

      Mlp( const std::string& file )
      {
         read( file );
      }

      Mlp( const char* file )
      {
         read( file );
      }

      /**
       @brief Create an empty network
       */
      Mlp()
      {
      }

      /**
       @brief Construct a new neural network with the specified layout

       Vector must define operator[](unsigned) and size().
       @todo add C++0x Concept
       */
      template <class Vector>
      void createNetwork( const Vector& layers )
      {
         ensure( layers.size() >= 3, "only valid for at least 3 layers" );
         size_t size = static_cast<size_t>( layers.size() );
         _layersDesc = Vectori( size );
         for ( size_t n = 0; n < size; ++n )
         {
            // add the bias unit
            _layersDesc[ n ] = layers[ n ] + ( n != ( layers.size() - 1 ) );
         }
      }

      virtual ~Mlp()
      {
      }

   protected:
      /**
       @brief Create and initialize with random value the network using <code>_layersDesc</code>
              The bias units are located at the end of each layer except the las one
       */
      void _createNetwork()
      {
         _layers = Layers( _layersDesc.size() );
         for ( size_t n = 0; n < _layersDesc.size(); ++n )
            _layers[ n ] = Layer( _layersDesc[ n ] );
         for ( size_t n = 0; n < _layersDesc.size() - 1; ++n )
         {
            for ( size_t nn = 0; nn < _layers[ n ].size(); ++nn )
            {
               // check if ( n + 1 ) is last layer to remove the bias weight
               const size_t isLastLayer = ( n + 1 ) == ( _layers.size() - 1 );
               const size_t size = _layers[ n + 1 ].size() - ( isLastLayer != 1 );
               _layers[ n ][ nn ].weights = Vectorf( _layers[ n + 1 ].size() - ( isLastLayer != 1 ) );
               _layers[ n ][ nn ].olddw = Vectorf( _layers[ n + 1 ].size() - ( isLastLayer != 1 ) );
               for ( size_t nnn = 0; nnn < size; ++nnn )
               {
                  // init the network with very small random weights
                  _layers[ n ][ nn ].weights[ nnn ] = core::generateUniformDistribution( -0.1, 0.1 );
                  _layers[ n ][ nn ].olddw[ nnn ] = 0;
               }
            }

            // set the bias factor to 1
            _layers[ n ][ _layers[ n ].size() - 1 ].y = 1;
         }

         // init the result vector
         _result = core::Buffer1D<double>( _layersDesc[ (size_t)_layers.size() - 1 ] );
      }

      /**
       @brief the input must be in the range of the transfer function

       y & s are updated for each unit
       */
      template <class Vector>
      void _propagate( const Vector& input ) const
      {
         FunctionSimpleDifferenciable f;

         ensure( _layers.size(), "init the network first" );
         ensure( input.size() == _layers[ 0 ].size() - 1, "error: input size doesn't match" );
         for ( size_t n = 0; n < (size_t)input.size(); ++n )
         {
            _layers[ 0 ][ n ].y = input[ n ];
         }

         for ( size_t layer = 1; layer < _layers.size(); ++layer )
         {
            size_t isLastLayer = ( ( layer + 1 ) == _layers.size() );
            size_t nbUnits = _layersDesc[ layer ] - ( isLastLayer != 1 );
            for ( size_t n = 0; n < nbUnits; ++n )
            {
               double sn = 0;
               for ( size_t nn = 0; nn < _layersDesc[ layer - 1 ]; ++nn )
                  sn += _layers[ layer - 1 ][ nn ].weights[ n ] * _layers[ layer - 1 ][ nn ].y;
               _layers[ layer ][ n ].s = sn;
               _layers[ layer ][ n ].y = f.evaluate( sn );
            }
         }
      }

      /**
       @brief Compute the squared sum of weights
       */
      double _errorWeightDecay() const
      {
         double weight = 0;
         for ( size_t layer = 0; layer < _layers.size() - 1; ++layer )
         {
            size_t isLastLayer = ( static_cast<size_t>( _layers.size() ) - 1 - 1 ) == layer;
            size_t size = _layersDesc[ layer + 1 ] - ( isLastLayer != 1 );
            for ( size_t k = 0; k < layer; ++k )
               for ( size_t o = 0; o < size; ++o )
               {
                  const double val = _layers[ layer ][ k ].weights[ o ];
                  weight += val * val;
               }
         }
         return 0.5 * weight;
      }

      /**
       @brief Compute the error. Assumed clamped & propagated error.

       E^p=0.5*sum( (output_o - expected_o )^2 )
       E=sum( E^p )
       */
      template <class Vector>
      double _errorPattern( const Vector& output ) const
      {
         double sum = 0;
         const size_t lastLayer = (size_t)_layers.size() - 1;
         ensure( (size_t)output.size() == _layersDesc[ lastLayer ], "size doesn't match" );
         for ( size_t n = 0; n < _layersDesc[ lastLayer ]; ++n )
         {
            double s = output[ n ] - _layers[ lastLayer ][ n ].y;
            sum += s * s;
         }
         return sum / 2;
      }

      /**
       @brief Evaluate grad = -dE^p/s_k
              See http://www.learnartificialneuralnetworks.com/backpropagation.html for implementation details
       @param weight the weight of the current sample must be in [0..1]
       */
      template <class Vector>
      void _computeGrad( const Vector& output, double weight )
      {
         FunctionSimpleDifferenciable f;

         // last layer
         size_t lastLayer = static_cast<size_t>( _layers.size() ) - 1;
         for ( size_t n = 0; n < _layersDesc[ lastLayer ]; ++n )
         {
            _layers[ lastLayer ][ n ].grad = ( output[ n ] - _layers[ lastLayer ][ n ].y ) * 
                                             f.evaluateDerivative( _layers[ lastLayer ][ n ].s ) * weight;
         }

         // other layers
         for ( int layer = static_cast<int>( lastLayer ) - 1; layer >= 0; --layer )
         {
            for ( size_t k = 0; k < _layersDesc[ layer ]; ++k )
            {
               double sum = 0;
               const size_t isLast = ( (size_t)layer + 1 + 1 ) == _layers.size();
               const size_t size = _layersDesc[ layer + 1 ] - ( isLast != 1 );
               for ( size_t o = 0; o < size; ++o )
                  sum += _layers[ layer + 1 ][ o ].grad * _layers[ layer ][ k ].weights[ o ];
               _layers[ layer ][ k ].grad = f.evaluateDerivative( _layers[ layer ][ k ].s ) * sum;
            }
         }
      }

      /**
       @brief Compute -dE^p/dwjk with E^p= 0.5 * sum_i( doi^p - yoi^p )^2
              and E = sum_p( E^p ), p a pattern, and update the weights
       */
      void _updateWeights( double learningRate, double momentum, double decayRate )
      {
         // now update the weights
         for ( size_t layer = 0; layer < (size_t)( _layers.size() - 1 ); ++layer )
         {
            for ( size_t i = 0; i < (size_t)_layers[ layer ].size(); ++i )
            {
               size_t isLastLayer = ( layer + 1 + 1 ) == (size_t)_layers.size();
               const size_t size = (size_t)_layers[ layer + 1 ].size() - ( isLastLayer != 1 );
               for ( size_t j = 0; j < size; ++j )
               {
                  const double delta = _layers[ layer + 1 ][ j ].grad * _layers[ layer ][ i ].y * learningRate;

                  // normal backprop
                  _layers[ layer ][ i ].weights[ j ] += _layers[ layer ][ i ].olddw[ j ] * momentum +
                                                        delta;

                  // weight decay term
                  _layers[ layer ][ i ].weights[ j ] -= decayRate * _layers[ layer ][ i ].weights[ j ];

                  _layers[ layer ][ i ].olddw[ j ] = delta;
               }
            }
         }
      }

   public:
      /**
       @brief propagate an input vector that must match the size of the network input layer through the
              network, return the output units.
       */
      template <class Vector>
      const core::Buffer1D<double>& propagate( const Vector& v ) const
      {
         _propagate( v );
         size_t lastLayer = static_cast<size_t>( _layers.size() ) - 1;
         for ( size_t n = 0; n < _layersDesc[ lastLayer ]; ++n )
            _result[ n ] = _layers[ lastLayer ][ n ].y;
         return _result;
      }

      /**
       @brief Learn using a backpropagation algorithm
       @note Point & Point2 must define operator[], size()
       @param database the database containing all the samples. Only samples using the validation or learning
              error should be used (although the test error is accessible for test purposes only)
       @param stop the stop condition
       @param learningRate the learning rate of the algorithm. The bigger, the faster the network will learn. If
              too big, the network won't converge, and thus error oscillate. Typical value is small, but depend of
              the problem
       @param momentum a momemtum term is added (a proportion of the delta weight of previous round in learning). Typical value is 0.1
       @param weightDecayRate if not 0, the weight of the network will add a penalty, thus it will try to minize them.
              It could be an alternative to early stopping. If too big, the newtork won't learn, too small, it won't do anything.
       @param reportTimeIntervalInSec specifies the interval in seconds between each logging of the state of the neural network
       @param weights the weights given to each sample in the database, must be in range [0..1]. By default a weight to 1 is given to all samples
       */
      template <class Point, class Point2>
      Result learn( const core::Database< core::ClassificationSample<Point, Point2> >& database, const StopConditionMlp& stop, double learningRate = 0.05, double momentum = 0.1, double weightDecayRate = 0, double reportTimeIntervalInSec = 0.2, const core::Buffer1D<float> weights_ = core::Buffer1D<float>() )
      {
         size_t nbIter = 0;
         _createNetwork();

         double errorT;
         double errorL;
         double errorV;

         ensure( weights_.size() == 0 || weights_.size() == database.size(), "must be empty or each sample must have an associated weight" );
         core::Buffer1D<double> weights( database.size() );
         for ( size_t n = 0; n < weights.size(); ++n )
         {
            weights[ n ] = ( weights_.size() == 0 ) ? 1 : weights_[ n ];
         }

         core::Timer timer;
         do
         {
            errorT = 0;
            errorL = 0;
            errorV = 0;

            typedef core::ClassificationSample<Point, Point2> Sample;
            for ( size_t n = 0; n < database.size(); ++n )
            {
               // run the learning
               switch ( database[ n ].type )
               {
               case Sample::LEARNING:
                  _propagate( database[ n ].input );
                  _computeGrad( database[ n ].output, weights[ n ] );
                  _updateWeights( learningRate, momentum, weightDecayRate );
                  errorL += _errorPattern( database[ n ].output );
                  break;
               case Sample::TESTING:
                  _propagate( database[ n ].input );
                  errorT += _errorPattern( database[ n ].output );
                  break;
               case Sample::VALIDATION:
                  _propagate( database[ n ].input );
                  errorV += _errorPattern( database[ n ].output );
                  break;
               default:
                  unreachable( "Must never be reached" );
               }
            }

            // log messages
            if ( timer.getCurrentTime() >= reportTimeIntervalInSec )
            {
               double weight = _errorWeightDecay();
               std::stringstream s;
               s << "cycle:" << nbIter << " errorL=" << errorL << " errorV=" << errorV << " errorT=" << errorT << " weight:" << weight;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, s.str() );
               timer.start();
            }

            ++nbIter;
         } while ( !stop.stop( nbIter, errorL, errorV, errorT ) );

         // log end of state
         {
            double weight = _errorWeightDecay();
            std::stringstream s;
            s << "cycle:" << nbIter << " errorL=" << errorL << " errorV=" << errorV << " errorT=" << errorT << " weight:" << weight;
            core::LoggerNll::write( core::LoggerNll::INFO, s.str() );
            timer.start();
         }
         Result result =
         {
            errorL,
            errorV,
            errorT
         };
         return result;
      }

      bool write( const std::string& file ) const
      {
         std::ofstream f( file.c_str(), std::ios_base::binary );
         assert( f.is_open() );
         if ( !f.is_open() )
            return false;
         return write( f );
      }

      bool write( std::ostream& f ) const
      {
         core::write<Vectori>( _layersDesc, f );
         core::write<Layers>( _layers, f );
         return true;
      }

      bool read( const std::string& file )
      {
         std::ifstream f( file.c_str(), std::ios_base::binary | std::ios_base::in );
         assert( f.is_open() );
         if ( !f.is_open() )
            return false;
         return read( f );
      }

      bool read( std::istream& f )
      {
         core::read<Vectori>( _layersDesc, f );
         core::read<Layers>( _layers, f );

         // recreate the result vector
         _result = core::Buffer1D<double>( _layersDesc[ (size_t)_layers.size() - 1 ] );

         // reset the bias output to 1
         for ( size_t n = 0; n < (size_t)_layers.size() - 1; ++n )
            _layers[ n ][ _layersDesc[ n ] - 1 ].y = 1;
         return true;
      }

      /**
       @brief return the size of the input layer
       */
      size_t getInputSize() const
      {
         ensure( _layersDesc.size(), "no layer descriptor" );
         return _layersDesc[ 0 ] - 1;
      }

      /**
       @brief return the size of the output layer
       */
      size_t getOutputSize() const
      {
         ensure( _layersDesc.size(), "no layer descriptor" );
         return _layersDesc[ (size_t)_layers.size() - 1 ];
      }

      /**
       @brief reset the weights of a network.
       */
      void reset()
      {
         _createNetwork();
      }

   protected:
      Vectori  _layersDesc;
      Layers   _layers;
      mutable core::Buffer1D<double>  _result; // it is mutable as it doesn't define the network design but a temporary state
   };
}
}

#endif
