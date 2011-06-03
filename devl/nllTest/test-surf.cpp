#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace
{
   std::string pairs[] =
   {
      "data/feature/sq1.bmp",
      "data/feature/sq2.bmp",

      "data/feature/sq1.bmp",
      "data/feature/sq3.bmp",

      "data/feature/sq1.bmp",
      "data/feature/sq4.bmp",

      "data/feature/sq1.bmp",
      "data/feature/sq5.bmp",

      "data/feature/sq1.bmp",
      "data/feature/sq6.bmp",

      "data/feature/sq1.bmp",
      "data/feature/sq7.bmp",

      "data/feature/sq1.bmp",
      "data/feature/sq8.bmp",

      "data/feature/sq1.bmp",
      "data/feature/sq9.bmp",
   };
}

namespace nll
{
namespace algorithm
{
   
}
}

class TestSurf
{
public:

   void printPoints( core::Image<ui8>& output, algorithm::SpeededUpRobustFeatures::Points& points )
   {
      for ( ui32 n = 0; n < points.size(); ++n )
      {
         ui32 px = points[ n ].position[ 0 ];
         ui32 py = points[ n ].position[ 1 ];
         ui32 scale = points[ n ].scale;
         ui32 half = scale / 2;

         int dx = (int)(cos( points[ n ].orientation ) * half);
         int dy = (int)(sin( points[ n ].orientation ) * half);
         if ( px > 5 &&
              py > 5 &&
              px + dx < output.sizex() - 1 &&
              py + dy < output.sizey() - 1 &&
              px + dx > 0 &&
              py + dy > 0 )
         {
            core::bresham( output, core::vector2i( px + 5, py ), core::vector2i( px - 5, py ),    core::vector3uc(255, 255, 255) );
            core::bresham( output, core::vector2i( px, py - 5 ), core::vector2i( px, py + 5 ),    core::vector3uc(255, 255, 255) );
            core::bresham( output, core::vector2i( px, py ),     core::vector2i( px + dx, py + dy), core::vector3uc(0, 0, 255) );
         }
      }
   }

   /**
    @brief the similarity can be greater than 1 if the points are clustered with a distance < tolPixel
    */
   template <class Transformation>
   std::vector< std::pair<ui32, ui32> > getRepeatablePointPosition( const algorithm::SpeededUpRobustFeatures::Points& p1,
                                                                    const algorithm::SpeededUpRobustFeatures::Points& p2,
                                                                    const Transformation& tfm,
                                                                    double tolPixel )
   {
      std::vector< std::pair<ui32, ui32> > index;
      const double tolPixel2 = tolPixel * tolPixel;

      for ( ui32 n1 = 0; n1 < (ui32)p1.size(); ++n1 )
      {
         for ( ui32 n2 = 0; n2 < (ui32)p2.size(); ++n2 )
         {
            const algorithm::SpeededUpRobustFeatures::Point& t1 = p1[ n1 ];
            const algorithm::SpeededUpRobustFeatures::Point& t2 = p2[ n2 ];

            core::vector2f p = tfm( core::vector2f( (f32)t2.position[ 0 ], (f32)t2.position[ 1 ] ) );

            float dx = p[ 0 ] - t1.position[ 0 ];
            float dy = p[ 1 ] - t1.position[ 1 ];
            double d = dx * dx + dy * dy;
            if ( d < tolPixel2 )
            {
               index.push_back( std::make_pair( n1, n2 ) );
               break;
            }
         }
      }

      return index;
   }

