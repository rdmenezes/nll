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

#ifndef NLL_ALGORITHM_RBF_NETWORK_H_
# define NLL_ALGORITHM_RBF_NETWORK_H_

namespace nll
{
namespace algorithm
{
   class FunctionSimpleIdentity
   {
   public:
      inline double evaluate( double val ) const
      {
         return val;
      }

      inline double evaluateDerivative( double ) const
      {
         return 1;
      }
   };

   /**
    @brief Radial basis function neural network, using gaussian function as basis functions

    For partial implementation details see http://www.cs.iastate.edu/~jtian/cs474/fall06/lectures/lecture6-RBF.pdf
    Instead of taking Zk = sum_j ( Wkj * RBFj ) we take Zk = f( sum_j ( Wkj * RBFj ) ) so that the units are bounded,
    essentially because we want to do some classification. In case this is used for general function approximation,
    simply set f = identity
    */
   template <class DFunction = FunctionSimpleDifferenciableSigmoid>
   class RadialBasisNetwork
   {
   public:
      typedef double             type;    // defines the internal type to be used
      typedef core::Matrix<type> Matrix;

   public:
      /**
       @brief Gaussian basis function
       */
      struct Rbf
      {
         std::vector<type> mean;
         type              var;

         /**
          @brief evaluate the RBF at point x
          */
         template <class Vector>
         type evaluate( const Vector& x ) const
         {
            assert( x.size() == mean.size() );
            type accum = 0;
            for ( size_t n = 0; n < mean.size(); ++n )
            {
               const type diff = mean[ n ] - x[ n ];
               accum += diff * diff;
            }

            return exp( - accum / ( 2 * var ) );
         }
      };

      class RadialBasisNetworkMonitoring
      {
      public:
         virtual ~RadialBasisNetworkMonitoring()
         {}

         virtual void monitor( const std::vector<Rbf>& rbfs, const Matrix& weights ) const = 0;
      };

      RadialBasisNetwork( const RadialBasisNetworkMonitoring* monitoring = 0 )
      {
         _monitoring = monitoring;
      }

      void setMonitoring( const RadialBasisNetworkMonitoring* monitoring )
      {
         _monitoring = monitoring;
      }

      /**
       @brief train the network
       @note in the database we expect to have an output as an array and not a class label
       */
      template <class Database>
      double learn( const Database& dat, size_t nbRbf, double weightLr, double meanLr, double varianceLr, double minLearningError, double timeout, double reportTimeIntervalInSec = 0.1, bool skipInit = false )
      {
         ensure( nbRbf > 1 && weightLr >= 0 && meanLr >= 0 && varianceLr >= 0, "invalid parameter" );

         // work only on the learning set
         Database learning = core::filterDatabase( dat, core::make_vector<size_t>( (size_t) Database::Sample::LEARNING ), (size_t) Database::Sample::LEARNING );
         if ( learning.size() == 0 )
            return 0;
         if ( nbRbf > learning.size() )
            nbRbf = learning.size();

         std::stringstream ss;
         ss << "starting RBF learning:" << std::endl
            << "nb RBF=" << nbRbf << std::endl
            << "weight learning rate=" << weightLr << std::endl
            << "mean learning rate=" << meanLr << std::endl
            << "variance learning rate=" << varianceLr;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );

         if ( !skipInit )
         {
            _init( learning, nbRbf );
         } else {
            ensure( _rbf.size() == nbRbf, "if initialization is skipped, current parameters must match!" );
         }

         return _learn( learning, weightLr, meanLr, varianceLr, minLearningError, timeout, reportTimeIntervalInSec );
      }

      /**
       @brief Evaluate a point, and return ( y_k = f( sum_j WkjRbfj ) )
       */
      template <class Point>
      Point evaluate( const Point& p ) const
      {
         Point r( _w.sizey() );
         _forward( p, r );
         return r;
      }

      /**
       @brief Returns the RBF used to model the data
       */
      const std::vector<Rbf>& getRbfs() const
      {
         return _rbf;
      }

      /**
       @brief returns the weights to assign to each gaussian for each unit

       The weights are stored as w( outputUnit, RBF )
       */
      const Matrix& getWeights() const
      {
         return _w;
      }

      void write( std::ostream& f ) const
      {
         size_t nbRbf = (size_t)_rbf.size();
         core::write<size_t>( nbRbf, f );
         for ( size_t n = 0; n < nbRbf; ++n )
         {
            core::write< std::vector<type> >( _rbf[ n ].mean, f );
            core::write< type >( _rbf[ n ].var, f );
         }
         _w.write( f );
      }

      void read( std::istream& f )
      {
         size_t nbRbf = 0;
         core::read<size_t>( nbRbf, f );
         if ( nbRbf == 0 )
            return;
         _rbf = std::vector<Rbf>( nbRbf );
         for ( size_t n = 0; n < nbRbf; ++n )
         {
            core::read< std::vector<type> >( _rbf[ n ].mean, f );
            core::read< type >( _rbf[ n ].var, f );
         }
         _w.read( f );
      }

