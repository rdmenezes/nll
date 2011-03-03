#include "stdafx.h"
#include <nll/nll.h>
#include <tester/register.h>
#include "database-benchmark.h"

using namespace nll;

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Restricted Boltzmann machine with binary activation units 
    @see contrastive divergence proof www.robots.ox.ac.uk/~ojw/files/NotesOnCD.pdf
         RBM http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.35.8613
             http://www.cs.toronto.edu/~hinton/ucltutorial.pdf
    */
   class RestrictedBoltzmannMachineBinary
   {
   public:
      typedef double                   type;
      typedef core::Matrix<type>       Matrix;
      typedef core::Buffer1D<type>     Vector;
      typedef core::Buffer1D<ui32>     Index;
      typedef core::Buffer1D<bool>     VectorB;

   public:
      /**
       @brief Train a restricted boltzmann machine using contrastive divergence
       @param points the points to use - these must be in the [0..1] interval so that they can be interpreted as probability
       @param nbHiddenStates the size of the hidden layer
       @param learningRate the learning rate used to update the weights
       @param nbEpoch the number of iterations that should be used to train the RBM
       @param batchSize the number of samples used to updated the weights
       @return the reconstruction error
       */
      template <class Points>
      double trainContrastiveDivergence( const Points& points, ui32 nbHiddenStates, type learningRate, ui32 nbEpoch,
                                         ui32 batchSize = 100 )
      {
         const int inputSize = points[ 0 ].size();
         const ui32 nbBatches = (ui32)std::ceil( static_cast<type>( points.size() ) / batchSize );
         const FunctionSimpleDifferenciableSigmoid sigm;
         core::Timer timer;

         {
            std::stringstream ss;
            ss << "train binary restricted boltzmann machine" << std::endl
               << "visible units=" << inputSize << std::endl
               << "hidden units=" << nbHiddenStates << std::endl
               << "learning rate=" << learningRate << std::endl
               << "nbEpoch=" << nbEpoch << std::endl
               << "batchSize=" << batchSize << std::endl;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         ensure( nbBatches > 0, "invalid batch size" );
         ensure( nbHiddenStates > 0 && learningRate > 0 && nbEpoch > 0 && batchSize > 0, "invalid parameters" );

         // create the batches
         std::vector< std::vector<ui32> > batchList( nbBatches );
         for ( ui32 n = 0; n < nbBatches; ++n )
            batchList[ n ].reserve( batchSize );
         Index index( points.size() );
         for ( ui32 n = 0; n < points.size(); ++n )
            index[ n ] = n / batchSize;
         core::randomize( index, 2 );
         for ( ui32 n = 0; n < points.size(); ++n )
            batchList[ index[ n ] ].push_back( n );

         // initialize
         // we include the bias = 1, it is located at the end of the hidden and visible layer
         Vector hstates( nbHiddenStates + 1 );
         Vector vstates( inputSize + 1 );
         Vector vstatesOrig( inputSize + 1 );
         Vector tmpstates( std::max<ui32>( inputSize + 1, nbHiddenStates + 1 ) );

         hstates[ nbHiddenStates ] = 1;
         vstates[ inputSize ] = 1;
         vstatesOrig[ inputSize ] = 1;

         _w = Matrix( inputSize + 1, nbHiddenStates + 1 );
         for ( ui32 n = 0; n < _w.size(); ++n )
            _w[ n ] = (type)core::generateUniformDistribution( 1e-4, 2e-1 );

         type error = 0;
         for ( ui32 epoch = 0; epoch < nbEpoch; ++epoch )
         {
            error = 0;
            for ( ui32 batch = 0; batch < nbBatches; ++batch )
            {
               Matrix dw( inputSize + 1, nbHiddenStates + 1 );

               // preallocate
               Vector hsum0( nbHiddenStates + 1 );
               Vector hsum1( inputSize      + 1 );
               Vector hsum2( nbHiddenStates + 1 );
               const ui32 size = static_cast<ui32>( batchList[ batch ].size() );
               for ( ui32 point = 0; point < size; ++point )
               {
                  const ui32 sampleId = batchList[ batch ][ point ];

                  // fetch the input
                  #pragma omp parallel for
                  for ( int n = 0; n < inputSize; ++n )
                  {
                     assert( points[ sampleId ][ n ] >= 0 && points[ sampleId ][ n ] <= 1 ); // ensure the data is correctly distributed
                     vstates[ n ] = static_cast<type>( points[ sampleId ][ n ] );
                     vstatesOrig[ n ] = vstates[ n ];
                  }

                  // go up
                  mulup( _w, vstates, hsum0 );
#ifndef NLL_NOT_MULTITHREADED
                  #pragma omp parallel for
#endif
                  for ( int n = 0; n < (int)nbHiddenStates; ++n )
                  {
                     const type p = sigm.evaluate( hsum0[ n ] );
                     hsum0[ n ] = p;
                     hstates[ n ] = p > core::generateUniformDistributionf( 0.0f, 1.0f );
                  }
                  hsum0[ nbHiddenStates ] = sigm.evaluate( hsum0[ nbHiddenStates ] );

                  // go down
                  muldown( _w, hstates, hsum1 );

#ifndef NLL_NOT_MULTITHREADED
                  #pragma omp parallel for
#endif
                  for ( int n = 0; n < inputSize; ++n )
                  {
                     const type p = sigm.evaluate( hsum1[ n ] );
                     hsum1[ n ] = p;
                     vstates[ n ] = p > core::generateUniformDistributionf( 0.0f, 1.0f );
                  }
                  hsum1[ inputSize ] = sigm.evaluate( hsum1[ inputSize ] );

                  // go up one more time
                  mulup( _w, vstates, hsum2 );

#ifndef NLL_NOT_MULTITHREADED
                  #pragma omp parallel for
#endif
                  for ( int n = 0; n < (int)nbHiddenStates; ++n )
                  {
                     const type p = sigm.evaluate( hsum2[ n ] );
                     hsum2[ n ] = p;
                  }
                  hsum2[ nbHiddenStates ] = sigm.evaluate( hsum2[ nbHiddenStates ] );
                  
                  // compute the weight update
#ifndef NLL_NOT_MULTITHREADED
                  #pragma omp parallel for
#endif
                  for ( int j = 0; j < (int)_w.sizex(); ++j )
                  {
                     for ( ui32 i = 0; i < _w.sizey(); ++i )
                     {
                        dw( i, j ) += vstatesOrig[ i ] * hsum0[ j ] - vstates[ i ] * hsum2[ j ];
                     }
                  }

                  // compute error
#ifndef NLL_NOT_MULTITHREADED
                  #pragma omp parallel for reduction(+:error)
#endif
                  for ( int n = 0; n < (int)inputSize; ++n )
                  {
                     const type val = vstatesOrig[ n ] - hsum1[ n ];
                     error += val * val;
                  }
               }

               // update the weights
#ifndef NLL_NOT_MULTITHREADED
               #pragma omp parallel for
#endif
               for ( int n = 0; n < (int)_w.size(); ++n )
               {
                  _w[ n ] += dw[ n ] * learningRate / batchSize;
               }
            }

            {
               std::stringstream ss;
               ss << " epoch=" << epoch << " reconstruction error=" << error;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }
         }

         {
            std::stringstream ss;
            ss << "training time (s)=" << timer.getCurrentTime();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }
         return error;
      }

      const Matrix& getWeights() const
      {
         return _w;
      }

   private:
      void mulup( const Matrix& w, const Vector& stateInput, Vector& stateOut )
      {
         ensure( w.sizey() == stateInput.size(), "error size" );
         ensure( w.sizex() == stateOut.size(), "error size" );

         const int nbHidden = (int)w.sizex();
         const int nbVisible = (int)w.sizey();

#ifndef NLL_NOT_MULTITHREADED
         #pragma omp parallel for
#endif
         for ( int j = 0; j < nbHidden; ++j )
         {
            type accum = 0;
            for ( int i = 0; i < nbVisible; ++i )
            {
               accum += _w( i, j ) * stateInput[ i ];
            }
            stateOut[ j ] = accum;
         }
      }

      void muldown( const Matrix& w, const Vector& stateOutput, Vector& stateOut )
      {
         ensure( w.sizey() == stateOut.size(), "error size" );
         ensure( w.sizex() == stateOutput.size(), "error size" );

         const int nbHidden = (int)w.sizex();
         const int nbVisible = (int)w.sizey();

#ifndef NLL_NOT_MULTITHREADED
         #pragma omp parallel for
#endif
         for ( int j = 0; j < nbVisible; ++j )
         {
            type accum = 0;
            for ( int i = 0; i < nbHidden; ++i )
            {
               accum += _w( j, i ) * stateOutput[ i ];
            }
            stateOut[ j ] = accum;
         }
      }

   private:
      Matrix      _w;   // _w(i, j) i visible, j hidden
   };
}
}