   template <class Transformation>
   std::vector< ui32 > getRepeatablePointOrientation( const std::vector< std::pair<ui32, ui32> >& match,
                                                      const algorithm::SpeededUpRobustFeatures::Points& p1,
                                                      const algorithm::SpeededUpRobustFeatures::Points& p2,
                                                      const Transformation& tfm,
                                                      double tol )
   {

      std::vector< ui32 > matchOrientation;
      for ( ui32 n = 0; n < (ui32)match.size(); ++n )
      {
         const algorithm::SpeededUpRobustFeatures::Point& t1 = p1[ match[ n ].first ];
         const algorithm::SpeededUpRobustFeatures::Point& t2 = p2[ match[ n ].second ];

         //const ui32 id2 = match[ n ].second;
         core::vector2f p( (f32)( 1000 * cos( t2.orientation ) ), 
                           (f32)( 1000 * sin( t2.orientation ) ) );
         core::vector2f pTfm = tfm( p );
         const double a1 = core::getAngle(  pTfm[ 0 ], pTfm[ 1 ] );
         if ( fabs( a1 - t1.orientation ) < tol )
         {
            matchOrientation.push_back( n );
         }
      }

      return matchOrientation;
   }

   void composeMatch( const core::Image<ui8>& i1, const core::Image<ui8>& i2, core::Image<ui8>& output,
                      const algorithm::SpeededUpRobustFeatures::Points& p1,
                      const algorithm::SpeededUpRobustFeatures::Points& p2,
                      const algorithm::impl::FeatureMatcher::Matches& matches )
   {
      output = core::Image<ui8>( i1.sizex() + i2.sizex(),
                                 std::max( i1.sizey(), i2.sizey() ),
                                 3 );
      for ( ui32 y = 0; y < i1.sizey(); ++y )
      {
         for ( ui32 x = 0; x < i1.sizex(); ++x )
         {
            output( x, y, 0 ) = i1( x, y, 0 );
            output( x, y, 1 ) = i1( x, y, 1 );
            output( x, y, 2 ) = i1( x, y, 2 );
         }
      }

      for ( ui32 y = 0; y < i2.sizey(); ++y )
      {
         for ( ui32 x = 0; x < i2.sizex(); ++x )
         {
            output( x + i1.sizex(), y, 0 ) = i2( x, y, 0 );
            output( x + i1.sizex(), y, 1 ) = i2( x, y, 1 );
            output( x + i1.sizex(), y, 2 ) = i2( x, y, 2 );
         }
      }


      ui32 start = 0;
      for ( ui32 n = start; n < std::min<ui32>(start + 50, (ui32)matches.size()); ++n )
      //for ( ui32 n = 0; n < (ui32)matches.size(); ++n ) // TODO PUT IT BACK
      {
         //std::cout << "d=" << matches[ n ].dist << std::endl;
         const algorithm::SpeededUpRobustFeatures::Point& f1 = p1[ matches[ n ].index1 ];
         const algorithm::SpeededUpRobustFeatures::Point& f2 = p2[ matches[ n ].index2 ];
         core::bresham( output, f1.position, core::vector2i( f2.position[ 0 ] + i1.sizex(), f2.position[ 1 ] ), core::vector3uc( (ui8)( rand() % 255 ), (ui8)( rand() % 255 ), (ui8)( rand() % 255 ) ) );
      }
   }

   void displayTransformation( const core::Image<ui8>& i1, const core::Image<ui8>& i2, core::Image<ui8>& output, core::Matrix<double>& tfm )
   {
      output = core::Image<ui8>( i2.sizex(), i2.sizey(), 3 );

      core::vector3uc black( 0, 0, 0 );
      core::resampleNearestNeighbour( i1, output, tfm, black );

      double coef = 0.5;
      for ( ui32 y = 0; y < i2.sizey(); ++y )
      {
         for ( ui32 x = 0; x < i2.sizex(); ++x )
         {
            for ( ui32 c = 0; c < 3; ++c )
            {
               output( x, y, c ) = (ui8)( (c==2) * coef * i2( x, y, c ) + ( 1 - coef) * output( x, y, c ) );
            }
         }
      }
   }

   void testBasic()
   {
      // init
      core::Image<ui8> image( NLL_TEST_PATH "data/feature/sf.bmp" );

      core::Image<ui8> output;
      output.clone( image );

      TESTER_ASSERT( image.sizex() );
      core::decolor( image );

      algorithm::SpeededUpRobustFeatures surf( 5, 4, 2, 0.005 );

      nll::core::Timer timer;
      algorithm::SpeededUpRobustFeatures::Points points1 = surf.computesFeatures( image );
      std::cout << "done=" << timer.getCurrentTime() << std::endl;

      std::cout << "nbPOints=" << points1.size() << std::endl;

      printPoints( output, points1 );
      core::writeBmp( output, "c:/tmp/o.bmp" );
   }