   private:
      // for all RBF, pre-compute its value
      template <class Point>
      void _precomputeRbf( const Point& p, std::vector<type>& precomputedRbf ) const
      {
         precomputedRbf = std::vector<type>( _rbf.size() );
         for ( size_t n = 0; n < _rbf.size(); ++n )
         {
            precomputedRbf[ n ] = _rbf[ n ].evaluate( p );
         }
      }

      template <class Database>
      double _learn( const Database& dat, double weightLr, double meanLr, double varianceLr, double minLearningError, double timeout, double reportTimeIntervalInSec )
      {
         core::Timer timer;
         core::Timer start;
         size_t cycle = 0;
         double learningError = 1e10;
         while ( learningError > minLearningError && start.getCurrentTime() < timeout )
         {
            learningError = 0;

            // the gradients to be computed
            Matrix               gradientW( _w.sizey(), _w.sizex() );
            Matrix               gradientMean( _w.sizex(), (size_t)_rbf[ 0 ].mean.size() );
            std::vector<type>    gradientVar( _w.sizex() );
            std::vector<type>    fderivsum( _w.sizey() );
            std::vector<type>    result( _w.sizey() );
            std::vector<type>    diffResult( _w.sizey() );

            // stochastic gradient update
            std::vector<type> precomputedRbf;
            for ( size_t n = 0; n < dat.size(); ++n )
            {
               // precompute the shared computations
               _precomputeRbf( dat[ n ].input, precomputedRbf );
               for ( size_t unit = 0; unit < _w.sizey(); ++unit )
               {
                  type sum = 0;
                  for ( size_t rbf = 0; rbf < _w.sizex(); ++rbf )
                  {
                     sum += _w( unit, rbf ) * precomputedRbf[ rbf ];
                  }
                  fderivsum[ unit ] = _func.evaluateDerivative( sum );
               }

               type sumError = 0;
               _forwardPrecomputed( precomputedRbf, result );
               for ( size_t unit = 0; unit < _w.sizey(); ++unit )
               {
                  const type diff = dat[ n ].output[ unit ] - result[ unit ];
                  diffResult[ unit ] = diff;
                  sumError += diff * diff;
               }
               learningError += sqrt( sumError );

               // compute the gradients
               _computeGradientWeight( precomputedRbf, fderivsum, diffResult, gradientW );
               _computeGradientCenter( dat[ n ].input, precomputedRbf, fderivsum, diffResult, gradientMean );
               _computeGradientVar( precomputedRbf, fderivsum, diffResult, gradientVar );

               
               // update the free parameters
               for ( size_t n = 0; n < _w.size(); ++n )
               {
                  _w[ n ] += - weightLr * gradientW[ n ];
               }

               for ( size_t rbf = 0; rbf < _w.sizex(); ++rbf )
               {
                  for ( size_t n = 0; n < _rbf[ rbf ].mean.size(); ++n )
                  {
                     _rbf[ rbf ].mean[ n ] += - meanLr * gradientMean( rbf, n );
                  }
               }

               for ( size_t rbf = 0; rbf < _w.sizex(); ++rbf )
               {
                  type newVar = fabs( _rbf[ rbf ].var - varianceLr * gradientVar[ rbf ] );
                  _rbf[ rbf ].var = std::max( 1e-10, newVar );
               }
            }

            // log messages
            if ( cycle == 0 || timer.getCurrentTime() >= reportTimeIntervalInSec )
            {
               std::stringstream ss;
               ss << "cycle:" << cycle << " training error=" << learningError;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
               timer.start();

               if ( _monitoring )
                  _monitoring->monitor( _rbf, _w );
            }

            ++cycle;
         }

         // log the last step
         std::stringstream ss;
         ss << "cycle:" << cycle << " training error=" << learningError;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );

         return learningError;
      }

      template <class Point>
      void _computeGradientCenter( const Point& p, const std::vector<type>& precomputedRbf, const std::vector<type>& fderivsum, const std::vector<type>& diffResult, Matrix& gradient )
      {
         for ( size_t rbf = 0; rbf < _w.sizex(); ++rbf )
         {
            const type val = - precomputedRbf[ rbf ] / ( _rbf[ rbf ].var );
            for ( size_t cmp = 0; cmp < _rbf[ rbf ].mean.size(); ++cmp )
            {
               type sum = 0;
               for ( size_t unit = 0; unit < _w.sizey(); ++unit )
               {
                  sum += diffResult[ unit ] * fderivsum[ unit ] * _w( unit, rbf );
               }
               gradient( rbf, cmp ) = val * ( p[ cmp ] - _rbf[ rbf ].mean[ cmp ] ) * sum;
            }
         }
      }

