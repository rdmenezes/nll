#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

class TestRansac
{
public:
   static void generateLine( ui32 size, double outlier, ui32 nbPoints, double stddev, std::vector< std::pair<ui32, ui32> >& points_out, double& model_a_out, double& model_b_out )
   {
      ensure( outlier >= 0 && outlier < 1, "bad arg" );
      points_out.clear();
      points_out.reserve( nbPoints );

      model_a_out = core::generateUniformDistribution( -1, 1 );
      if ( fabs( model_a_out ) < 1e-2 )
         model_a_out = 1e-2;

      model_b_out = core::generateUniformDistribution( size / 4, size / 2 );

      std::vector<double> pbs = core::make_vector<double>( outlier, 1 - outlier );
      core::Buffer1D<ui32> pointsSampled = core::sampling( pbs, nbPoints );
      for ( ui32 n = 0; n < nbPoints; ++n )
      {
         if ( pointsSampled[ n ] == 0 )
         {
            // outlier
            points_out.push_back( std::make_pair( core::generateUniformDistribution( 0, (double)size ),
                                                  core::generateUniformDistribution( 0, (double)size ) ) );
         } else {
            // normal point
            double x, y;
            do
            {
               x = core::generateUniformDistribution( 0, (double)size - 1 );
               y = model_a_out * x + model_b_out + core::generateGaussianDistribution( 0, stddev );
               x += core::generateGaussianDistribution( 0, stddev );
            } while ( y >= (double)size || y <= 0 );
            points_out.push_back( std::make_pair( x, y ) );
         }
      }
   }

   static void printPoints( core::Image<ui8>& i, const std::vector< std::pair<ui32, ui32> >& points )
   {
      for ( ui32 n = 0; n < points.size(); ++n )
      {
         const std::pair<ui32, ui32>& p = points[ n ];
         if ( p.first + 5 < i.sizex() && p.second +5 < i.sizey() &&
              p.first > 5 &&             p.second > 5 )
         {
            core::bresham( i, core::vector2i( p.first - 4, p.second ), core::vector2i( p.first + 4, p.second ), core::vector3uc( 255, 0, 0 ) );
            core::bresham( i, core::vector2i( p.first, p.second - 4 ), core::vector2i( p.first, p.second + 4 ), core::vector3uc( 255, 0, 0 ) );
         }
      }
   }

   template <class Type>
   static void printModel( core::Image<ui8>& i, const typename algorithm::LineEstimator<Type>::Model& m )
   {
      const core::vector2i bl( 0, m.b );
      const core::vector2i tr( i.sizex() / 2, m.b + m.a * i.sizex() / 2 );

      if ( tr[ 1 ] > 0 && tr[ 1 ] < i.sizey() )
      {
         core::bresham( i, bl, tr, core::vector3uc( 0, 255, 0 ) );
      }
   }


   void testRobustLineFitting()
   {
      const ui32 size = 512;
      const double outliers = 0.5;
      typedef std::vector<double> Point;
      srand( 1 );

      for ( ui32 n = 0; n < 40; ++n )
      {
         core::Image<ui8> i( size, size, 3 );
         double a, b;
         std::vector< std::pair<ui32, ui32> > points;

         generateLine( size, outliers, 100, 8.5, points, a, b );
         printPoints( i, points );


         std::cout << "Model=" << a << " " << b << std::endl;

         std::vector< Point > pointsTfm;
         for ( ui32 nn = 0; nn < points.size(); ++nn )
         {
            pointsTfm.push_back( core::make_vector<double>( points[ nn ].first, points[ nn ].second ) );
         }
         algorithm::Ransac< algorithm::LineEstimator<Point> > estimator;
         algorithm::LineEstimator<Point>::Model model = estimator.estimate( pointsTfm, 3, 500, 1e-4 );
         printModel<Point>( i, model );

         std::cout << "Estimated Model=" << model.a << " " << model.b << std::endl;
         core::writeBmp( i, NLL_TEST_PATH "data/test" + core::val2str( n ) + ".bmp" );

         TESTER_ASSERT( fabs( model.a - a ) / a < 0.1 );
         TESTER_ASSERT( fabs( model.b - b ) / b < 0.1 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRansac);
TESTER_TEST(testRobustLineFitting);
TESTER_TEST_SUITE_END();
#endif