   void testRepeatability()
   {
      const std::string path = "data/feature/sq1.bmp";
      

      for ( ui32 n = 0; n < 100; ++n )
      {
         // init
         core::Image<ui8> image( NLL_TEST_PATH + path );

         core::Image<ui8> output;
         output.clone( image );

         core::Image<ui8> image2;
         image2.clone( image );

         //core::TransformationRotation tfm( 0, core::vector2f( 40, 0 ) );
         const double angle = core::generateUniformDistribution( -0.2, 0.1 );
         const double dx = core::generateUniformDistribution( 0, 40 );
         core::TransformationRotation tfm( (float)angle, core::vector2f( 0, (float)dx ) );
         core::transformUnaryFast( image2, tfm );

         core::Image<ui8> oi1;
         oi1.clone( image );
         core::Image<ui8> oi2;
         oi2.clone( image2 );

         core::writeBmp( image, "c:/tmp/oi1.bmp" );
         core::writeBmp( image2, "c:/tmp/oi2.bmp" );

         core::Image<ui8> output2;
         output2.clone( image2 );

         TESTER_ASSERT( image.sizex() );
         core::decolor( image );
         core::decolor( image2 );

         algorithm::SpeededUpRobustFeatures surf( 5, 6, 2, 0.00011 );

         nll::core::Timer timer;
         algorithm::SpeededUpRobustFeatures::Points points1 = surf.computesFeatures( image );
         std::cout << "done=" << timer.getCurrentTime() << std::endl;

         nll::core::Timer timer2;
         algorithm::SpeededUpRobustFeatures::Points points2 = surf.computesFeatures( image2 );
         std::cout << "done2=" << timer2.getCurrentTime() << std::endl;

         std::cout << "nbPOints=" << points1.size() << std::endl;
         std::cout << "nbPOints=" << points2.size() << std::endl;

         std::vector< std::pair<ui32, ui32> > repeatablePointIndex = getRepeatablePointPosition( points1, points2, tfm, 4);
         const double repatablePointRatio = (double)repeatablePointIndex.size() / std::max( points1.size(), points2.size() );
         std::cout << "similarity=" << repatablePointRatio << std::endl;

         std::vector< ui32 > orientation = getRepeatablePointOrientation( repeatablePointIndex, points1, points2, tfm, 0.3 );
         const double repeatbleOrientationRatio = (double)orientation.size() / repeatablePointIndex.size();
         std::cout << "repeatable orientation=" << repeatbleOrientationRatio << std::endl;

         TESTER_ASSERT( repatablePointRatio > 0.5 );
         TESTER_ASSERT( repeatbleOrientationRatio > 0.75 );
      }
   }

