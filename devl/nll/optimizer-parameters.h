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

#ifndef NLL_OPTIMIZER_PARAMETERS_H_
# define NLL_OPTIMIZER_PARAMETERS_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Specify the parameter behaviour: range, generation, search, and iteration...

    It is used to define how a parameter should be optimized
    */
   class ParameterOptimizer
   {
   public:
      /**
       @brief each parameter must be bounded in space. Outside of the space, parameters are considered invalid.
       */
      ParameterOptimizer( double min, double max ) : _min( min ), _max( max )
      {
         ensure( max >= min, "max must be >= min" );
      }

      virtual ~ParameterOptimizer()
      {}

      /**
       @brief Define the next value of <code>val</code>.
       */
      virtual double next( double val ) const = 0;

      /**
       @brief Clone the parameter. It must be deallocated by <code>delete</code>
       */
      virtual ParameterOptimizer* clone() const = 0;

      /**
       @brief Randomly generate a value for this parameter.
      */
      virtual double generate() const = 0;

      /**
       @brief Randomly modify a value. Return a value based on the argument
       */
      virtual double modify( double val ) const = 0;

      /**
       @brief Get the min value of this parameter
       */
      double getMin() const { return _min; }

      /**
       @brief Get the max value of this parameter
       */
      double getMax() const { return _max; }

      /**
       @brief Set the max value for this parameter
       */
      void setMin( double min )
      {
         _min = min;
      }

      /**
       @brief Set the min value for this parameter
       */
      void setMax( double max )
      {
         _max = max;
      }

   protected:
      double   _min;
      double   _max;
   };

   /**
    @brief A sequence of parameters.
    */
   class ParameterOptimizers
   {
      typedef std::vector<ParameterOptimizer*>  Container;

   public:
      ParameterOptimizers( ui32 size, bool owns = true ) : _container( size ), _owns( owns )
      {}
      ParameterOptimizers( bool owns = true ) : _owns( owns )
      {}
      ~ParameterOptimizers()
      {
         _free();
      }
      void push_back( ParameterOptimizer* param )
      {
         assert( param );
         _container.push_back( param );
      }
      const ParameterOptimizer& operator[]( ui32 n ) const
      {
         return *_container[ n ];
      }
      ParameterOptimizer& operator[]( ui32 n )
      {
         return *_container[ n ];
      }
      ui32 size() const
      {
         return static_cast<ui32>( _container.size() );
      }

      ParameterOptimizers( const ParameterOptimizers& params )
      {
         _copy( params );
      }

      ParameterOptimizers& operator=( const ParameterOptimizers& params )
      {
         _copy( params );
         return *this;
      }

   private:
      void _copy( const ParameterOptimizers& params )
      {
         _free();
         _owns = true;
         for ( ui32 n = 0; n < params.size(); ++n )
            _container.push_back( params[ n ].clone() );
      }

      void _free()
      {
         if ( _owns )
         {
            for ( ui32 n = 0; n < static_cast<ui32>( _container.size() ); ++n )
               delete _container[ n ];
         }
      }

   private:
      Container      _container;
      bool           _owns;
   };

   

   /**
    @ingroup algorithm
    @brief Define a base class will generate a gaussian parameter
    */
   class ParameterOptimizerGaussian : public ParameterOptimizer
   {
   public:
      ParameterOptimizerGaussian( double min, double max, double mean, double variance ) :
         ParameterOptimizer( min, max ), _mean( mean ), _variance( variance )
      {}

      virtual double generate() const
      {
         double val = core::generateGaussianDistribution( _mean, _variance );
         return NLL_BOUND( val, _min, _max );
      }

   protected:
      double   _mean;
      double   _variance;
   };

   /**
    @ingroup algorithm
    @brief Define a parameter following a geometric progression like.
    */
   class ParameterOptimizerGaussianGeometric : public ParameterOptimizerGaussian
   {
   public:
      /**
       @brief construct the sequence parameters
       @param min the min value
       @param max the max value
       @param mean the mean value (when randomly generated)
       @param variance the mean value (when randomly generated)
       @param ratioIncrease used in <code>modify</code>, the next value would follow <code>v_next = v * ratioIncrease</code>, in the case the value is increased
       @param ratioDecrease used in <code>modify</code>, the next value would follow <code>v_next = v * ratioDecrease</code>, in the case the value is decreased
       */
      ParameterOptimizerGaussianGeometric( double min, double max, double mean, double variance, double ratioIncrease, double ratioDecrease ) :
         ParameterOptimizerGaussian( min, max, mean, variance ), _ratioIncrease( ratioIncrease ), _ratioDecrease( ratioDecrease )
      {}

      virtual double next( double val ) const
      {
         return val * _ratioIncrease;
      }

      virtual double modify( double val ) const
      {
         // generate a point following a normal distribution (1, 0.25)
         // so the modifier will vary from 0.75 to 1.25
         double modifier = core::generateGaussianDistribution( 1, 0.25 );
         double v = ( modifier >= 1 ) ? val * _ratioIncrease * modifier : val * _ratioDecrease * modifier;
         return NLL_BOUND( v, _min, _max );
      }

      virtual ParameterOptimizer* clone() const
      {
         return new ParameterOptimizerGaussianGeometric( _min, _max, _mean, _variance, _ratioIncrease, _ratioDecrease );
      }

   protected:
      double   _ratioIncrease;
      double   _ratioDecrease;
   };

   /**
    @ingroup algorithm
    @brief Define a parameter following a linear progression like.
    */
   class ParameterOptimizerGaussianLinear : public ParameterOptimizerGaussian
   {
   public:
      /**
       @brief construct the sequence parameters
       @param min the min value
       @param max the max value
       @param mean the mean value (when randomly generated)
       @param variance the mean value (when randomly generated)
       @param granularity define the granularity of the search space ( all modification will we > <code>granularityMin</code> )
       */
      ParameterOptimizerGaussianLinear( double min, double max, double mean, double variance, double granularity ) :
         ParameterOptimizerGaussian( min, max, mean, variance ), _granularity( granularity )
      {}

      /**
       @brief define the next value as <code>next = val + granularityMin</code>
       */
      virtual double next( double val ) const
      {
         return val + _granularity;
      }

      virtual double modify( double val ) const
      {
         double modifier = fabs( core::generateGaussianDistribution( 1, 0.35 ) );
         double sign = ( rand() % 2 ) ? -1 : 1;
         double v = val + sign * modifier * _granularity;
         return NLL_BOUND( v, _min, _max );
      }

      virtual ParameterOptimizer* clone() const
      {
         return new ParameterOptimizerGaussianLinear( _min, _max, _mean, _variance, _granularity );
      }

   protected:
      double   _granularity;
   };

   /**
    @ingroup algorithm
    @brief Handle the parameter as an Integer
    */
   class ParameterOptimizerInteger : public ParameterOptimizer
   {
   public:
      ParameterOptimizerInteger( double min, double max ) : ParameterOptimizer( min, max )
      {}
      virtual double next( double val ) const
      {
         return val + 1;
      }

      virtual double modify( double val ) const
      {
         double sign = ( rand() % 2 ) ? 1 : -1;
         val += sign;
         return NLL_BOUND( val, _min, _max );
      }

      virtual double generate() const
      {
         unsigned range = static_cast<unsigned>( _max - _min );
         double sign = ( rand() % 2 ) ? 1 : -1;
         return sign * ( rand() % range ) + _min;
      }

      virtual ParameterOptimizer* clone() const
      {
         return new ParameterOptimizerInteger( _min, _max );
      }
   };

   /**
    @ingroup algorithm
    @brief Handle the parameter as an Integer
    */
   class ParameterOptimizerGaussianInteger : public ParameterOptimizerGaussian
   {
   public:
      ParameterOptimizerGaussianInteger( double min, double max, double mean, double variance, double granularity ) :
         ParameterOptimizerGaussian( min, max, mean, variance ), _granularity( granularity )
      {}

      virtual double next( double val ) const
      {
         return val + _granularity;
      }

      virtual double modify( double val ) const
      {
         double sign = ( rand() % 2 ) ? 1 : -1;
         val += sign * _granularity;
         return NLL_BOUND( val, _min, _max );
      }

      virtual ParameterOptimizer* clone() const
      {
         return new ParameterOptimizerGaussianInteger( _min, _max, _mean, _variance, _granularity );
      }

   protected:
      double   _granularity;
   };
}
}

#endif
