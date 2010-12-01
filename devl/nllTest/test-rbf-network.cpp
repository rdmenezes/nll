#include <nll/nll.h>
#include <tester/register.h>
#include "database-builder.h"

using namespace nll;

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
         type evaluate( const Vector& x )
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

      RadialBasisNetwork()
      {}

      /**
       @brief train the network
       @note in the database we expect to have an output as an array and not a class label
       */
      template <class Database>
      void learn( const Database& dat, ui32 nbRbf, double weightLr, double meanLr, double varianceLr, double minLearningError )
      {
         ensure( nbRbf > 1 && weightLr >= 0 && meanLr >= 0 && varianceLr >= 0, "invalid parameter" );

         // work only on the learning set
         Database learning = core::filterDatabase( dat, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );
         if ( learning.size() == 0 )
            return;
         if ( nbRbf > learning.size() )
            nbRbf = learning.size();

         _init( learning, nbRbf );
         _learn( learning, weightLr, meanLr, varianceLr, minLearningError );
      }

      template <class Point>
      Point evaluate( const Point& p )
      {
         Point r( _w.sizey() );
         _forward( p, r );
         return r;
      }

   private:
      // for all RBF, pre-compute its value
      template <class Point>
      void _precomputeRbf( const Point& p, std::vector<type>& precomputedRbf )
      {
         precomputedRbf = std::vector<type>( _rbf.size() );
         for ( size_t n = 0; n < _rbf.size(); ++n )
         {
            precomputedRbf[ n ] = _rbf[ n ].evaluate( p );
         }
      }

      template <class Database>
      void _learn( const Database& dat, double weightLr, double meanLr, double varianceLr, double minLearningError )
      {
         double learningError = 1e10;
         while ( learningError > minLearningError )
         {
            learningError = 0;

            // the gradients to be computed
            Matrix               gradientW( _w.sizey(), _w.sizex() );
            Matrix               gradientMean( _w.sizex(), (ui32)_rbf[ 0 ].mean.size() );
            std::vector<type>    gradientVar( _w.sizex() );
            std::vector<type>    fderivsum( _w.sizey() );
            std::vector<type>    result( _w.sizey() );
            std::vector<type>    diffResult( _w.sizey() );

            // stochastic gradient update
            std::vector<type> precomputedRbf;

            for ( ui32 n = 0; n < dat.size(); ++n )
            {
               // precompute the shared computations
               _precomputeRbf( dat[ n ].input, precomputedRbf );
               for ( ui32 unit = 0; unit < _w.sizey(); ++unit )
               {
                  type sum = 0;
                  for ( ui32 rbf = 0; rbf < _w.sizex(); ++rbf )
                  {
                     sum += _w( unit, rbf ) * precomputedRbf[ rbf ];
                  }
                  fderivsum[ unit ] = _func.evaluateDerivative( sum );
               }

               type sumError = 0;
               _forwardPrecomputed( precomputedRbf, result );
               for ( ui32 unit = 0; unit < _w.sizey(); ++unit )
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
               for ( ui32 n = 0; n < _w.size(); ++n )
               {
                  _w[ n ] += - weightLr * gradientW[ n ];
               }

               for ( ui32 rbf = 0; rbf < _w.sizex(); ++rbf )
               {
                  for ( ui32 n = 0; n < _rbf[ rbf ].mean.size(); ++n )
                  {
                     _rbf[ rbf ].mean[ n ] += - meanLr * gradientMean( rbf, n );
                  }
               }

               for ( ui32 rbf = 0; rbf < _w.sizex(); ++rbf )
               {
                  _rbf[ rbf ].var = fabs( _rbf[ rbf ].var - varianceLr * gradientVar[ rbf ] );
               }
            }
            std::cout << "error=" << learningError << std::endl;
         }
      }

      template <class Point>
      void _computeGradientCenter( const Point& p, const std::vector<type>& precomputedRbf, const std::vector<type>& fderivsum, const std::vector<type>& diffResult, Matrix& gradient )
      {
         for ( ui32 rbf = 0; rbf < _w.sizex(); ++rbf )
         {
            const type val = - precomputedRbf[ rbf ] / ( _rbf[ rbf ].var );
            for ( ui32 cmp = 0; cmp < _rbf[ rbf ].mean.size(); ++cmp )
            {
               type sum = 0;
               for ( ui32 unit = 0; unit < _w.sizey(); ++unit )
               {
                  sum += diffResult[ unit ] * fderivsum[ unit ] * _w( unit, rbf );
               }
               gradient( rbf, cmp ) = val * ( p[ cmp ] - _rbf[ rbf ].mean[ cmp ] ) * sum;
            }
         }
      }

      void _computeGradientVar( const std::vector<type>& precomputedRbf, const std::vector<type>& fderivsum, const std::vector<type>& diffResult, std::vector<type>& gradient )
      {
         for ( ui32 rbf = 0; rbf < _w.sizex(); ++rbf )
         {
            const type val = precomputedRbf[ rbf ] * 2 / _rbf[ rbf ].var * log( precomputedRbf[ rbf ] );
            type sum = 0;
            for ( ui32 unit = 0; unit < _w.sizey(); ++unit )
            {
               sum += diffResult[ unit ] * fderivsum[ unit ] * _w( unit, rbf );
            }
            gradient[ rbf ] = val * sum;
         }
      }

      void _computeGradientWeight( const std::vector<type>& precomputedRbf, const std::vector<type>& fderivsum, const std::vector<type>& diffResult, Matrix& gradient )
      {
         for ( ui32 unit = 0; unit < _w.sizey(); ++unit )
         {
            const double val = - diffResult[ unit ] * fderivsum[ unit ];
            for ( ui32 rbf = 0; rbf < _w.sizex(); ++rbf )
            {
               gradient( unit, rbf ) = val * precomputedRbf[ rbf ];
            }
         }
      }

      template <class Point>
      void _forward( const Point& p, Point& units )
      {
         assert( units.size() == _w.sizey() );
         std::vector<type> precomputed;
         _precomputeRbf( p, precomputed );

         for ( ui32 unit = 0; unit < _w.sizey(); ++unit )
         {
            type sum = 0;
            for ( ui32 rbf = 0; rbf < _w.sizex(); ++rbf )
            {
               sum += precomputed[ rbf ] * _w( unit, rbf );
            }
            units[ unit ] = _func.evaluate( sum );
         }
      }

      void _forwardPrecomputed( const std::vector<type>& precomputed, std::vector<type>& units )
      {
         assert( units.size() == _w.sizey() );
         for ( ui32 unit = 0; unit < _w.sizey(); ++unit )
         {
            type sum = 0;
            for ( ui32 rbf = 0; rbf < _w.sizex(); ++rbf )
            {
               sum += precomputed[ rbf ] * _w( unit, rbf );
            }
            units[ unit ] = _func.evaluate( sum );
         }
      }

      // we do a very rough initialization of the RBF
      template <class Database>
      void _init( const Database& dat, ui32 nbRbf )
      {
         // init the RBF centers using k-means
         // first see the database as a collection of points
         typedef core::DatabaseInputAdapterRead<Database> Adapter;
         typedef typename Adapter::Point                  Point;

         // configure kmeans
         Adapter reader( dat );
         typedef BuildKMeansUtility<Point, MetricEuclidian<Point>, Adapter>  Utility;

         MetricEuclidian<Point> metric;
         const ui32 nbDim = dat[ 0 ].input.size();
         Utility utility( nbDim, metric );
         typedef KMeans<Utility> KMeansConf;
         KMeansConf kmeans( utility );

         // get the centroids
         typename KMeansConf::KMeansResult results = kmeans( reader, nbRbf );
         _rbf = std::vector<Rbf>( nbRbf );

         for ( ui32 n = 0; n < nbRbf; ++n )
         {
            _rbf[ n ].mean = std::vector<type>( nbDim );
            for ( ui32 nn = 0; nn < nbDim; ++nn )
            {
               const Point& point = results.second[ n ];
               _rbf[ n ].mean[ nn ] = point[ nn ];
            }
         }

         // measure the mean distance between centroids
         // TODO: improvement: use the distance for each cluster instead of global
         double d = 0;
         for ( ui32 n = 1; n < nbRbf; ++n )
         {
            double sum = 0;

            const std::vector<type>& p1 = _rbf[ 0 ].mean;
            const std::vector<type>& p2 = _rbf[ n ].mean;
            for ( ui32 nn = 0; nn < nbDim; ++nn )
            {
               const type diff = p1[ nn ] - p2[ nn ];
               sum += core::sqr( diff );
            }

            d += sqrt( sum );
         }

         const type var = std::max( 0.1, d / ( nbRbf - 1 ) );
         std::cout << "initial variance=" << var << std::endl;

         for ( ui32 n = 0; n < nbRbf; ++n )
         {
            _rbf[ n ].var = var;
         }

         // init the weights with small random weights
         _w = Matrix( dat[ 0 ].output.size(), nbRbf );
         for ( ui32 n = 0; n < _w.size(); ++n )
         {
            _w[ n ] = core::generateUniformDistribution( -0.05, 0.05 );
            while ( _w[ n ] == 0 )
            {
               _w[ n ] = core::generateUniformDistribution( -0.05, 0.05 );
            }
         }
      }

   private:
      std::vector<Rbf>  _rbf;
      Matrix            _w;      // w( k, j ), k = the output unit, j = the RBF unit
      DFunction         _func;
   };
}
}

