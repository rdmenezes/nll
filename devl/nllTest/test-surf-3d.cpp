#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>
#include "config.h"

//#define NLL_NOT_MULTITHREADED

using namespace nll;

namespace nll
{
namespace algorithm
{
   

   
}
}

class TestSurf3D
{
public:
   void testSurf3d()
   {
      const std::string volname = NLL_TEST_PATH "data/medical/ct.mf2";

      imaging::VolumeSpatial<float> volume;
      
      std::cout << "loadind..." << std::endl;
      bool loaded = nll::imaging::loadSimpleFlatFile( volname, volume );
      TESTER_ASSERT( loaded );

      imaging::LookUpTransformWindowingRGB lut( -100, 600, 256, 1 );
      lut.createGreyscale();
      for ( imaging::VolumeSpatial<float>::iterator it = volume.begin(); it != volume.end(); ++it )
         *it = lut.transform( *it )[ 0 ];

      std::cout << "runing surf..." << std::endl;
      algorithm::SpeededUpRobustFeatures3d surf( 5, 4, 2, 0.01 );

      algorithm::SpeededUpRobustFeatures3d::Points points = surf.computesFeatures( volume );
      std::cout << "nbPoints=" << points.size() << std::endl;
      std::ofstream f( "c:/tmp/points.txt" );
      for ( size_t n = 0; n < points.size(); ++n )
      {
         const core::vector3f point = volume.indexToPosition( core::vector3f( points[ n ].position[ 0 ],
                                                                              points[ n ].position[ 1 ],
                                                                              points[ n ].position[ 2 ] ) );
         f << point[ 0 ] << " " << point[ 1 ] << " " << point[ 2 ] << " " << ( points[ n ].scale * volume.getSpacing()[ 0 ] ) << std::endl;
      }

      // TESTER_ASSERT( points.size() > 500 );
   }

   void testcartesianToSphericalCoordinate()
   {
      // checked against matlab cart2sph(x,y,z)
      {
         core::vector3d pos( 3, 0, 0 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 3, 1e-8 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0, 1e-8 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0, 1e-8 ) );
      }

