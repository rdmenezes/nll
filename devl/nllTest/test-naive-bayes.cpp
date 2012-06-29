#include <nll/nll.h>
#include <tester/register.h>
#include "bounds.h"

using namespace nll;
/*
namespace nll
{
namespace algorithm
{
   class NayiveBayes
   {
      typedef core::Buffer1D<double>   Vector;

      struct Class
      {
         Vector      mean;
         Vector      var;

         template <class Point>
         double computeProbability( const Point& p ) const
         {
            double pp = 0;
            for ( size_t n = 0; n < mean.size(); ++n )
            {
               // if we have a log of features, better do a log for numerical stability
               const double pf = _sqrdiv[ n ] * exp( - core::sqr( p[ n ] - mean[ n ] ) / _norm[ n ] );
               pp += log( pf );
            }
            return exp( pp );
         }

         void precompute()
         {
            _sqrdiv = Vector( mean.size() );
            _norm = Vector( mean.size() );
            for ( size_t n = 0; n < mean.size(); ++n )
            {
               _sqrdiv[ n ] = 1.0 / ( sqrt( 2 * nll::core::PI * core::sqr( var[ n ] ) ) + 1e-8 );
               _norm[ n ] = 2.0 * core::sqr( var[ n ] ) + 1e-8;
            }
         }

      private:
         Vector _sqrdiv;
         Vector _norm;
      };
      typedef std::vector<Class> Classes;

   public:
      template <class Database>
      void compute( const Database& database )
      {
         if ( database.size() == 0 )
            return;
         const size_t nbFeatures = database[ 0 ].input.size();

         // compute some constants
         const size_t nbClasses = getNumberOfClass( database );
         _classes = Classes( nbClasses );

         // first filter the training samples
         Database training = core::filterDatabase( database, core::make_vector<size_t>( Database::Sample::LEARNING ), Database::Sample::LEARNING );

         for ( size_t classid = 0; classid < nbClasses; ++classid )
         {
            _classes[ classid ].mean = Vector( nbFeatures );
            _classes[ classid ].var  = Vector( nbFeatures );

            Vector& mean = _classes[ classid ].mean;
            Vector& var  = _classes[ classid ].var;

            // filter the samples by class
            typedef core::DatabaseInputAdapterClass<Database> Adapter;
            Adapter adapter( training, classid );

            // compute the mean
            for ( size_t n = 0; n < adapter.size(); ++n )
            {
               Adapter::Point& s = adapter[ n ];
               for ( size_t f = 0; f < nbFeatures; ++f )
               {
                  mean[ f ] += s[ f ];
               }
            }

            for ( size_t f = 0; f < nbFeatures; ++f )
            {
               mean[ f ] /= adapter.size();
            }

            // compute the variance
            for ( size_t n = 0; n < adapter.size(); ++n )
            {
               Adapter::Point& s = adapter[ n ];
               for ( size_t f = 0; f < nbFeatures; ++f )
               {
                  double val = s[ f ] - mean[ f ];
                  var[ f ] += core::sqr( val );
               }
            }

            for ( size_t f = 0; f < nbFeatures; ++f )
            {
               var[ f ] /= adapter.size();
            }

            _classes[ classid ].precompute();
         }
      }

      void write( std::ostream& o ) const
      {
         if ( !o.good() )
            throw std::runtime_error( "cannot read from stream" );

         size_t nbC = (size_t)_classes.size();
         core::write<size_t>( nbC, o );
         for ( size_t n = 0; n < nbC; ++n )
         {
            _classes[ n ].mean.write( o );
            _classes[ n ].var.write( o );
         }
      }

      void read( std::istream& i )
      {
         if ( !i.good() )
            throw std::runtime_error( "cannot read from stream" );
         
         // read
         size_t nbC = 0;
         core::read<size_t>( nbC, i );
         _classes = Classes( nbC );
         for ( size_t classid = 0; classid < nbC; ++classid )
         {
            _classes[ classid ].mean.read( i );
            _classes[ classid ].var.read( i );
            _classes[ classid ].precompute();
         }
      }

      template <class Point>
      size_t test( const Point& p, core::Buffer1D<double>* probability = 0 ) const
      {
         double max = (double)INT_MIN;
         double sum = 0;
         size_t index = (size_t)_classes.size();

         if ( probability )
         {
            *probability = core::Buffer1D<double>( (size_t)_classes.size() );
         }

         for ( size_t n = 0; n < (size_t)_classes.size(); ++n )
         {
            double v = _classes[ n ].computeProbability( p );
            if ( v > max )
            {
               max = v;
               index = n;
            }

            if ( probability )
            {
               sum += v;
               ( *probability )[ n ] = v;
            }
         }

         if ( probability )
         {
            for ( size_t n = 0; n < (size_t)_classes.size(); ++n )
               ( *probability )[ n ] /= sum;
         }

         assert( index < _classes.size() );
         return index;
      }

      const Classes& getParameters() const
      {
         return _classes;
      }

   private:
      Classes  _classes;
   };
}
}
*/
class TestNaiveBayes
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
      const size_t nbSamplesPerClass = 1000;

      // generate the samples
      typedef core::ClassificationSample<core::Buffer1D<double>, size_t>  Sample;
      core::Database< Sample > database;

      core::NormalMultiVariateDistribution generator1( mean1b, cov1 );
      core::NormalMultiVariateDistribution generator2( mean2b, cov2 );
      core::NormalMultiVariateDistribution generator3( mean3b, cov3 );
      for ( size_t n = 0; n < nbSamplesPerClass; ++n )
      {
         database.add( Sample( generator1.generate(), 0, ( n > nbSamplesPerClass / 2 ) ? Sample::LEARNING : Sample::TESTING ) );
         database.add( Sample( generator2.generate(), 1, ( n > nbSamplesPerClass / 2 ) ? Sample::LEARNING : Sample::TESTING ) );
         database.add( Sample( generator3.generate(), 2, ( n > nbSamplesPerClass / 2 ) ? Sample::LEARNING : Sample::TESTING ) );
      }

      // train the classifier
      algorithm::NaiveBayes bayes;
      bayes.compute( database );

      for ( size_t n = 0; n < 2; ++n )
      {
         bayes.getParameters()[ n ].mean.print( std::cout );
         bayes.getParameters()[ n ].var.print( std::cout );
      }

      // now just test it!
      size_t nbErrors = 0;
      core::Database< Sample > testing = core::filterDatabase( database, core::make_vector<size_t>( Sample::TESTING ), Sample::TESTING );
      for ( size_t n = 0; n < testing.size(); ++n )
      {
         core::Buffer1D<double> p;
         size_t id = bayes.test( testing[ n ].input, &p );
         if ( id != testing[ n ].output )
            ++nbErrors;
      }

            
      // export to bitmap the region of decision
      core::Image<ui8> i = findBounds( bayes, core::vector2d( 20, 5 ), core::vector2d( 50, 50 ), testing, 200 );
      core::writeBmp( i, "tmp.bmp" );

      std::stringstream ss;
      bayes.write( ss );

      TESTER_ASSERT( nbErrors == 0 );

      algorithm::NaiveBayes    bayes2;
      bayes.read( ss );
      for ( size_t n = 0; n < testing.size(); ++n )
      {
         size_t id = bayes.test( testing[ n ].input );
         if ( id != testing[ n ].output )
            ++nbErrors;
      }

      TESTER_ASSERT( nbErrors == 0 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNaiveBayes);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
#endif