class TestRBFNetwork
{
   typedef nll::core::Buffer1D<double>             Point;
   typedef nll::algorithm::ClassifierMlp<Point>    Mlp;
   typedef Mlp::Database                           Database;

public:
   void testSimple()
   {
      srand( (int)time( 0 ) );
      typedef core::Database< core::ClassificationSample<Point, Point> > DatabaseRbf;

      Database dat = loadDatabaseSpect<Point>();
      const ui32 nbClass = core::getNumberOfClass( dat );

      // encode the class to an array [0..nbClass]
      DatabaseRbf datRbf;
      for ( ui32 n = 0; n < dat.size(); ++n )
      {
         Point out( nbClass );
         for ( ui32 nn = 0; nn < nbClass; ++nn )
         {
            out[ nn ] = ( dat[ n ].output == nn );               
         }
         datRbf.add( DatabaseRbf::Sample( dat[ n ].input, out, (DatabaseRbf::Sample::Type)dat[ n ].type ) );
      }

      //algorithm::RadialBasisNetwork<algorithm::FunctionSimpleIdentity> rbf;
      algorithm::RadialBasisNetwork<> rbf;

      const double param = 1;
      rbf.learn( datRbf, 20, param, param, param, 10 );
   }

   void testApproximation()
   {
      srand( (int)time( 0 ) );
      typedef core::Database< core::ClassificationSample<Point, Point> > DatabaseRbf;

      // encode the class to an array [0..nbClass]
      DatabaseRbf datRbf;
      int nn = 0;
      for ( double n = 0; n < 1000; ++n, ++nn )
      {
         const double val = n / 1000;
         datRbf.add( DatabaseRbf::Sample( core::make_buffer1D<double>( val ), 
                                          core::make_buffer1D<double>( cos( val ) ),
                                          (DatabaseRbf::Sample::Type)(nn%2) ) );
      }

      //algorithm::RadialBasisNetwork<algorithm::FunctionSimpleIdentity> rbf;
      algorithm::RadialBasisNetwork<> rbf;

      const double param = 0.5;
      rbf.learn( datRbf, 15, param, param, param, 0.01 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRBFNetwork);
//TESTER_TEST(testSimple);
TESTER_TEST(testApproximation);
TESTER_TEST_SUITE_END();
#endif
