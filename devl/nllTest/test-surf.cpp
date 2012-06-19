#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;
using namespace nll::algorithm;

namespace nll
{
namespace algorithm
{
   
}
}

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
      "data/feature/sq9.bmp"
   };
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
      for ( ui32 n = 0; n < (ui32)matches.size(); ++n )
      {
         //std::cout << "d=" << matches[ n ].dist << std::endl;
         const algorithm::SpeededUpRobustFeatures::Point& f1 = p1[ matches[ n ].index1 ];
         const algorithm::SpeededUpRobustFeatures::Point& f2 = p2[ matches[ n ].index2 ];
         core::bresham( output, f1.position, core::vector2i( f2.position[ 0 ] + i1.sizex(), f2.position[ 1 ] ), core::vector3uc( (ui8)( rand() % 255 ), (ui8)( rand() % 255 ), (ui8)( rand() % 255 ) ) );
      }
   }

   void composeMatch( const core::Image<ui8>& i1, const core::Image<ui8>& i2, core::Image<ui8>& output,
      algorithm::AffineRegistrationPointBased2d<>::PointPairs& points )
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
      for ( ui32 n = 0; n < (ui32)points.size(); ++n )
      {
         const algorithm::SpeededUpRobustFeatures::Point& f1 = points[ n ].first;
         const algorithm::SpeededUpRobustFeatures::Point& f2 = points[ n ].second;
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
               const double val = ( (c==2) * i2( x, y, c ) * 2 + output( x, y, c ) );
               output( x, y, c ) = NLL_BOUND( val, 0, 255 );
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

      algorithm::SpeededUpRobustFeatures surf( 5, 4, 2, 0.001 );

      nll::core::Timer timer;
      algorithm::SpeededUpRobustFeatures::Points points1 = surf.computesFeatures( image );
      std::cout << "done=" << timer.getCurrentTime() << std::endl;

      std::cout << "nbPOints=" << points1.size() << std::endl;

      printPoints( output, points1 );
      core::writeBmp( output, NLL_TEST_PATH "data/pointsSurf.bmp" );
   }

   void testRepeatability()
   {
      const std::string path = "data/feature/sq1.bmp";
      
      double meanOrientationRepeatability = 0;
      double meanSimilarity = 0;

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

         core::writeBmp( image, "oi1.bmp" );
         core::writeBmp( image2, "oi2.bmp" );

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
         TESTER_ASSERT( repeatbleOrientationRatio >= 0.55 );

         meanOrientationRepeatability += repeatbleOrientationRatio;
         meanSimilarity += repatablePointRatio;
      }

      meanOrientationRepeatability /= 100;
      meanSimilarity /= 100;
      std::cout << "MEAN meanOrientationRepeatability=" << meanOrientationRepeatability << std::endl;
      std::cout << "MEAN repeatability=" << meanSimilarity << std::endl;
   }

   void testRegistration()
   {
      const std::string rootOut = NLL_TEST_PATH "data/";

      for ( ui32 n = 0; n < core::getStaticBufferSize( pairs ) / 2; ++n )
      {
         // init
         core::Image<ui8> image( NLL_TEST_PATH + pairs[ 2 * n + 0 ] );
         //core::addBorder( image, 80, 80 );

         core::Image<ui8> output;
         output.clone( image );

         core::Image<ui8> image2( NLL_TEST_PATH + pairs[ 2 * n + 1 ] );
         //core::addBorder( image2, 80, 80 );

         core::Image<ui8> oi1;
         oi1.clone( image );
         core::Image<ui8> oi2;
         oi2.clone( image2 );

         core::Image<ui8> output2;
         output2.clone( image2 );

         TESTER_ASSERT( image.sizex() );
         core::decolor( image );
         core::decolor( image2 );

         algorithm::SpeededUpRobustFeatures surf( 5, 6, 2, 0.000011 );

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
         algorithm::impl::FeatureMatcher::Matches matchesTrimmed( matches.begin(), matches.begin() + std::min<ui32>( 150, (ui32)matches.size() - 1 ) );

         core::Image<ui8> output3;
         composeMatch( output, output2, output3, points1, points2, matchesTrimmed );
         core::writeBmp( output3, rootOut + "o3-" + core::val2str(n) + ".bmp" );

         // estimate the transformation
         typedef algorithm::impl::SurfEstimatorSimilarityIsotropicFactory              SurfEstimatorFactory;
         typedef algorithm::impl::SurfEstimatorSimilarityIsotropicFactory::Estimator   SurfEstimator;
         typedef algorithm::Ransac<SurfEstimator, SurfEstimatorFactory> Ransac;

         SurfEstimatorFactory estimatorFactory( points1, points2 );
         Ransac ransac( estimatorFactory );

         core::Timer ransacOptimTimer;
         core::Buffer1D<float> weights;
         SurfEstimator::Model model = ransac.estimate( matchesTrimmed, 5, 50000, 0.01, weights );
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
         std::cout << "Registration=" << n << std::endl;
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

         algorithm::AffineRegistrationPointBased2d<> registration( 5, 6, 0.000001 );
         core::Matrix<double> regTfm = registration.compute( image, image2 );

         

         core::Image<ui8> outputReg;
         displayTransformation( oi1, oi2, outputReg, regTfm );
         core::writeBmp( outputReg, rootOut + "oreg-2-" + core::val2str(n) + ".bmp" );
      }
   }

   void createProjections()
   {
      //const std::string inputDir = "D:/devel/sandbox/nllTest/data/reg1/";
      const std::string inputDir = "D:/devel/sandbox/regionDetectionTest/data/";
      //const std::string inputDir = "I:/work/data_CT/";
      const std::string input = inputDir + "list.txt";
      const std::string outputDir = "c:/tmp/proj/";

      typedef nll::imaging::VolumeSpatial<double>           Volume;
      imaging::LookUpTransformWindowingRGB lut( -10, 250, 256, 1 );
      lut.createGreyscale();

      std::ifstream f( input.c_str() );
      int startCase = 0;
      int n = startCase;
      while ( !f.eof() )
      {
         while ( startCase != 0 )
         {
            std::string file;
            std::getline( f, file );
            --startCase;
         }

         std::string file;
         std::getline( f, file );
         if ( file != "" )
         {
            try
            {
               std::cout << "loading:" << file << std::endl;
               Volume ct1;
               bool loaded = nll::imaging::loadSimpleFlatFile( inputDir + file, ct1 );
               if ( loaded )
               {
                  ui32 normSizeY;
                  ui32 normSizeX;
                  core::Image<ui8> py3 = algorithm::AffineRegistrationCT3d<>::projectImageZ( ct1, lut, normSizeY, normSizeX );
                  core::extend( py3, 3 );
                  int ymax = algorithm::AffineRegistrationCT3d<>::findTableY( py3 );
                  if ( ymax > 0 )
                  {
                     for ( ui32 x = 0; x < py3.sizex(); ++x )
                     {
                        for ( ui32 y = ymax; y < py3.sizey(); ++y )
                        {
                           py3( x, y, 0 ) = 0;
                           py3( x, y, 1 ) = 0;
                           py3( x, y, 2 ) = 0;
                        }
                     }
                  } else {
                     ymax = ct1.getSize()[ 1 ] * ct1.getSpacing()[ 1 ] - 1;
                  }
                  core::writeBmp( py3, outputDir + "pz-" + core::val2str( n ) + ".bmp" );

                  

                  core::Image<ui8> py1 = algorithm::AffineRegistrationCT3d<>::projectImageX( ct1, lut, ymax, normSizeX / 2 );
                  core::extend( py1, 3 );
                  core::writeBmp( py1, outputDir + "px-" +  core::val2str( n ) + ".bmp" );

                  core::Image<ui8> py2 = algorithm::AffineRegistrationCT3d<>::projectImageY( ct1, lut, ymax, normSizeY / 2 );
                  core::extend( py2, 3 );
                  core::writeBmp( py2, outputDir + "py-" + core::val2str( n ) + ".bmp" );

               }
               ++n;
            } catch (...)
            {
               std::cout << "EXCEPTION" << std::endl;
            }
         }
      }
   }

   void testProjections()
   {
      const std::string outputDir = "c:/tmp/proj/";
      int n = 0;
      for ( ui32 n = 0; n < 2004; ++n )
      {
         std::cout << "reg=" << n << std::endl;
         core::Image<ui8> py1;
         core::readBmp( py1, outputDir + "py-" + core::val2str( n ) + ".bmp", core::Image<ui8>::Allocator() );
         //core::convolve( py1, core::buildGaussian() );
         //core::addBorder( py1, 40, 40 );

         core::Image<ui8> py2;
         core::readBmp( py2, outputDir + "py-" + core::val2str( n + 1 ) + ".bmp", core::Image<ui8>::Allocator() );
         //core::convolve( py2, core::buildGaussian() );
         //core::addBorder( py2, 40, 40 );

         core::decolor( py1 );
         core::decolor( py2 );
         algorithm::AffineRegistrationPointBased2d<> registration;
         try
         {
            core::Matrix<double> regTfm = registration.compute( py1, py2 );

            core::extend( py1, 3 );
            core::extend( py2, 3 );
            core::Image<ui8> outputReg;
            //regTfm( 0 , 2 ) = 0;
            /*
            for ( ui32 y = 0; y < 2; ++y )
               for ( ui32 x = 0; x < 2; ++x )
                  regTfm( y, x ) = x == y;
                  */
            regTfm.print( std::cout );
            displayTransformation( py1, py2, outputReg, regTfm );
            core::writeBmp( outputReg, outputDir + "../result" + core::val2str( n ) + ".bmp" );
            std::cout << "Run ok..." << std::endl;
         } catch(...)
         {
            std::cout << "Error, not enough inliers..." << std::endl;
         }
      }
   }

   void createTfmVolume()
   {
      typedef nll::imaging::VolumeSpatial<double>           Volume;

      core::Matrix<double> rz;
      core::matrix4x4RotationZ( rz, 0);

      core::Matrix<double> ry;
      core::matrix4x4RotationY( ry, 0.1 );

      core::Matrix<double> rx;
      core::matrix4x4RotationX( rx, -0.1 );

      core::Matrix<double> tfmMat = rz * ry * rx;
      tfmMat( 0, 3 ) = 20;
      tfmMat( 1, 3 ) = 10;
      tfmMat( 2, 3 ) = -35;

      tfmMat.print( std::cout );

      imaging::TransformationAffine tfm( tfmMat );
      tfm.getAffineMatrix().print( std::cout );
      tfm.getInvertedAffineMatrix().print( std::cout );

      Volume ct1;
      const std::string inputDir = "c:/tmp/";
      bool loaded = nll::imaging::loadSimpleFlatFile( inputDir + "sourceo.mf2", ct1 );
      ct1.setOrigin( core::vector3f(1000, 1100, 1350) );

      core::Matrix<float> newPst;
      newPst.clone( ct1.getPst() );
      imaging::VolumeSpatial<double> resampled( ct1.size(), newPst );
      resampled.setOrigin( core::vector3f(1050, 1130, 1390) );

      imaging::resampleVolumeTrilinear( ct1, tfm, resampled );

      imaging::saveSimpleFlatFile( "c:/tmp/target.mf2", resampled );
      imaging::saveSimpleFlatFile( "c:/tmp/source.mf2", ct1 );
   }

   void createTfmVolume2()
   {
      typedef nll::imaging::VolumeSpatial<double>           Volume;

      Volume ct1;
      Volume ct2;
      const std::string inputDir = "D:/devel/sandbox/regionDetectionTest/data/";
      bool loaded = nll::imaging::loadSimpleFlatFile( inputDir + "case38.mf2", ct1 );
      loaded &= nll::imaging::loadSimpleFlatFile( inputDir + "case39.mf2", ct2 );
      TESTER_ASSERT( loaded );
      //ct1.setOrigin( core::vector3f(0, 0, 0) );
      //ct2.setOrigin( core::vector3f(0, 0, 0) );
      //imaging::resampleVolumeTrilinear( ct1, resampled, tfm );

      imaging::saveSimpleFlatFile( "c:/tmp/target.mf2", ct2 );
      imaging::saveSimpleFlatFile( "c:/tmp/source.mf2", ct1 );
   }

   void testResampling()
   {
      const std::string inputDir = "c:/tmp/";
      typedef nll::imaging::VolumeSpatial<float>           Volume;
      core::Matrix<float> tfmMat = core::identityMatrix< core::Matrix<float> >( 4 );

      tfmMat( 0, 0 ) = 0.9211;
      tfmMat( 0, 1 ) = 0;
      tfmMat( 0, 2 ) = -0.3894;
      tfmMat( 0, 3 ) = -32.05;

      tfmMat( 1, 0 ) = -0.0774;
      tfmMat( 1, 1 ) = 0.9801;
      tfmMat( 1, 2 ) = -0.1830;
      tfmMat( 1, 3 ) = -14.6579;

      tfmMat( 2, 0 ) = 0.3817;
      tfmMat( 2, 1 ) = 0.1987;
      tfmMat( 2, 2 ) = 0.9027;
      tfmMat( 2, 3 ) = 21.9747;

      /*
      tfmMat( 0, 0 ) = 1;
      tfmMat( 0, 1 ) = 0;
      tfmMat( 0, 2 ) = 0;
      tfmMat( 0, 3 ) = -32.05;

      tfmMat( 1, 0 ) = 0;
      tfmMat( 1, 1 ) = 0;
      tfmMat( 1, 2 ) = -1;
      tfmMat( 1, 3 ) = -14.6579;

      tfmMat( 2, 0 ) = 0;
      tfmMat( 2, 1 ) = 1;
      tfmMat( 2, 2 ) = 0;
      tfmMat( 2, 3 ) = 221.9747;
      */
      tfmMat( 3, 0 ) = 0;
      tfmMat( 3, 1 ) = 0;
      tfmMat( 3, 2 ) = 0;
      tfmMat( 3, 3 ) = 1;
      imaging::TransformationAffine tfm( tfmMat );

      Volume ct2;
      imaging::loadSimpleFlatFile( inputDir + "target.mf2", ct2 );

      
      core::Matrix<float> pst;
      pst.clone( ct2.getPst() );
      pst( 0, 3 ) += 80;

      Volume resampled( ct2.size(), pst );
      imaging::resampleVolumeTrilinear( ct2, tfm, resampled );
      imaging::saveSimpleFlatFile( "c:/tmp/resampled.mf2", resampled );
   }

   void test()
   {
      typedef nll::imaging::VolumeSpatial<float>           Volume;

      const std::string inputDir = "c:/tmp/";
      const std::string outputDir = "c:/tmp/proj/";

      Volume ct1;
      Volume ct2;
      bool loaded = nll::imaging::loadSimpleFlatFile( inputDir + "source.mf2", ct1 );
          loaded &= nll::imaging::loadSimpleFlatFile( inputDir + "target.mf2", ct2 );
      TESTER_ASSERT( loaded );
      std::cout << "volumes loaded..." << std::endl;

      algorithm::AffineRegistrationCT3d<> ctRegistration;
      core::Matrix<double> tfm;
      core::Timer regTime;
      algorithm::AffineRegistrationCT3d<>::Result r = ctRegistration.process( ct1, ct2, tfm );
      std::cout << "Registration time=" << regTime.getCurrentTime() << std::endl;
      if ( r == algorithm::AffineRegistrationCT3d<>::SUCCESS )
      {
         std::cout << "tfm REG=" << std::endl;
         tfm.print( std::cout );
      } else {
         std::cout << "case error" << std::endl;
      }

      core::Image<ui8> result;
      core::extend( ctRegistration.pxSrc, 3 );
      core::extend( ctRegistration.pxTgt, 3 );
      composeMatch( ctRegistration.pxSrc, ctRegistration.pxTgt, result, ctRegistration.pxInliers );
      core::writeBmp( result, "c:/tmp/reg3d-match-px.bmp" );

      core::extend( ctRegistration.pySrc, 3 );
      core::extend( ctRegistration.pyTgt, 3 );
      composeMatch( ctRegistration.pySrc, ctRegistration.pyTgt, result, ctRegistration.pyInliers );
      core::writeBmp( result, "c:/tmp/reg3d-match-py.bmp" );

      displayTransformation( ctRegistration.pxSrc, ctRegistration.pxTgt, result, ctRegistration.pxTfm );
      core::writeBmp( result, "c:/tmp/reg3d-fused-px.bmp" );
      displayTransformation( ctRegistration.pySrc, ctRegistration.pyTgt, result, ctRegistration.pyTfm );
      core::writeBmp( result, "c:/tmp/reg3d-fused-py.bmp" );

      core::writeBmp( ctRegistration.pySrc, "c:/tmp/py-src.bmp" );
      core::writeBmp( ctRegistration.pxSrc, "c:/tmp/px-src.bmp" );

      core::writeBmp( ctRegistration.pyTgt, "c:/tmp/py-tgt.bmp" );
      core::writeBmp( ctRegistration.pxTgt, "c:/tmp/px-tgt.bmp" );

      // output resampled volume
      Volume vout( ct2.size(), ct2.getPst() );
      imaging::resampleVolumeTrilinear( ct2, tfm, vout  );
      imaging::saveSimpleFlatFile( "c:/tmp/resampled.mf2", vout );
   }

   void test2()
   {
      typedef nll::imaging::VolumeSpatial<float>           Volume;

      const std::string inputDir = "c:/tmp/";
      const std::string outputDir = "c:/tmp/proj/";

      Volume ct1;
      Volume ct2;
      bool loaded = nll::imaging::loadSimpleFlatFile( "\\\\ox4705vc/RegTestData/Random/case1.mf2", ct1 );
          loaded &= nll::imaging::loadSimpleFlatFile( "\\\\ox4705vc/RegTestData/Random/case13.mf2", ct2 );
      TESTER_ASSERT( loaded );
      std::cout << "volumes loaded..." << std::endl;

      typedef algorithm::AffineRegistrationCT3d<algorithm::impl::SurfEstimatorAffineFactory> Registration;
      //typedef algorithm::AffineRegistrationCT3d<algorithm::impl::SurfEstimatorAffineFactory> Registration;
      Registration ctRegistration;
      core::Matrix<double> tfm;
      core::Timer regTime;
      Registration::Result r = ctRegistration.process( ct1, ct2, tfm, true, core::vector3i( -682, -1814, -444 ), core::vector3i( 1423, 2316, 924 ),
                                                                                             core::vector3i( 0, 0, 81 ), core::vector3i( 512, 512, 323 ) );
      std::cout << "Registration time=" << regTime.getCurrentTime() << std::endl;
      if ( r == Registration::SUCCESS )
      {
         std::cout << "tfm REG=" << std::endl;
         tfm.print( std::cout );
      } else {
         std::cout << "case error" << std::endl;
      }

      core::Image<ui8> result;
      core::extend( ctRegistration.pxSrc, 3 );
      core::extend( ctRegistration.pxTgt, 3 );
      composeMatch( ctRegistration.pxSrc, ctRegistration.pxTgt, result, ctRegistration.pxInliers );
      core::writeBmp( result, "c:/tmp/reg3d-match-px.bmp" );

      core::extend( ctRegistration.pySrc, 3 );
      core::extend( ctRegistration.pyTgt, 3 );
      composeMatch( ctRegistration.pySrc, ctRegistration.pyTgt, result, ctRegistration.pyInliers );
      core::writeBmp( result, "c:/tmp/reg3d-match-py.bmp" );

      displayTransformation( ctRegistration.pxSrc, ctRegistration.pxTgt, result, ctRegistration.pxTfm );
      core::writeBmp( result, "c:/tmp/reg3d-fused-px.bmp" );
      displayTransformation( ctRegistration.pySrc, ctRegistration.pyTgt, result, ctRegistration.pyTfm );
      core::writeBmp( result, "c:/tmp/reg3d-fused-py.bmp" );

      core::writeBmp( ctRegistration.pySrc, "c:/tmp/py-src.bmp" );
      core::writeBmp( ctRegistration.pxSrc, "c:/tmp/px-src.bmp" );

      core::writeBmp( ctRegistration.pyTgt, "c:/tmp/py-tgt.bmp" );
      core::writeBmp( ctRegistration.pxTgt, "c:/tmp/px-tgt.bmp" );

      // output resampled volume
      Volume vout( ct2.size(), ct2.getPst() );
      imaging::resampleVolumeTrilinear( ct2, tfm, vout  );
      imaging::saveSimpleFlatFile( "c:/tmp/resampled.mf2", vout );
   }

   void createPairTruncated()
   {
      const std::string outDir = "c:/VolumePairPart/";
      const std::string inDir = "c:/VolumePair/";
      std::ifstream f( ( inDir + "list.txt" ).c_str() );

      std::string line1;
      std::string line2;
      int n = 1;
      while ( !f.eof() )
      {
         std::getline( f, line1 );
         std::getline( f, line2 );

         typedef nll::imaging::VolumeSpatial<float>    Volume;

         Volume target;
         imaging::loadSimpleFlatFile( inDir + line2, target );
         
         ui32 nbSlices = 50 + ( rand() % 10 );
         ui32 position = target.size()[ 2 ] / 2;
         Volume part( core::vector3ui( target.size()[ 0 ],
                                       target.size()[ 1 ],
                                       nbSlices ),
                      target.getPst() );

         for ( ui32 z = 0; z < nbSlices; ++z )
         {
            for ( ui32 x = 0; x < target.size()[ 0 ]; ++x )
            {
               for ( ui32 y = 0; y < target.size()[ 1 ]; ++y )
               {
                  part( x, y, z ) = target( x, y, z + position );
               }
            }
         }

         imaging::saveSimpleFlatFile( outDir + line2, part );
         ++n;
      }
   }

   // take a source image, take random points within the image
   // then do the same for the same target image but rotated
   // check the 2 features are similar despite the rotation
   void testRepeatabilityAngle()
   {
      const std::string path = "data/feature/sq1.bmp";
      
      double meanErrorAngle = 0;
      const ui32 nbCases = 10;


      for ( ui32 n = 0; n < nbCases; ++n )
      {
         // init
         core::Image<ui8> image( NLL_TEST_PATH + path );
         image = core::addBorder( image, 200, 50 );

         core::Image<ui8> image2;
         image2.clone( image );

         //core::TransformationRotation tfm( 0, core::vector2f( 40, 0 ) );
         const double angle = core::generateUniformDistribution( -0.8, 0.8 );
         const double dx = core::generateUniformDistribution( 0, 40 );
         core::TransformationRotation tfm( angle, core::vector2f( -(int)image2.sizex() / 2, -(int)image2.sizey() / 2 ), core::vector2f( (int)image2.sizex() / 2, (int)image2.sizey() / 2 ) );
         core::TransformationRotation tfm2( -angle, core::vector2f( -(int)image2.sizex() / 2, -(int)image2.sizey() / 2 ), core::vector2f( (int)image2.sizex() / 2, (int)image2.sizey() / 2 ) );
         core::transformUnaryFast( image2, tfm );
         /*
         image = core::addBorder( image, 40, 40 );
         image2 = core::addBorder( image2, 40, 40 );
         */
         core::Image<ui8> oi1;
         oi1.clone( image );
         core::Image<ui8> oi2;
         oi2.clone( image2 );

         core::Image<ui8> output2;
         output2.clone( image2 );

         TESTER_ASSERT( image.sizex() );
         core::decolor( image );
         core::decolor( image2 );

         algorithm::SpeededUpRobustFeatures surf( 5, 6, 2, 0.0011 );
         algorithm::SpeededUpRobustFeatures::Points points1 = surf.computesFeatures( image );

         std::vector<ui32> index;
         std::vector<core::vector3ui> sourcePoint;
         std::vector<core::vector3ui> targetPoint;
         for ( ui32 n = 0; n < points1.size(); ++n )
         {
          //  if ( oi1( points1[ n ].position[ 0 ], points1[ n ].position[ 1 ], 0 ) > 100 )
            {
               core::vector2f t = tfm2( core::vector2f( points1[ n ].position[ 0 ],
                                                       points1[ n ].position[ 1 ] ) );

               if ( t[ 0 ] >= 0 && t[ 0 ] < oi2.sizex() &&
                    t[ 1 ] >= 0 && t[ 1 ] < oi2.sizey() )
               {
                  sourcePoint.push_back( core::vector3ui( points1[ n ].position[ 0 ], points1[ n ].position[ 1 ], points1[ n ].scale ) );
                  targetPoint.push_back( core::vector3ui( t[ 0 ], t[ 1 ], points1[ n ].scale ) );
                  index.push_back( n );
               }
            }
         }

         for ( ui32 n = 0; n < sourcePoint.size(); ++n )
         {
            oi1( sourcePoint[ n ][ 0 ], sourcePoint[ n ][ 1 ], 2 ) = 255;
            oi1( sourcePoint[ n ][ 0 ], sourcePoint[ n ][ 1 ], 1 ) = 0;
            oi1( sourcePoint[ n ][ 0 ], sourcePoint[ n ][ 1 ], 0 ) = 0;

            oi2( targetPoint[ n ][ 0 ], targetPoint[ n ][ 1 ], 2 ) = 255;
            oi2( targetPoint[ n ][ 0 ], targetPoint[ n ][ 1 ], 1 ) = 0;
            oi2( targetPoint[ n ][ 0 ], targetPoint
               [ n ][ 1 ], 0 ) = 0;
         }

         // compute the corresponding points, we must find similar values in features, and shifted angle...
         algorithm::SpeededUpRobustFeatures::Points transformed = createFrom( targetPoint );
         surf.computesFeatures( image, transformed );

         float errorAngle = 0;
         ui32 nbAngles = 0;
         for ( ui32 n = 0; n < transformed.size(); ++n )
         {
            if ( !core::equal<float>( transformed[ n ].orientation, 0, 1e-3 ) )
            {
               // normalize the angles
               float found = transformed[ n ].orientation;
               float expected = points1[ index[ n ] ].orientation + angle;
               if ( found - expected > core::PIf )
               {
                  found -= 2 * core::PIf;
               }

               if ( expected - found > core::PIf )
               {
                  expected -= 2 * core::PIf;
               }

               errorAngle += fabs( found - expected );
               ++nbAngles;
               std::cout << "angle=" << found << " expected=" << expected << std::endl;
            }
         }
         errorAngle /= nbAngles;
         meanErrorAngle += errorAngle;

         std::cout << "Angle error=" << errorAngle << std::endl;


         printPoints( oi2, transformed );
         printPoints( oi1, points1 );

         core::writeBmp( oi1, NLL_TEST_PATH "data/oi1-" + core::val2str( n ) + ".bmp" );
         core::writeBmp( oi2, NLL_TEST_PATH "data/oi2-" + core::val2str( n ) + ".bmp" );
      }

      meanErrorAngle /= nbCases;

      std::cout << "Angle mean error=" << meanErrorAngle << std::endl;
      TESTER_ASSERT( meanErrorAngle < 1.25 );
   }

   algorithm::SpeededUpRobustFeatures::Points createFrom( const std::vector<core::vector3ui>& points )
   {
      algorithm::SpeededUpRobustFeatures::Points newPoints;
      std::for_each( points.begin(), points.end(), [&](const core::vector3ui& pos)
      {
         algorithm::SpeededUpRobustFeatures::Point p;
         p.position[ 0 ] = pos[ 0 ];
         p.position[ 1 ] = pos[ 1 ];
         p.scale         = pos[ 2 ];
         newPoints.push_back( p );
      });

      return newPoints;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSurf);
TESTER_TEST(testBasic);
TESTER_TEST(testRepeatability);
TESTER_TEST(testRepeatabilityAngle);
TESTER_TEST(testRegistration2);   // test full registration

// UTILITY FUNCTIONS
//TESTER_TEST(testRegistrationVolume);
//TESTER_TEST(createProjections);   // create projection of hundreds of cases YZ or XZ
//TESTER_TEST(testProjections);     // test the registration on the projections
//TESTER_TEST(createTfmVolume);     // create a normalized volume
//TESTER_TEST(createTfmVolume2);    // create a normalized volume
//TESTER_TEST(createPairTruncated); // create from a pair of volume, a pair of truncated volume
//TESTER_TEST(test);                // test volume
//TESTER_TEST(test2);               // test volume
//TESTER_TEST(testRegistration);    // test volume
//TESTER_TEST(testRegistration2);   // test volume
//TESTER_TEST(testResampling);      // test resampling
//TESTER_TEST(testRegistration);   // check the slices are registered
TESTER_TEST_SUITE_END();
#endif
