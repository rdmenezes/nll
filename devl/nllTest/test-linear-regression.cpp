#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
   
}
}

class TestLinearRegression
{
public:
   void testLinearRegression()
   {
      typedef core::Database<core::ClassificationSample<std::vector<float>, float>> Database;
      for ( size_t n = 0; n < 50; ++n )
      {
         const float a = (float)core::generateUniformDistribution( -3, 3 );
         const float b = (float)core::generateUniformDistribution( -5, 5 );
         const double err = 4;

         Database dat;
         for ( size_t nn = 0; nn < 200; ++nn )
         {
            const float x = (float)core::generateUniformDistribution( -10, 10 );
            const float y = a * x + b + (float)core::generateGaussianDistribution( 0, err );
            dat.add( Database::Sample( core::make_vector<float>( x ), y, Database::Sample::LEARNING ) );
         }

         algorithm::LinearRegression classifier;
         classifier.learn( dat, 250, 0.01 );

         const float e = error( classifier, dat );
         std::cout << "dat=" << e << std::endl;
         std::cout << "found=" << classifier.getParams()[ 0 ] << " " << classifier.getParams()[ 1 ] << std::endl;
         std::cout << "expected=" << a << " " << b << std::endl;

         TESTER_ASSERT( fabs( e ) <= 6 );
      }
   }

   template <class Classifier, class Database>
   float error( const Classifier& c, const Database& dat )
   {
      float e = 0;
      for ( size_t n = 0; n < dat.size(); ++n )
      {
         const float out = (float)c.test( dat[ n ].input );
         const float out2 = dat[ n ].output ;
         e += core::sqr( out - out2 );
      }

      return e / dat.size();
   }
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestLinearRegression);
TESTER_TEST(testLinearRegression);
TESTER_TEST_SUITE_END();
#endif