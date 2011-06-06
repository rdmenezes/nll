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

      "data/feature/xz-case-1.bmp",
      "data/feature/xz-case-10.bmp",

      "data/feature/xz-case-13.bmp",
      "data/feature/xz-case-14.bmp",

      "data/feature/xz-case-15.bmp",
      "data/feature/xz-case-17.bmp",

      "data/feature/xz-case-18.bmp",
      "data/feature/xz-case-19.bmp",

      "data/feature/xz-case-2.bmp",
      "data/feature/xz-case-20.bmp",

      "data/feature/xz-case-3.bmp",
      "data/feature/xz-case-5.bmp",

      "data/feature/xz-case-3.bmp",
      "data/feature/xz-case-20.bmp",

      "data/feature/xz-case-228.bmp",
      "data/feature/xz-case-500.bmp",
   };
}

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief 3D registration of CT-CT medical volume
    
    Internally, it is assumed the volume is correctly oriented in a Head-First-Supine orientation
    (this is assumed for the table removal algorithm). The volume is then projected on the XZ, YZ and XY
    planes.
    The XY plane is used to determine the Y position of the table, everything below this position will be removed.
    Finally the 2 pairs of projection are registered using a SURF 2D - ransac - affine estimator one by one and
    a global transformation matrix is computed.    
    */
   class AffineRegistrationCT3d
   {
   public:
   private:
      /*
      static core::Image<ui8> projectImageY( const imaging::VolumeSpatial<double>& v, const imaging::LookUpTransformWindowingRGB& lut, int ymax )
      {
         core::Image<ui8> p( v.getSize()[ 0 ] * v.getSpacing()[ 0 ], v.getSize()[ 2 ] * v.getSpacing()[ 2 ], 3 );
         for ( ui32 z = 0; z < v.getSize()[ 2 ] * v.getSpacing()[ 2 ] - 1; ++z )
         {
            for ( ui32 x = 0; x < v.getSize()[ 0 ] * v.getSpacing()[ 0 ] - 1; ++x )
            {
               double accum = 0;
               for ( ui32 y = 0; y < ymax / v.getSpacing()[ 1 ]; ++y )
               {
                  const double val = lut.transform( v( x / v.getSpacing()[ 0 ], y, z / v.getSpacing()[ 2 ] ) )[ 0 ];
                  accum += val;
                  accum /= v.getSize()[ 1 ];

                  const ui8 val = static_cast<ui8>( accum );
                  p( x, z, 0 ) = val;
                  p( x, z, 1 ) = val;
                  p( x, z, 2 ) = val;
               
            }
         }

         return p;
      }

      static core::Image<ui8> projectImageX( const imaging::VolumeSpatial<double>& v, const imaging::LookUpTransformWindowingRGB& lut, int ymax )
      {
         core::Image<ui8> p( v.getSize()[ 1 ] * v.getSpacing()[ 1 ], v.getSize()[ 2 ] * v.getSpacing()[ 2 ], 3 );
         for ( ui32 z = 0; z < v.getSize()[ 2 ] * v.getSpacing()[ 2 ] - 1; ++z )
         {
            for ( ui32 y = 0; y < ymax; ++y )
            {
               double accum = 0;
               for ( ui32 x = 0; x < v.getSize()[ 0 ]; ++x )
               {
                  const double val = lut.transform( v( x, y / v.getSpacing()[ 1 ], z / v.getSpacing()[ 2 ] ) )[ 0 ];
                  accum += val;
               }
               accum /= v.getSize()[ 0 ];
               const ui8 val = static_cast<ui8>( accum );

               p( y, z, 0 ) = val;
               p( y, z, 1 ) = val;
               p( y, z, 2 ) = val;
               
            }
         }

         return p;
      }

      static core::Image<ui8> projectImageZ( const imaging::VolumeSpatial<double>& v, const imaging::LookUpTransformWindowingRGB& lut )
      {
         core::Image<ui8> p( v.getSize()[ 0 ] * v.getSpacing()[ 0 ], v.getSize()[ 1 ] * v.getSpacing()[ 1 ], 3 );
         for ( ui32 x = 0; x < v.getSize()[ 0 ] * v.getSpacing()[ 0 ] - 1; ++x )
         {
            for ( ui32 y = 0; y < v.getSize()[ 1 ] * v.getSpacing()[ 1 ] - 1; ++y )
            {
               double accum = 0;
               for ( ui32 z = 0; z < v.getSize()[ 2 ]; ++z )
               {
                  const double val = lut.transform( v( x / v.getSpacing()[ 0 ], y / v.getSpacing()[ 1 ], z ) )[ 0 ];
                  accum += val;
               }

               accum /= v.getSize()[ 0 ];
               const ui8 val = static_cast<ui8>( accum );

               p( x, y, 0 ) = val;
               p( x, y, 1 ) = val;
               p( x, y, 2 ) = val;
               
            }
         }

         return p;
      }*/
   };
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
         SurfEstimator::Model model = ransac.estimate( matchesTrimmed, 5, 50000, 0.01 );
         std::cout << "ransac optim time=" << ransacOptimTimer.getCurrentTime() << std::endl;
         model.print( std::cout );

         core::Image<ui8> outputReg;
         displayTransformation( oi1, oi2, outputReg, model.tfm );
         core::writeBmp( outputReg, rootOut + "oreg" + core::val2str(n) + ".bmp" );
      }
   }

   // assume a GREY LUT
   core::Image<ui8> projectImageY( const imaging::VolumeSpatial<double>& v, const imaging::LookUpTransformWindowingRGB& lut, int ymax, ui32 maxSizeY )
   {
      std::cout << "size=" << v.getSize() << std::endl;
      core::Image<ui8> p( v.getSize()[ 0 ] * v.getSpacing()[ 0 ], v.getSize()[ 2 ] * v.getSpacing()[ 2 ], 3 );
      for ( ui32 z = 0; z < v.getSize()[ 2 ] * v.getSpacing()[ 2 ] - 1; ++z )
      {
         for ( ui32 x = 0; x < v.getSize()[ 0 ] * v.getSpacing()[ 0 ] - 1; ++x )
         {
            double accum = 0;
            //ui32 nbVoxelsNonEmpty = 0;
            for ( ui32 y = 0; y < ymax / v.getSpacing()[ 1 ]; ++y )
            {
               const double val = lut.transform( v( x / v.getSpacing()[ 0 ], y, z / v.getSpacing()[ 2 ] ) )[ 0 ];
               accum += val;
            //   if ( val > 0 )
            //      ++nbVoxelsNonEmpty;
            }
            /*
            if ( nbVoxelsNonEmpty < 1 )
                  accum = 0;
               else
                  accum /= nbVoxelsNonEmpty;
                  */
            accum /= ( maxSizeY * v.getSpacing()[ 1 ] );

            const ui8 val = static_cast<ui8>( NLL_BOUND( accum, 0, 255 ) );
            p( x, z, 0 ) = val;
            p( x, z, 1 ) = val;
            p( x, z, 2 ) = val;
            
         }
      }

      return p;
   }

   core::Image<ui8> projectImageX( const imaging::VolumeSpatial<double>& v, const imaging::LookUpTransformWindowingRGB& lut, int ymax, ui32 maxSizeY )
   {
      std::cout << "size=" << v.getSize() << std::endl;
      core::Image<ui8> p( v.getSize()[ 1 ] * v.getSpacing()[ 1 ], v.getSize()[ 2 ] * v.getSpacing()[ 2 ], 3 );
      for ( ui32 z = 0; z < v.getSize()[ 2 ] * v.getSpacing()[ 2 ] - 1; ++z )
      {
         for ( ui32 y = 0; y < ymax; ++y )
         {
            double accum = 0;
            ui32 nbVoxelsNonEmpty = 0;
            for ( ui32 x = 0; x < v.getSize()[ 0 ]; ++x )
            {
               const double val = lut.transform( v( x, y / v.getSpacing()[ 1 ], z / v.getSpacing()[ 2 ] ) )[ 0 ];
               accum += val;
               if ( val > 0 )
                  ++nbVoxelsNonEmpty;
            }
            /*
            if ( nbVoxelsNonEmpty < 50 && accum / nbVoxelsNonEmpty > 180 )
                  accum = 0;
               else
                  accum /= nbVoxelsNonEmpty;
                  */
            accum /= ( maxSizeY * v.getSpacing()[ 0 ] );

            const ui8 val = static_cast<ui8>( NLL_BOUND( accum, 0, 255 ) );
            p( y, z, 0 ) = val;
            p( y, z, 1 ) = val;
            p( y, z, 2 ) = val;
            
         }
      }

      return p;
   }

   core::Image<ui8> projectImageZ( const imaging::VolumeSpatial<double>& v, const imaging::LookUpTransformWindowingRGB& lut, ui32& maxSizeY, ui32& maxSizeX )
   {
      std::cout << "size=" << v.getSize() << std::endl;
      core::Image<ui8> p( v.getSize()[ 0 ] * v.getSpacing()[ 0 ], v.getSize()[ 1 ] * v.getSpacing()[ 1 ], 3 );
      ui32 min = p.sizey() - 1;
      ui32 max = 0;

      ui32 minX = p.sizex() - 1;
      ui32 maxX = 0;
      for ( ui32 x = 0; x < v.getSize()[ 0 ] * v.getSpacing()[ 0 ] - 1; ++x )
      {
         for ( ui32 y = 0; y < v.getSize()[ 1 ] * v.getSpacing()[ 1 ] - 1; ++y )
         {
            double accum = 0;
            for ( ui32 z = 0; z < v.getSize()[ 2 ]; ++z )
            {
               const double val = lut.transform( v( x / v.getSpacing()[ 0 ], y / v.getSpacing()[ 1 ], z ) )[ 0 ];
               accum += val;
            }

            accum /= v.getSize()[ 0 ];

            const ui8 val = static_cast<ui8>( NLL_BOUND( accum * 3, 0, 255 ) );
            p( x, y, 0 ) = val;
            p( x, y, 1 ) = val;
            p( x, y, 2 ) = val;
            if ( val )
            {
               if ( min > y )
               {
                  min = y;
               }

               if ( max < y )
               {
                  max = y;
               }

               if ( minX > x )
               {
                  minX = x;
               }

               if ( maxX < x )
               {
                  maxX = x;
               }
            }
            
         }
      }

      maxSizeY = max - min + 1;
      maxSizeX = maxX - minX + 1;
      return p;
   }

   // find the table: from top to bottom, the Y position can be determined: detect the points top to bottom with only a few connected
   // pixels
   int findTableY( const core::Image<ui8>& iz )
   {
      ui32 nbPixelTable = 0;
      double mean = 0;
      for ( ui32 x = 0; x < iz.sizex(); ++x )
      {
         ui32 lineId = 0;
         ui32 nbConnected[ 5 ] = {0, 0, 0, 0, 0};
         int ymin[ 5 ];
         int ymax[ 5 ];
         for ( int y = iz.sizey() - 1; y > 0; --y )
         {
            if ( iz( x, y, 0 ) > 0 )
            {
               if ( nbConnected[ lineId ] == 0 )
                  ymin[ lineId ] = y;
               ++nbConnected[ lineId ];
            } else if ( nbConnected[ lineId ] && abs( y - ymin[ lineId ] ) < 30 )
            {
               ymax[ lineId ] = y;
               ++lineId;
               if ( lineId >= 3)
                  break;
            }
         }

         if ( lineId )
         {
            ++nbPixelTable;
            mean += ymax[ lineId - 1 ];
         }
      }

      std::cout << "nbPixel=" << nbPixelTable << " mean=" << (mean / nbPixelTable) << std::endl;

      if ( nbPixelTable > 120 )
         return mean / nbPixelTable;
      return -1;
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

         algorithm::AffineRegistrationPointBased2d<> registration;
         core::Matrix<double> regTfm = registration.compute( image, image2 );

         

         core::Image<ui8> outputReg;
         displayTransformation( oi1, oi2, outputReg, regTfm );
         core::writeBmp( outputReg, rootOut + "oreg-2-" + core::val2str(n) + ".bmp" );
      }
   }
   /*
   void testRegistrationVolume()
   {
      const std::string rootOut = NLL_TEST_PATH "data/";
      const std::string ct1name = NLL_TEST_PATH "../regionDetectionTest/data/case51.mf2";
      const std::string ct2name = NLL_TEST_PATH "../regionDetectionTest/data/case65.mf2";
      
      typedef nll::imaging::VolumeSpatial<double>           Volume;
      imaging::LookUpTransformWindowingRGB lut( -250, 250, 256, 1 );
      lut.createGreyscale();

      Volume ct1;
      Volume ct2;

      std::cout << "loading..." << std::endl;
      bool loaded = nll::imaging::loadSimpleFlatFile( ct1name, ct1 );
      TESTER_ASSERT( loaded );

      loaded = nll::imaging::loadSimpleFlatFile( ct2name, ct2 );
      TESTER_ASSERT( loaded );

      core::Image<ui8> py1 = projectImageX( ct1, lut );
      core::writeBmp( py1, rootOut + "py1.bmp" );

      core::Image<ui8> py2 = projectImageX( ct2, lut );
      core::writeBmp( py2, rootOut + "py2.bmp" );

      core::decolor( py1 );
      core::decolor( py2 );
      algorithm::AffineRegistrationPointBased2d<> registration;
      core::Matrix<double> regTfm = registration.compute( py1, py2 );

      core::extend( py1, 3 );
      core::extend( py2, 3 );
      core::Image<ui8> outputReg;
      displayTransformation( py1, py2, outputReg, regTfm );
      core::writeBmp( outputReg, rootOut + "result.bmp" );
   }
   */
   void createProjections()
   {
      const std::string inputDir = "D:/devel/sandbox/nllTest/data/reg1/";
      //const std::string inputDir = "I:/work/data_CT/";
      const std::string input = inputDir + "list.txt";
      const std::string outputDir = "c:/tmp/proj/";

      typedef nll::imaging::VolumeSpatial<double>           Volume;
      imaging::LookUpTransformWindowingRGB lut( -10, 250, 256, 1 );
      lut.createGreyscale();

      std::ifstream f( input.c_str() );
      int n = 0;
      while ( !f.eof() )
      {
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
                  core::Image<ui8> py3 = projectImageZ( ct1, lut, normSizeY, normSizeX );
                  int ymax = findTableY( py3 );
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

                  

                  core::Image<ui8> py1 = projectImageX( ct1, lut, ymax, normSizeX / 2 );
                  core::writeBmp( py1, outputDir + "px-" +  core::val2str( n ) + ".bmp" );

                  core::Image<ui8> py2 = projectImageY( ct1, lut, ymax, normSizeY / 2 );
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
      for ( ui32 n = 0; n < 100; ++n )
      {
         std::cout << "reg=" << n << std::endl;
         core::Image<ui8> py1;
         core::readBmp( py1, outputDir + "py-" + core::val2str( n + 1 ) + ".bmp" );
         //core::convolve( py1, core::buildGaussian() );
         //core::addBorder( py1, 40, 40 );

         core::Image<ui8> py2;
         core::readBmp( py2, outputDir + "py-" + core::val2str( n ) + ".bmp" );
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

   void test()
   {
      const std::string outputDir = "c:/tmp/proj/";
      core::Image<ui8> py2;
      core::readBmp( py2, outputDir + "pz-" + core::val2str( 2 ) + ".bmp" );

      int ymax = findTableY( py2 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSurf);
//TESTER_TEST(testBasic);
//TESTER_TEST(testRepeatability);
//TESTER_TEST(testRegistration);
//TESTER_TEST(testRegistration2);
//TESTER_TEST(testRegistrationVolume);
//TESTER_TEST(createProjections);
TESTER_TEST(testProjections);
//TESTER_TEST(test);
TESTER_TEST_SUITE_END();
#endif
