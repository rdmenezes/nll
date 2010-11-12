#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

template <class Classifier, class Database>
core::Image<ui8> findBounds( const Classifier& classifier, const core::vector2d& min , const core::vector2d& max, const Database& dat, ui32 size )
{
   ui8 colors[][3] =
   {
      {255, 0, 0},
      {0, 255, 0},
      {0, 0, 255},
      {255, 255, 0},
      {255, 255, 255},
      {255, 0, 255},
      {255, 255, 0},
      {0, 255, 255},
   };

   //core::Image<ui8> i( max[ 0 ] - min[ 0 ], max[ 1 ] - min[ 1 ], 3 );
   core::Image<ui8> i( size, size, 3 );
   double px = min[ 0 ];
   double py = min[ 1 ];
   const double stepx = ( max[ 0 ] - min[ 0 ] ) / size;
   const double stepy = ( max[ 1 ] - min[ 1 ] ) / size;

   for ( i32 y = 0; y < size; ++y, py += stepy )
   {
      px = min[ 0 ];
      for ( i32 x = 0; x < size; ++x, px += stepx )
      {
         core::Buffer1D<double> buf( 2 );
         buf[ 0 ] = px;
         buf[ 1 ] = py;
         ui32 c = classifier.test( buf );

         ui8* p = i.point( x, y );
         assert( c < 8 );
         p[ 0 ] = colors[ c ][ 0 ];
         p[ 1 ] = colors[ c ][ 1 ];
         p[ 2 ] = colors[ c ][ 2 ];
      }
   }

   for ( ui32 n = 0; n < dat.size(); ++n )
   {
      if ( dat[ n ].input[ 0 ] >= min[ 0 ] && dat[ n ].input[ 0 ] < max[ 0 ] &&
           dat[ n ].input[ 1 ] >= min[ 1 ] && dat[ n ].input[ 1 ] < max[ 1 ] )
      {
         ui32 c = dat[ n ].output;
         ui8* p = i.point( ( dat[ n ].input[ 0 ] - min[ 0 ] ) / stepx, ( dat[ n ].input[ 1 ] - min[ 1 ] ) / stepy );
         p[ 0 ] = colors[ c ][ 0 ] * 0.75;
         p[ 1 ] = colors[ c ][ 1 ] * 0.75;
         p[ 2 ] = colors[ c ][ 2 ] * 0.75;
      }
   }
   return i;
}

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
      cov1( 0, 0 ) = 2;

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

      // export to bitmap the region of decision
      core::Image<ui8> i = findBounds( discriminant, core::vector2d( 20, 5 ), core::vector2d( 50, 50 ), testing, 200 );
      core::writeBmp( i, "tmp.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllDiscriminantAnalysis);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
#endif