#include <nll/nll.h>
#include <tester/register.h>

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
      typedef core::Buffer1D<double>   Vector;
      typedef core::Buffer1D<bool>     VectorB;

   public:
      /**
       @brief Train a restricted boltzmann machine using contrastive divergence
       @param points the points to use
       @param nbHiddenStates the size of the hidden layer
       @param learningRate the learning rate used to update the weights
       @param nbEpoch the number of iterations that should be used to train the RBM
       @param batchSize the number of samples used to updated the weights
       @return the energy of the model
       */
      template <class Points>
      double trainContrastiveDivergence( const Points& points, ui32 nbHiddenStates, double learningRate, ui32 nbEpoch,
                                         ui32 batchSize = 100 )
      {
         const ui32 inputSize = points[ 0 ].size();
         const ui32 nbBatches = static_cast<double>( points.size() ) / batchSize;
         const FunctionSimpleDifferenciableSigmoid sigm;

         ensure( nbBatches > 0, "invalid batch size" );
         ensure( nbHiddenStates > 0 && learningRate > 0 && nbEpoch > 0 && batchSize > 0, "invalid parameters" );

         // create the batches
         std::vector< std::vector<ui32> > > batchList( nbBatches );
         for ( ui32 n = 0; n < nbBatches; ++n )
            batchList[ n ].reserve( batchSize );
         Vector index( points.size() );
         for ( ui32 n = 0; n < points.size(); ++n )
            index[ n ] = n / nbBatches;
         core::randomize( index, 0.8 );
         for ( ui32 n = 0; n < points.size(); ++n )
            batchList[ index[ n ] ].push_back( n );

         // initialize
         // we include the bias = 1, it is located at the end of the hidden and visible layer
         Vector hstates( nbHiddenStates + 1 );
         Vector vstates( inputSize + 1 );
         Vector tmpstates( std::max( inputSize + 1, nbHiddenStates + 1 ) );

         hstates[ nbHiddenStates ] = 1;
         vstates[ inputSize ] = 1;

         Matrix w( inputSize + 1, nbHiddenStates + 1, false );
         for ( ui32 n = 0; n < w.size(); ++n )
            core::generateUniformDistribution( 1e-15, 1e-1 );

         for ( ui32 epoch = 0; epoch < nbEpoch; ++epoch )
         {
            for ( ui32 batch = 0; batch < nbBatches; ++batch )
            {
               for ( ui32 point = 0; point < batchList[ batch.size() ]; ++point )
               {
                  // fetch the input
                  ui32 sampleId = batchList[ batch ][ point ];
                  for ( ui32 n = 0; n < inputSize; ++n )
                  {
                     vstates[ n ] = points[ sampleId ][ n ];
                  }

                  Vector hsum0( nbHiddenStates + 1 );
                  mul( _w, vstates, hsum0 );
                  for ( ui32 n = 0; n < nbHiddenStates; ++n )
                  {
                     //const double p = sigm.evaluate( XX );
                  }
               }
            }
         }
      }

   private:
      static void mul( const Matrix& w, const Vector& stateInput, Vector& stateOut )
      {
         // TODO
      }

   private:
      Matrix      _w;
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

public:
   void testRbm1()
   {
      
   }

   
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRbm);
TESTER_TEST(testRbm1);
TESTER_TEST_SUITE_END();
#endif
