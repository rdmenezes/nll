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
      RestrictedBoltzmannMachineBinary() : _nbClass( 0 )
      {
      }

      void write( std::ostream& out )
      {
         if ( !out.good() )
            throw std::runtime_error( "stream error" );

         _w.write( out );
         core::write<ui32>( _nbClass, out );
      }

      void read( std::istream& in )
      {
         if ( !in.good() )
            throw std::runtime_error( "stream error" );

         _w.read( in );
         core::read<ui32>( _nbClass, in );
      }

      // if no index is found, it returns an ID class >> _nbClass
      ui32 getClass( const Vector& input, ui32 nbCycles = 10 )
      {
         Vector hstates( _w.sizex() );
         Vector hsum( hstates.size() );
         Vector vstates( _w.sizey() );
         Vector vsum( vstates.size() );

         const ui32 nbVisibleStates = _w.sizey() - 1 - _nbClass;
         const ui32 nbHiddenStates = _w.sizex() - 1;

         // set the bias & label correctly
         hstates[ _w.sizex() - 1 ] = 1;
         vstates[ _w.sizey() - 1 ] = 1;

         for ( ui32 n = 0; n < (int)nbVisibleStates; ++n )
         {
            vstates[ n ] = input[ n ];
         }

         Vector nbSamples( _nbClass );
         for ( ui32 n = 0; n < nbCycles; ++n )
         {
            _updown( vstates, hstates, hsum, vsum, nbHiddenStates, nbVisibleStates );
            for ( ui32 nn = 0; nn < _nbClass; ++nn )
            {
               if ( vstates[ nn + nbVisibleStates ] >= 1 )
               {
                  ++nbSamples[ nn ];
                  break;
               }
            }
         }

         int max = 0;
         int index = 1;
         for ( ui32 nn = 0; nn < _nbClass; ++nn )
         {
            if ( max < nbSamples[ nn ] )
            {
               max = nbSamples[ nn ];
               index = nn;
            }
         }
         return (ui32)index;
      }

      Vector generate( ui32 nbIter = 1000 )
      {
         Vector hstates( _w.sizex() );
         Vector hsum( hstates.size() );
         Vector vstates( _w.sizey() );
         Vector vsum( vstates.size() );

         const ui32 nbVisibleStates = _w.sizey() - 1 - _nbClass;
         const ui32 nbHiddenStates = _w.sizex() - 1;

         // set the bias & label correctly
         hstates[ _w.sizex() - 1 ] = 1;
         vstates[ _w.sizey() - 1 ] = 1;

         {
            std::stringstream ss;
            _w.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // initialize randomly the states
         for ( ui32 n = 0; n < (int)nbVisibleStates; ++n )
         {
            vstates[ n ] = core::generateUniformDistributionf( 0.7f, 0.8f );
         }

         // 
         for ( ui32 n = 0; n < nbIter; ++n )
         {
            _updown( vstates, hstates, hsum, vsum, nbHiddenStates, nbVisibleStates );
         }

         vsum.print(std::cout );
         hsum.print(std::cout );
         vstates.print(std::cout );
         hstates.print(std::cout );

         return Vector( vsum.stealBuf(), nbVisibleStates, true );
      }

      /**
       @brief Train a restricted boltzmann machine using contrastive divergence
       @param points the points to use - these must be in the [0..1] interval so that they can be interpreted as probability
       @param labels a serie of labels. Size can be 0 if no labels are associated, else the size of the data. Labels start at 0, until n
       @param nbHiddenStates the size of the hidden layer
       @param learningRate the learning rate used to update the weights
       @param nbEpoch the number of iterations that should be used to train the RBM
       @param batchSize the number of samples used to updated the weights
       @return the reconstruction error
       */
      template <class Points, class Labels>
      double trainContrastiveDivergence( const Points& points, const Labels& labels, ui32 nbHiddenStates, type learningRate, ui32 nbEpoch,
                                         ui32 batchSize = 100 )
      {
         //
         // current layout:
         //   hidden [0..nbHiddenStates-1] bias
         //   visible [0..nbVisibleStates-1] class [nbVisibleStates..nbVisibleStates+nbClassUnits] bias
         // the bias state is always set to 1
         // the class is set when the data is fetched
         //

         const int inputSize = points[ 0 ].size();
         const ui32 nbBatches = (ui32)std::ceil( static_cast<type>( points.size() ) / batchSize );
         core::Timer timer;

         int maxLabel = -1;
         ui32 nbLabels = static_cast<ui32>( labels.size() );
         for ( ui32 n = 0; n < nbLabels; ++n )
         {
            ui32 label = labels[ n ];
            maxLabel = std::max<int>( label, maxLabel );
         }
         ensure( maxLabel == -1 || labels.size() == points.size(), "if labels are present, it must be for all data" );
         const ui32 nbClassUnits = ( maxLabel != -1 ) ? ( maxLabel + 1 ) : 0;
         _nbClass = nbClassUnits;

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
         Vector vstates( inputSize + 1 + nbClassUnits );
         Vector vstatesOrig( vstates.size() );
         Vector tmpstates( std::max<ui32>( vstates.size(), hstates.size() ) );

         hstates[ nbHiddenStates ] = 1;
         vstates[ inputSize + nbClassUnits ] = 1;
         vstatesOrig[ inputSize + nbClassUnits ] = 1;

         _w = Matrix( vstates.size(), hstates.size() );
         for ( ui32 n = 0; n < _w.size(); ++n )
            _w[ n ] = (type)core::generateUniformDistribution( -2e-1, 2e-1 );

         type error = 0;
         for ( ui32 epoch = 0; epoch < nbEpoch; ++epoch )
         {
            error = 0;
            for ( ui32 batch = 0; batch < nbBatches; ++batch )
            {
               Matrix dw( vstates.size(), hstates.size() );

               // preallocate
               Vector hsum0( hstates.size() );
               Vector hsum1( vstates.size() );
               Vector hsum2( hstates.size() );
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
                  for ( ui32 n = 0; n < nbClassUnits; ++n )
                  {
                     vstates[ inputSize + n ] = labels[ sampleId ] == n;
                     vstatesOrig[ inputSize + n ] = vstates[ inputSize + n ];
                  }

                  _update( vstates, hstates, hsum0, hsum1, hsum2, nbHiddenStates, inputSize );

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
                  for ( int n = 0; n < (int)inputSize + (int)nbClassUnits; ++n )
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
      void _updown(Vector vstates, Vector hstates, Vector hsum0, Vector hsum1, int nbHiddenStates, int inputSize )
      {
         const FunctionSimpleDifferenciableSigmoid sigm;
         Vector softmax( _nbClass );

         // go up
         mulup( _w, vstates, hsum0 );
#ifndef NLL_NOT_MULTITHREADED
         #pragma omp parallel for
#endif
         for ( int n = 0; n < nbHiddenStates; ++n )
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
         hsum1[ inputSize + _nbClass ] = sigm.evaluate( hsum1[ _nbClass + inputSize ] );

         // handle the softmax units
         type sum = 0;
         for ( int n = 0; n < (int)_nbClass; ++n )
         {
            softmax[ n ] = exp( hsum1[ inputSize + n ] );
            sum += softmax[ n ];
         }
         if ( sum > 0 )
         {
            for ( int n = 0; n < (int)_nbClass; ++n )
            {
               hsum1[ inputSize + n ] = softmax[ n ] / sum;
               softmax[ n ] = hsum1[ inputSize + n ];
            }
            ui32 unitClass = core::sampling( softmax, 1 )[ 0 ];   // sample from the softmax variable
            for ( ui32 n = 0; n < (ui32)_nbClass; ++n )
            {
               vstates[ inputSize + n ] = ( unitClass == n );
            }
         } else {
            for ( ui32 n = 0; n < (ui32)_nbClass; ++n )
            {
               vstates[ inputSize + n ] = 0;
            }
         }
      }

      void _update(Vector vstates, Vector hstates, Vector hsum0, Vector hsum1, Vector hsum2, int nbHiddenStates, int inputSize )
      {
         const FunctionSimpleDifferenciableSigmoid sigm;
         _updown( vstates, hstates, hsum0, hsum1, nbHiddenStates, inputSize );

         // go up one more time
         mulup( _w, vstates, hsum2 );

#ifndef NLL_NOT_MULTITHREADED
         #pragma omp parallel for
#endif
         for ( int n = 0; n < nbHiddenStates; ++n )
         {
            const type p = sigm.evaluate( hsum2[ n ] );
            hsum2[ n ] = p;
         }
         hsum2[ nbHiddenStates ] = sigm.evaluate( hsum2[ nbHiddenStates ] );
         // don't recompute the hidden states, we don't need it (so we can save a copy)
      }

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
      ui32        _nbClass;
   };

   class RestrictedBoltzmannMachineBinary2
   {
   public:
      typedef double                   type;
      typedef core::Matrix<type>       Matrix;
      typedef core::Buffer1D<type>     Vector;
      typedef core::Buffer1D<ui32>     Index;

   public:
      template <class Points, class Labels>
      double trainContrastiveDivergence( const Points& points, const Labels& labels, ui32 nbHiddenStates, type learningRate, ui32 nbEpoch,
                                         ui32 batchSize = 100 )
      {
         const ui32 inputSize = points[ 0 ].size();
         const ui32 nbBatches = (ui32)std::ceil( static_cast<type>( points.size() ) / batchSize );
         core::Timer timer;

         int maxLabel = -1;
         ui32 nbLabels = static_cast<ui32>( labels.size() );
         for ( ui32 n = 0; n < nbLabels; ++n )
         {
            ui32 label = labels[ n ];
            maxLabel = std::max<int>( label, maxLabel );
         }
         ensure( maxLabel == -1 || labels.size() == points.size(), "if labels are present, it must be for all data" );
         const ui32 nbClassUnits = ( maxLabel != -1 ) ? ( maxLabel + 1 ) : 0;

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

         // train
         Vector hstates( nbHiddenStates );      // hidden states
         Vector vstates( inputSize );           // visible states
         Matrix w( nbHiddenStates, inputSize ); // weights connection
         Vector c( inputSize );                 // biases on visible layer
         Vector b( nbHiddenStates );            // biases on hidden layer
         Matrix wc( nbClassUnits, nbHiddenStates ); // weight classes
         Vector cc( nbClassUnits );                 // biases on class layer

         // randomly init the weights
         for ( ui32 n = 0; n < w.size(); ++n )
            w[ n ] = (type)core::generateUniformDistribution( -2e-1, 2e-1 );

         double error;
         const FunctionSimpleDifferenciableSigmoid sigm;
         for ( ui32 epoch = 0; epoch < nbEpoch; ++epoch )
         {
            error = 0;
            for ( ui32 batch = 0; batch < nbBatches; ++batch )
            {
               const ui32 nbPoints = batchList[ batch ].size();
               Matrix dw( nbHiddenStates, inputSize );
               Vector dc( inputSize );
               Vector db( nbHiddenStates );
               for ( ui32 n = 0; n < nbPoints; ++n )
               {
                  const ui32 sampleId = batchList[ batch ][ n ];

                  // fetch the data
                  Vector input;
                  core::convert( points[ n ], input );

                  // up - down
                  Matrix ph, negData;
                  Vector phstates, negDataStates;
                  _upDown( w, b, c, input, ph, phstates, negData, negDataStates );

                  // go up one more time
                  Matrix nh = w * Matrix( negDataStates, negDataStates.size(), 1 );
                  Vector nhstates( nh.size() );
                  for ( ui32 n = 0; n < nh.size(); ++n )
                  {
                     nh[ n ] += b[ n ];
                     nh[ n ] = sigm.evaluate( nh[ n ] );
                     nhstates[ n ] = ( nh[ n ] > core::generateUniformDistribution( 0, 1 ) );
                  }

                  // compute updates
                  for ( ui32 n = 0; n < inputSize; ++n )
                  {
                     dc[ n ] += ( points[ sampleId ][ n ] - negDataStates[ n ] );
                  }

                  for ( ui32 n = 0; n < nbHiddenStates; ++n )
                  {
                     db[ n ] += ( ph[ n ] - nh[ n ] );
                  }

                  for ( int i = 0; i < (int)w.sizey(); ++i )
                  {
                     for ( ui32 j = 0; j < w.sizex(); ++j )
                     {
                        dw( i, j ) += input[ j ] * ph[ i ] - negDataStates[ j ] * nh[ i ];
                     }
                  }

                  // error
                  for ( int n = 0; n < (int)inputSize; ++n )
                  {
                     const type val = input[ n ] - negData[ n ];
                     error += val * val;
                  }
               }

               // update w
               for ( int n = 0; n < (int)w.size(); ++n )
               {
                  w[ n ] += dw[ n ] * learningRate / batchSize;
               }
            }

            std::cout << "epoch=" << epoch << " error=" << error << std::endl;
         }

         std::cout << "DONE!" << std::endl;

         _w = w;
         _b = b;
         _c = c;
         return error;
      }

      const Matrix& getWeights() const
      {
         return _w;
      }

      void write( std::ostream& out )
      {
         if ( !out.good() )
            throw std::runtime_error( "stream error" );

         _w.write( out );
         _b.write( out );
         _c.write( out );
      }

      void read( std::istream& in )
      {
         if ( !in.good() )
            throw std::runtime_error( "stream error" );

         _w.read( in );
         _b.read( in );
         _c.read( in );
      }

      Vector generate( ui32 nbIter = 500, type eps = 1e-1 )
      {
         Vector vstates( _w.sizex() );
         Matrix negdata;

         const ui32 nbVisibleStates = vstates.size();
         const ui32 nbHiddenStates = _w.sizey();

         {
            std::stringstream ss;
            _w.print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // initialize randomly the states
         for ( ui32 n = 0; n < (int)nbVisibleStates; ++n )
         {
            vstates[ n ] = core::generateUniformDistribution( -eps, eps );
         }

         // up - down for reconstruction
         for ( ui32 n = 0; n < nbIter; ++n )
         {
            Vector hstates;
            Matrix hsum;
            Vector negdataStates;

            _upDown( _w, _b, _c, vstates, hsum, hstates, negdata, negdataStates );
            vstates = negdataStates;
         }

         return Vector( negdata.stealBuf(), negdata.size(), true );
      }

   private:
      // given the data input, weights and bias it computes the hidden state, and new visible state
      void _upDown( const Matrix& w, const Vector& b, const Vector& c, const Vector& input, Matrix& phOut, Vector& phstatesOut, Matrix& negDataOut, Vector& negDataStatesOut )
      {
         const FunctionSimpleDifferenciableSigmoid sigm;
         Matrix ph = w * Matrix( input, input.size(), 1 );
         Vector phstates( ph.size() );
         for ( ui32 n = 0; n < ph.size(); ++n )
         {
            ph[ n ] += b[ n ];
            ph[ n ] = sigm.evaluate( ph[ n ] );
            phstates[ n ] = ( ph[ n ] > core::generateUniformDistribution( 0, 1 ) );
         }

         // down
         Matrix negData = Matrix( phstates, 1, phstates.size() ) * w;
         Vector negDataStates( negData.size() );
         for ( ui32 n = 0; n < negData.size(); ++n )
         {
            negData[ n ] += c[ n ];
            negData[ n ] = sigm.evaluate( negData[ n ] );
            negDataStates[ n ] = ( negData[ n ] > core::generateUniformDistribution( 0, 1 ) );
         }

         phOut = ph;
         phstatesOut = phstates;
         negDataOut = negData;
         negDataStatesOut = negDataStates;
      }

   private:
      Matrix   _w;
      Vector   _b;
      Vector   _c;
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
      srand(time(0));
      //Database mnist = readSmallMnist();

      const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "usps" );
      ensure( benchmark, "can't find benchmark" );
      Classifier::Database mnist = benchmark->database;
      for ( ui32 n = 0; n < mnist.size(); ++n )
         for ( ui32 nn = 0; nn < mnist[n].input.size(); ++nn )
            mnist[ n ].input[ nn ] /= 2;
      mnist = core::filterDatabase( mnist, core::make_vector<ui32>( (ui32) Database::Sample::LEARNING ), (ui32) Database::Sample::LEARNING );

      typedef core::DatabaseInputAdapter<Database>          Adapter;
      Adapter points( mnist );

      typedef core::DatabaseClassAdapterRead<Database>      AdapterClass;
      AdapterClass classes( mnist );
      //std::vector<ui32> classes;

      algorithm::RestrictedBoltzmannMachineBinary2 rbm0;
      double e = rbm0.trainContrastiveDivergence( points, classes, 100, 0.20, 10 );

      const algorithm::RestrictedBoltzmannMachineBinary2::Matrix& w = rbm0.getWeights();
      for ( ui32 filter = 0; filter < w.sizey(); ++filter )
      {
         core::Image<ui8> i( 16, 16, 3 );
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            for ( ui32 x = 0; x < i.sizex(); ++x )
            {
               const ui32 index = x + y * i.sizex();
               //std::cout << "inde=" << index << std::endl;
               i( x, y, 0 ) = NLL_BOUND( w( filter, index ) * 127 + 128, 0, 255);
               i( x, y, 1 ) = i( x, y, 0 );
               i( x, y, 2 ) = i( x, y, 0 );
            }
         }
         core::writeBmp( i, NLL_DATABASE_PATH "filter-" + core::val2str(filter) + ".bmp" );
      }

      TESTER_ASSERT( e < 65500 );

      std::ofstream f( NLL_DATABASE_PATH "rbm0.bin", std::ios_base::out | std::ios_base::binary );
      rbm0.write( f );
   }

   void testRbmGenerate1()
   {
      algorithm::RestrictedBoltzmannMachineBinary2 rbm;

      std::ifstream f( NLL_DATABASE_PATH "rbm0.bin", std::ios_base::in | std::ios_base::binary );
      rbm.read( f );

      for ( ui32 n = 0; n < 500; ++n )
      {
         ui32 label = 9;
         //algorithm::RestrictedBoltzmannMachineBinary::Vector sample = rbm.generate(label, 50);
         algorithm::RestrictedBoltzmannMachineBinary2::Vector sample = rbm.generate(500, 1e-6);

         core::Image<ui8> i( 16, 16, 3 );
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            for ( ui32 x = 0; x < i.sizex(); ++x )
            {
               const ui32 index = x + y * i.sizex();
               i( x, y, 0 ) = NLL_BOUND( sample( index ) * 127 + 128, 0, 255);
               i( x, y, 1 ) = i( x, y, 0 );
               i( x, y, 2 ) = i( x, y, 0 );
            }
         }
         core::writeBmp( i, NLL_DATABASE_PATH "generate-" + core::val2str(label) + "-" + core::val2str( n ) + ".bmp" );
      }
   }

   void testRbmRecon1()
   {
      algorithm::RestrictedBoltzmannMachineBinary rbm;

      std::ifstream f( NLL_DATABASE_PATH "rbm0.bin", std::ios_base::in | std::ios_base::binary );
      rbm.read( f );

      const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "usps" );
      ensure( benchmark, "can't find benchmark" );
      Classifier::Database mnist = benchmark->database;
      for ( ui32 n = 0; n < mnist.size(); ++n )
         for ( ui32 nn = 0; nn < mnist[n].input.size(); ++nn )
            mnist[ n ].input[ nn ] /= 2;

      mnist = core::filterDatabase( mnist, core::make_vector<ui32>( (ui32) Database::Sample::TESTING ), (ui32) Database::Sample::TESTING );
      //Database mnist = readSmallMnist();


      ui32 nbGood = 0;
      ui32 nb = 0;
      ui32 nbUnknown = 0;
      for ( ui32 n = 0; n < mnist.size(); ++n )
      {
         ui32 id = rbm.getClass( mnist[ n ].input, 5 );
         std::cout << "label=" << mnist[ n ].output << " found=" << id << std::endl;

         if ( id > 10 )
            ++nbUnknown;
         else
         {
            if ( id == mnist[ n ].output )
               ++nbGood;
         }
         ++nb;
      }

      std::cout << "accuracy=" << (double)nbGood / nb << std::endl;
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRbm);
TESTER_TEST(testRbm1);
//TESTER_TEST(testRbmGenerate1);

//TESTER_TEST(testRbmRecon1);
TESTER_TEST_SUITE_END();
#endif