   void testRegistration()
   {
      const std::string rootOut = NLL_TEST_PATH "data/";

      for ( ui32 n = 0; n < core::getStaticBufferSize( pairs ) / 2; ++n )
      {
         // init
         core::Image<ui8> image( NLL_TEST_PATH + pairs[ 2 * n + 0 ] );

         core::Image<ui8> output;
         output.clone( image );

         core::Image<ui8> image2( NLL_TEST_PATH + pairs[ 2 * n + 1 ] );

         core::Image<ui8> oi1;
         oi1.clone( image );
         core::Image<ui8> oi2;
         oi2.clone( image2 );

         core::Image<ui8> output2;
         output2.clone( image2 );

         TESTER_ASSERT( image.sizex() );
         core::decolor( image );
         core::decolor( image2 );

         algorithm::SpeededUpRobustFeatures surf( 5, 6, 2, 0.00011 );

         nll::core::Timer timer;
         algorithm::SpeededUpRobustFeatures::Points points1 = surf.computesFeatures( image );
         std::cout << "done=" << timer.getCurrentTime() << std::endl;

         nll::core::Timer timer2;
         algorithm::SpeededUpRobustFeatures::Points points2 = surf.computesFeatures( image2 );
         std::cout << "done2=" << timer2.getCurrentTime() << std::endl;

         std::cout << "nbPOints=" << points1.size() << std::endl;
         std::cout << "nbPOints=" << points2.size() << std::endl;


         // match points
         algorithm::SpeededUpRobustFeatures::PointsFeatureWrapper p1Wrapper( points1 );
         algorithm::SpeededUpRobustFeatures::PointsFeatureWrapper p2Wrapper( points2 );

         core::Timer matchingTimer;
         algorithm::impl::FeatureMatcher matcher;
         algorithm::impl::FeatureMatcher::Matches matches;
         matcher.findMatch( p1Wrapper, p2Wrapper, matches );
         std::cout << "nb match=" << matches.size() << std::endl;
         std::cout << "maching time=" << matchingTimer.getCurrentTime() << std::endl;
         

         printPoints( output, points1 );
         core::writeBmp( output, rootOut + "o1-" + core::val2str(n) + ".bmp" );

         printPoints( output2, points2 );
         core::writeBmp( output2, rootOut + "o2-" + core::val2str(n) + ".bmp" );

         // take only the best subset...
         algorithm::impl::FeatureMatcher::Matches matchesTrimmed( matches.begin(), matches.begin() + std::min<ui32>( 200, (ui32)matches.size() - 1 ) );

         core::Image<ui8> output3;
         composeMatch( output, output2, output3, points1, points2, matchesTrimmed );
         core::writeBmp( output3, rootOut + "o3-" + core::val2str(n) + ".bmp" );

         // estimate the transformation
         typedef algorithm::impl::AffineTransformationEstimatorRansac SurfEstimator;
         typedef algorithm::impl::SurfEstimatorFactory<algorithm::impl::AffineTransformationEstimatorRansac> SurfEstimatorFactory;
         typedef algorithm::Ransac<SurfEstimator, SurfEstimatorFactory> Ransac;

         SurfEstimatorFactory estimatorFactory( points1, points2 );
         Ransac ransac( estimatorFactory );

         core::Timer ransacOptimTimer;
         SurfEstimator::Model model = ransac.estimate( matchesTrimmed, 5, 5000, 0.001 );
         std::cout << "ransac optim time=" << ransacOptimTimer.getCurrentTime() << std::endl;
         model.print( std::cout );

         core::Image<ui8> outputReg;
         displayTransformation( oi1, oi2, outputReg, model.tfm );
         core::writeBmp( outputReg, rootOut + "oreg" + core::val2str(n) + ".bmp" );
      }
   }

   void testRegistration2()
   {
      const std::string rootOut = NLL_TEST_PATH "data/";

      for ( ui32 n = 0; n < core::getStaticBufferSize( pairs ) / 2; ++n )
      {
         // init
         core::Image<ui8> image( NLL_TEST_PATH + pairs[ 2 * n + 0 ] );

         core::Image<ui8> output;
         output.clone( image );

         core::Image<ui8> image2( NLL_TEST_PATH + pairs[ 2 * n + 1 ] );

         core::Image<ui8> oi1;
         oi1.clone( image );
         core::Image<ui8> oi2;
         oi2.clone( image2 );

         core::Image<ui8> output2;
         output2.clone( image2 );

         TESTER_ASSERT( image.sizex() );
         core::decolor( image );
         core::decolor( image2 );

         algorithm::AffineRegistrationPointBased2d<> registration;
         core::Matrix<double> regTfm = registration.compute( image, image2 );

         

         core::Image<ui8> outputReg;
         displayTransformation( oi1, oi2, outputReg, regTfm );
         core::writeBmp( outputReg, rootOut + "oreg-2-" + core::val2str(n) + ".bmp" );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSurf);
//TESTER_TEST(testBasic);
//TESTER_TEST(testRepeatability);
//TESTER_TEST(testRegistration);
TESTER_TEST(testRegistration2);
TESTER_TEST_SUITE_END();
#endif
