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

#ifndef NLL_ALGORITHM_LINEAR_REGRESSION_H_
# define NLL_ALGORITHM_LINEAR_REGRESSION_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Regression algorithm that fits a linear model
    @see http://en.wikipedia.org/wiki/Linear_regression
    */
   class LinearRegression
   {
   public:
      typedef core::Buffer1D<double>                                 Vector;

   private:
      template <class Database>
      class FunctionRegression : public ObjectiveFunction<Database>
      {
         typedef typename Database::Sample::Input Point;

      public:
         FunctionRegression( const Database& learningDatabase, const std::vector<float>& weights ) : ObjectiveFunction( learningDatabase ), _weights( weights )
         {}

         /**
            @brief Given our model <theta>, Compute the gradient
            */
         virtual core::Buffer1D<double> computeGradient( const core::Buffer1D<double>& theta, ui32 index ) const
         {
            core::Buffer1D<double> g( theta.size() );
            const ui32 nbInput = _learningDatabase[ 0 ].input.size();

            const double error = test( theta, _learningDatabase[ index ].input ) - _learningDatabase[ index ].output;
            const double gradient = _weights[ index ] * error / _learningDatabase.size();

            const Point& p = _learningDatabase[ index ].input;
            for ( ui32 n = 0; n < nbInput; ++n )
            {
               g[ n ] = gradient * p[ n ];
               g[ nbInput ] = gradient * 1;
            }

            return g;
         }

         virtual double computeCostFunction( const core::Buffer1D<double>& theta ) const
         {
            double accum = 0;
            for ( ui32 n = 0; n < theta.size(); ++n )
            {
               accum += core::sqr( test( theta, _learningDatabase[ n ].input ) - _learningDatabase[ n ].output );
            }
            return accum / 2;
         }

      private:
         double test( const core::Buffer1D<double>& theta, const Point& p ) const
         {
            double accum = 0;

            ensure( p.size() + 1 == theta.size(), "wrong size!" );
            for ( ui32 n = 0; n < p.size(); ++n )
            {
               accum += p[ n ] * theta[ n ];
            }
            return accum + theta[ p.size() ];
         }

      private:
         const std::vector<float>&        _weights;
      };

   public:
      template <class Database>
      void learn( const Database& dat, ui32 nbIteration, double learningRate = 0.01f, const core::Buffer1D<float> _weights = core::Buffer1D<float>() )
      {
         // filter the database
         Database learning = core::filterDatabase( dat, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );
         if ( learning.size() == 0 )
         {
            return;
         }

         // extract the weights
         std::vector<float> weights;
         if ( _weights.size() == 0 )
         {
            weights = std::vector<float>( learning.size(), 1.0f );
         } else {
            weights.reserve( learning.size() );
            for ( ui32 n = 0; n < dat.size(); ++n )
            {
               if ( dat[ n ].type == Database::Sample::LEARNING )
               {
                  weights.push_back( _weights[ n ] );
               }
            }
         }

         // run the gradient descent
         typedef FunctionOptimizationGradientDescentStochastic<Database>   Optimizer;
         Optimizer optimizer( learningRate );
         _w = Vector( learning[ 0 ].input.size() + 1 );
         FunctionRegression<Database> function( learning, weights );
         _w = optimizer.compute( function, _w, nbIteration );
      }

      template <class Point>
      double test( const Point& point ) const
      {
         ensure( _w.size() && point.size() + 1 == _w.size(), "size don't match" );
         const ui32 size = static_cast<ui32>( point.size() );

         double accum = 0;
         for ( ui32 n = 0; n < size; ++n )
         {
            accum += point[ n ] * _w[ n ];
         }
         accum += _w[ size ];
         return accum;
      }

      const Vector& getParams() const
      {
         return _w;
      }

   private:
      Vector   _w;      // bias is stored at the last index _w[ _w.size() - 1 ]
   };
}
}

#endif