      void _computeGradientVar( const std::vector<type>& precomputedRbf, const std::vector<type>& fderivsum, const std::vector<type>& diffResult, std::vector<type>& gradient )
      {
         for ( size_t rbf = 0; rbf < _w.sizex(); ++rbf )
         {
            const type val = precomputedRbf[ rbf ] * 2 / _rbf[ rbf ].var * log( precomputedRbf[ rbf ] );
            type sum = 0;
            for ( size_t unit = 0; unit < _w.sizey(); ++unit )
            {
               sum += diffResult[ unit ] * fderivsum[ unit ] * _w( unit, rbf );
            }
            gradient[ rbf ] = val * sum;
         }
      }

      void _computeGradientWeight( const std::vector<type>& precomputedRbf, const std::vector<type>& fderivsum, const std::vector<type>& diffResult, Matrix& gradient )
      {
         for ( size_t unit = 0; unit < _w.sizey(); ++unit )
         {
            const double val = - diffResult[ unit ] * fderivsum[ unit ];
            for ( size_t rbf = 0; rbf < _w.sizex(); ++rbf )
            {
               gradient( unit, rbf ) = val * precomputedRbf[ rbf ];
            }
         }
      }

      template <class Point>
      void _forward( const Point& p, Point& units ) const
      {
         assert( units.size() == _w.sizey() );
         std::vector<type> precomputed;
         _precomputeRbf( p, precomputed );

         for ( size_t unit = 0; unit < _w.sizey(); ++unit )
         {
            type sum = 0;
            for ( size_t rbf = 0; rbf < _w.sizex(); ++rbf )
            {
               sum += precomputed[ rbf ] * _w( unit, rbf );
            }
            units[ unit ] = _func.evaluate( sum );
         }
      }

      void _forwardPrecomputed( const std::vector<type>& precomputed, std::vector<type>& units ) const
      {
         assert( units.size() == _w.sizey() );
         for ( size_t unit = 0; unit < _w.sizey(); ++unit )
         {
            type sum = 0;
            for ( size_t rbf = 0; rbf < _w.sizex(); ++rbf )
            {
               sum += precomputed[ rbf ] * _w( unit, rbf );
            }
            units[ unit ] = _func.evaluate( sum );
         }
      }

      // we do a very rough initialization of the RBF
      template <class Database>
      void _init( const Database& dat, size_t nbRbf )
      {
         // init the RBF centers using k-means
         // first see the database as a collection of points
         typedef core::DatabaseInputAdapterRead<Database> Adapter;
         typedef typename Adapter::Point                  Point;

         // configure kmeans
         Adapter reader( dat );
         typedef BuildKMeansUtility<Point, MetricEuclidian<Point>, Adapter>  Utility;

         MetricEuclidian<Point> metric;
         const size_t nbDim = dat[ 0 ].input.size();
         Utility utility( nbDim, metric );
         typedef KMeans<Utility> KMeansConf;
         KMeansConf kmeans( utility );

         // get the centroids
         typename KMeansConf::KMeansResult results = kmeans( reader, nbRbf );
         _rbf = std::vector<Rbf>( nbRbf );

         for ( size_t n = 0; n < nbRbf; ++n )
         {
            _rbf[ n ].mean = std::vector<type>( nbDim );
            for ( size_t nn = 0; nn < nbDim; ++nn )
            {
               const Point& point = results.second[ n ];
               _rbf[ n ].mean[ nn ] = point[ nn ];
            }
         }

         // measure the mean distance between centroids
         // TODO: improvement: use the distance for each cluster instead of global
         double d = 0;
         for ( size_t n = 1; n < nbRbf; ++n )
         {
            double sum = 0;

            const std::vector<type>& p1 = _rbf[ 0 ].mean;
            const std::vector<type>& p2 = _rbf[ n ].mean;
            for ( size_t nn = 0; nn < nbDim; ++nn )
            {
               const type diff = p1[ nn ] - p2[ nn ];
               sum += core::sqr( diff );
            }

            d += sqrt( sum );
         }

         const type var = std::max( 0.1, d / ( nbRbf - 1 ) );

         std::stringstream ss;
         ss << "initial variance=" << var;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );

         for ( size_t n = 0; n < nbRbf; ++n )
         {
            _rbf[ n ].var = var;
         }

         // init the weights with small random weights
         _w = Matrix( (size_t)dat[ 0 ].output.size(), nbRbf );
         for ( size_t n = 0; n < _w.size(); ++n )
         {
            _w[ n ] = core::generateUniformDistribution( -0.1, 0.1 );
            while ( _w[ n ] == 0 )
            {
               _w[ n ] = core::generateUniformDistribution( -0.1, 0.1 );
            }
         }
      }

   private:
      std::vector<Rbf>  _rbf;
      Matrix            _w;      // w( k, j ), k = the output unit, j = the RBF unit
      DFunction         _func;
      const RadialBasisNetworkMonitoring* _monitoring;
   };
}
}

#endif