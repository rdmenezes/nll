#include <nll/nll.h>
#include <tester/register.h>
#include "bounds.h"

using namespace nll;

class TestNllDiscriminantAnalysis
{
   typedef core::Matrix<double>  Matrix;

public:
   void test1()
   {
      srand( 0 );

      // set the case generator
      double mean1[] =
      {
         40, 15
      };
      core::Buffer1D<double> mean1b( mean1, 2, false );

      double mean2[] =
      {
         40, 25
      };
      core::Buffer1D<double> mean2b( mean2, 2, false );

      double mean3[] =
      {
         30, 10
      };
      core::Buffer1D<double> mean3b( mean3, 2, false );

      Matrix cov1 = nll::core::identityMatrix<Matrix>( 2 );
      cov1( 0, 0 ) = 3;
      cov1( 1, 1 ) = 1;

      const Matrix cov2 = nll::core::identityMatrix<Matrix>( 2 );
      const Matrix cov3 = nll::core::identityMatrix<Matrix>( 2 );
      const ui32 nbSamplesPerClass = 100;

      // generate the samples
      typedef core::ClassificationSample<core::Buffer1D<double>, ui32>  Sample;
      core::Database< Sample > database;

      core::NormalMultiVariateDistribution generator1( mean1b, cov1 );
      core::NormalMultiVariateDistribution generator2( mean2b, cov2 );
      core::NormalMultiVariateDistribution generator3( mean3b, cov3 );
      for ( ui32 n = 0; n < nbSamplesPerClass; ++n )
      {
         database.add( Sample( generator1.generate(), 0, ( n > nbSamplesPerClass / 2 ) ? Sample::LEARNING : Sample::TESTING ) );
         database.add( Sample( generator2.generate(), 1, ( n > nbSamplesPerClass / 2 ) ? Sample::LEARNING : Sample::TESTING ) );
         database.add( Sample( generator3.generate(), 2, ( n > nbSamplesPerClass / 2 ) ? Sample::LEARNING : Sample::TESTING ) );
      }

      // train the classifier
      algorithm::QuadraticDiscriminantAnalysis    discriminant;
      discriminant.compute( database );

      for ( ui32 n = 0; n < 2; ++n )
      {
         std::cout << "prior=" << discriminant.getParameters()[ n ].prior << std::endl;
         discriminant.getParameters()[ n ].mean.print( std::cout );
         discriminant.getParameters()[ n ].cov.print( std::cout );
      }

      // now just test it!
      ui32 nbErrors = 0;
      core::Database< Sample > testing = core::filterDatabase( database, core::make_vector<ui32>( Sample::TESTING ), Sample::TESTING );
      for ( ui32 n = 0; n < testing.size(); ++n )
      {
         ui32 id = discriminant.test( testing[ n ].input );
         if ( id != testing[ n ].output )
            ++nbErrors;
      }

            
      // export to bitmap the region of decision
      core::Image<ui8> i = findBounds( discriminant, core::vector2d( 20, 5 ), core::vector2d( 50, 50 ), testing, 200 );
      core::writeBmp( i, "tmp.bmp" );

      std::stringstream ss;
      discriminant.write( ss );

      TESTER_ASSERT( nbErrors == 0 );

      algorithm::QuadraticDiscriminantAnalysis    discriminant2;
      discriminant2.read( ss );
      for ( ui32 n = 0; n < testing.size(); ++n )
      {
         ui32 id = discriminant2.test( testing[ n ].input );
         if ( id != testing[ n ].output )
            ++nbErrors;
      }

      TESTER_ASSERT( nbErrors == 0 );

      discriminant2.project( testing[ 0 ].input );



      algorithm::FeatureTransformationDiscriminant< core::Buffer1D<double> >  dd1;
      dd1.compute( database );
      dd1.process( database[ 0 ].input );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllDiscriminantAnalysis);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
#endif