/**
 @brief test kdtree using different metrics
 */
class TestRbm
{
   typedef std::vector<double>                  Point;
   typedef std::vector< std::vector<double> >   Points;

   typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;
	typedef nll::algorithm::Classifier<Input>                            Classifier;
   typedef Classifier::Database                                         Database;

   Database readSmallMnist()
   {
      Database dat;
      std::ifstream fdata( NLL_DATABASE_PATH "mnist/data.bin", std::ios_base::in | std::ios_base::binary );
      std::ifstream flabel( NLL_DATABASE_PATH "mnist/labels.bin", std::ios_base::in | std::ios_base::binary );
      if ( !fdata.good() )
         throw std::runtime_error( "can't find dataset" );
      for ( ui32 n = 0; n < 5000; ++n )
      {
         if (fdata.eof())
            throw std::runtime_error("eof");
         core::Buffer1D<double> data( 28 * 28 );
         fdata.read( reinterpret_cast<i8*>( data.getBuf() ), 28 * 28 * sizeof( double ) );

         double label = 0;
         flabel.read( reinterpret_cast<i8*>( &label ), sizeof( double ) );

         Database::Sample s;
         s.input = data;
         s.output = static_cast<ui32>( label - 1 );
         s.type = Database::Sample::LEARNING;
         dat.add( s );
      }

      return dat;
   }

public:
   void testRbm1()
   {
      srand(15);
      //Database mnist = readSmallMnist();

      const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "usps" );
      ensure( benchmark, "can't find benchmark" );
      Classifier::Database mnist = benchmark->database;
      for ( ui32 n = 0; n < mnist.size(); ++n )
         for ( ui32 nn = 0; nn < mnist[n].input.size(); ++nn )
            mnist[ n ].input[ nn ] /= 2;

      typedef core::DatabaseInputAdapter<Database> Adapter;
      Adapter points( mnist );


      algorithm::RestrictedBoltzmannMachineBinary rbm0;
      double e = rbm0.trainContrastiveDivergence( points, 100, 0.2, 50 );

      const algorithm::RestrictedBoltzmannMachineBinary::Matrix& w = rbm0.getWeights();
      for ( ui32 filter = 0; filter < w.sizex(); ++filter )
      {
         core::Image<ui8> i( 16, 16, 3 );
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            for ( ui32 x = 0; x < i.sizex(); ++x )
            {
               const ui32 index = x + y * i.sizex();
               i( x, y, 0 ) = NLL_BOUND( w( index, filter ) * 127 + 128, 0, 255);
               i( x, y, 1 ) = i( x, y, 0 );
               i( x, y, 2 ) = i( x, y, 0 );
            }
         }
         core::writeBmp( i, NLL_DATABASE_PATH "filter-" + core::val2str(filter) + ".bmp" );
      }

      TESTER_ASSERT( e < 65000 );
   }

   
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRbm);
TESTER_TEST(testRbm1);
TESTER_TEST_SUITE_END();
#endif