      {
         core::vector3d pos( 1, 1, 0 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 1.4142, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0.7854, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0, 1e-4 ) );
      }

      {
         core::vector3d pos( 0, 0, 1 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 1, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 1.5708, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, 20, 15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, 20, 15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, -20, 15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, -20, 15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, 20, -15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, 20, -15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, -20, -15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, -20, -15 );
         core::vector3d result = core::cartesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      // test identity
      for ( size_t n = 0; n < 100; ++n )
      {
         const core::vector3d p( core::generateUniformDistribution( -100, 100 ),
                                 core::generateUniformDistribution( -100, 100 ),
                                 core::generateUniformDistribution( -100, 100 ) );

         const core::vector3d spherical = core::cartesianToSphericalCoordinate( p );
         const core::vector3d back = core::sphericalToCartesianCoordinate( spherical );

         const double norm = ( back - p ).norm2();
         TESTER_ASSERT( norm < 1e-3 );
      }
   }

   void testRotation3d()
   {
      for ( size_t n = 0; n < 100; ++n )
      {
         const double a1 = core::generateUniformDistribution( -core::PI, core::PI );
         const double a2 = core::generateUniformDistribution( -core::PI, core::PI );

         const core::vector3d p( 1, 0, 0 );
         const core::vector3d expectedp = core::sphericalToCartesianCoordinate( core::vector3d( 1, a1, a2 ) );

         algorithm::SpeededUpRobustFeatures3d::RotationFromSpherical rot( a1, a2 );
         const core::vector3d r = rot.transform( p );
         TESTER_ASSERT( (expectedp - r).norm2() < 1e-3 );
      }
   }

   void testRotation3dTfmId()
   {
      for ( size_t n = 0; n < 100; ++n )
      {
         const double a1 = core::generateUniformDistribution( -core::PI, core::PI );
         const double a2 = core::generateUniformDistribution( -core::PI, core::PI );

         core::vector3d p( core::generateUniformDistribution( -10, 10),
                           core::generateUniformDistribution( -10, 10),
                           core::generateUniformDistribution( -10, 10) );

         algorithm::SpeededUpRobustFeatures3d::RotationFromSpherical rot( a1, a2 );
         algorithm::SpeededUpRobustFeatures3d::RotationFromSpherical rotInv = rot.createInverse();
         const core::vector3d pt = rot.transform( p );
         const core::vector3d ptinv = rotInv.transform( pt );
         TESTER_ASSERT( (ptinv - p).norm2() < 1e-3 );
      }
   }

   // brief create artificial data with expected location of the point supposed to have the best response
   // we also know the best response scale which is sqrt(2) * circleSize * 2
   //
   // in these cases we are testing the position and the scale of the responses are clustered at the expected location,
   // unfortunately we can't reliably test the angle as even if we move by 1 pixel, the angle will be very different as the shape is a circle!
   void testPointLocation()
   {
      double averageErrorx = 0;
      double averageErrory = 0;
      double averageErrorz = 0;
      double averageErrors = 0;
      size_t nbSamples = 0;
      const size_t imageSize = 512;
      for ( ui32 n = 0; n < 10; ++n )
      {
         srand( n + 42 );
         std::cout << "case=" << n << std::endl;

         const size_t circleSize = 50 + core::generateUniformDistribution( -10, 40 );
         const double centerx = (double)imageSize / 2 + core::generateUniformDistribution( -30, 30 );
         const double centery = (double)imageSize / 2 + core::generateUniformDistribution( -30, 30 );
         const double centerz = (double)imageSize / 2 + core::generateUniformDistribution( -30, 30 );
         const double scale = circleSize * std::sqrt( 2.0 ) * 2;
         std::cout << "ground truth=" << centerx << " " << centery << " " << centerz << " " << scale << std::endl;
         imaging::Volume<ui8> image( imageSize, imageSize, imageSize );
         for ( size_t z = 0; z < image.sizez(); ++z )
         {
            for ( size_t y = 0; y < image.sizey(); ++y )
            {
               for ( size_t x = 0; x < image.sizex(); ++x )
               {
                  const double d = std::sqrt( core::sqr( (double)x - centerx ) +
                                              core::sqr( (double)y - centery ) +
                                              core::sqr( (double)z - centerz ) );
                  if ( d < circleSize )
                  {
                     image( x, y, z ) = 200;
                  }
               }
            }
         }

         algorithm::SpeededUpRobustFeatures3d surf( 5, 6, 2, 0.04 );
         algorithm::SpeededUpRobustFeatures3d::Points points1 = surf.computesFeatures( image );
         std::cout << "nbPoints=" << points1.size() << std::endl;

         ensure( points1.size() == 1, "select the point with the highest response. Must be at the center of the circle" );

         if ( points1.size() )
         {
            static const double scaleFactor = 1.2 / 9;
            for ( size_t n = 0; n < points1.size(); ++n )
            {
               std::cout << "found=" << points1[ n ].position[ 0 ] << " " << points1[ n ].position[ 1 ] << " " << points1[ n ].position[ 2 ] << " " << points1[ n ].scale / scaleFactor << std::endl;
            }

            const double errorx = fabs( points1[ 0 ].position[ 0 ] - centerx ) / ( 2 * circleSize );
            const double errory = fabs( points1[ 0 ].position[ 1 ] - centery ) / ( 2 * circleSize );
            const double errorz = fabs( points1[ 0 ].position[ 2 ] - centerz ) / ( 2 * circleSize );
            const double errors = fabs( ( points1[ 0 ].scale / scaleFactor ) /  scale );

            ++nbSamples;
            averageErrorx += errorx;
            averageErrory += errory;
            averageErrorz += errorz;
            averageErrors += errors;
            std::cout << "error=" << errorx << " " << errory << " " << errorz << " " << errors << std::endl;

            core::Image<ui8> xyView = print_xy( image, core::vector3f( centerx, centery, centerz ), points1 );
            core::writeBmp( xyView, NLL_TEST_PATH "data/" + core::val2str( n ) + "-xy.bmp" );

            core::Image<ui8> xzView = print_xz( image, core::vector3f( centerx, centery, centerz ), points1 );
            core::writeBmp( xzView, NLL_TEST_PATH "data/" + core::val2str( n ) + "-xz.bmp" );

            ensure( errorx < 0.15, "bad x" );
            ensure( errory < 0.15, "bad y" );
            ensure( errorz < 0.15, "bad z" );

            ensure( errors > 0.9, "bad s" );
            ensure( errors < 1.3, "bad s" );
         }
      }

      averageErrorx /= nbSamples;
      averageErrory /= nbSamples;
      averageErrorz /= nbSamples;
      averageErrors /= nbSamples;

      std::cout << "average error=" << averageErrorx << " " << averageErrory << " " << averageErrorz << " " << averageErrors << std::endl;
      ensure( averageErrorx < 0.05, "bad x" );
      ensure( averageErrory < 0.05, "bad y" );
      ensure( averageErrorz < 0.05, "bad z" );
      ensure( fabs( averageErrors - 1 ) < 0.15, "bad scale" );
   }

   core::Image<ui8> print_xy( const imaging::Volume<ui8>& image, const core::vector3f& groundTruth, const algorithm::SpeededUpRobustFeatures3d::Points& points )
   {
      core::Image<ui8> i( image.sizex(), image.sizey(), 3, false );
      for ( size_t y = 0; y < image.sizey(); ++y )
      {
         for ( size_t x = 0; x < image.sizex(); ++x )
         {
            i( x, y, 0 ) = image( x, y, groundTruth[ 2 ] );
            i( x, y, 1 ) = image( x, y, groundTruth[ 2 ] );
            i( x, y, 2 ) = image( x, y, groundTruth[ 2 ] );
         }
      }

      static const double scaleFactor = 1.2 / 9;

      for ( size_t n = 0; n < points.size(); ++n )
      {
         srand( 2 * n + 50 );
         ui8 v = rand() % 255;
         core::vector3uc color( v, 255 - v, rand() % 255 );

         size_t px = points[ n ].position[ 0 ];
         size_t py = points[ n ].position[ 1 ];
         size_t scale = points[ n ].scale / scaleFactor;
         size_t half = scale / 2;

         int dx = (int)(cos( points[ n ].orientation1 ) * half);
         int dy = (int)(sin( points[ n ].orientation1 ) * half);
         if ( px > 5 &&
              py > 5 &&
              px + dx < i.sizex() - 1 &&
              py + dy < i.sizey() - 1 &&
              px + dx > 0 &&
              py + dy > 0 )
         {
            core::bresham( i, core::vector2i( px + 5, py ), core::vector2i( px - 5, py ),    core::vector3uc(255, 255, 255) );
            core::bresham( i, core::vector2i( px, py - 5 ), core::vector2i( px, py + 5 ),    core::vector3uc(255, 255, 255) );
            core::bresham( i, core::vector2i( px, py ),     core::vector2i( px + dx, py + dy), color );
         }
      }
      return i;
   }

   core::Image<ui8> print_xz( const imaging::Volume<ui8>& image, const core::vector3f& groundTruth, const algorithm::SpeededUpRobustFeatures3d::Points& points )
   {
      core::Image<ui8> i( image.sizex(), image.sizez(), 3, false );
      for ( size_t z = 0; z < image.sizez(); ++z )
      {
         for ( size_t x = 0; x < image.sizex(); ++x )
         {
            i( x, z, 0 ) = image( x, groundTruth[ 1 ], z );
            i( x, z, 1 ) = image( x, groundTruth[ 1 ], z );
            i( x, z, 2 ) = image( x, groundTruth[ 1 ], z );
         }
      }

      static const double scaleFactor = 1.2 / 9;

      for ( size_t n = 0; n < points.size(); ++n )
      {
         srand( 2 * n + 50 );
         ui8 v = rand() % 255;
         core::vector3uc color( v, 255 - v, rand() % 255 );

         size_t px = points[ n ].position[ 0 ];
         size_t pz = points[ n ].position[ 2 ];
         size_t scale = points[ n ].scale / scaleFactor;
         size_t half = scale / 2;

         int dx = (int)(cos( points[ n ].orientation1 ) * half);
         int dz = (int)(sin( points[ n ].orientation1 ) * half);
         if ( px > 5 &&
              pz > 5 &&
              px + dx < i.sizex() - 1 &&
              pz + dz < i.sizey() - 1 &&
              px + dx > 0 &&
              pz + dz > 0 )
         {
            core::bresham( i, core::vector2i( px + 5, pz ), core::vector2i( px - 5, pz ),    core::vector3uc(255, 255, 255) );
            core::bresham( i, core::vector2i( px, pz - 5 ), core::vector2i( px, pz + 5 ),    core::vector3uc(255, 255, 255) );
            core::bresham( i, core::vector2i( px, pz ),     core::vector2i( px + dx, pz + dz), color );
         }
      }
      return i;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSurf3D);
TESTER_TEST(testSurf3d);
TESTER_TEST(testcartesianToSphericalCoordinate);
TESTER_TEST(testRotation3d);
TESTER_TEST(testRotation3dTfmId);
//TESTER_TEST(testPointLocation);
TESTER_TEST_SUITE_END();
#endif