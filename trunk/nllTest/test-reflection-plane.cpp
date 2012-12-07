#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"
#include "utils.h"

using namespace nll;

namespace nll
{
namespace algorithm
{
  

   /**
    @brief Find the symmetry plane of a volume
    @see Evaluation of the symmetry plane in 3D MR brain images,
         Alexander V. Tuzikov, Olivier Colliot, Isabelle Bloch
         http://perso.telecom-paristech.fr/~bloch/papers/PRL03_Olivier.pdf
    */
   class ReflectionPlaneCalculator
   {
   public:
      typedef EllipsoidAxesOfInertia::Matrix   Matrix;

   public:
      template <class T, class Buffer>
      core::GeometryPlane process( const imaging::VolumeSpatial<T, Buffer>& volume )  const
      {
         core::GeometryPlane plane;
         return plane;
      }

      /**
       @brief find a good initial transformation. ASUMPTION: the plane defined by highest and second highest inertia angles is defining the symmetry plane
       @note if the volume is far from the plane of symmetry (eg., rotation > 90 degrees), this will be problematic


       - The center of mass will be centered
       - the normal of the plane defined by the two biggest ellipsoid axis will be aligned using the smallest possible transformation
         (ie.e., by aligning with the closes space axis (1, 0, 0), (0, 1, 0), (0, 0, 1)
       - other axis are left unchanged
       */
      template <class T, class Buffer>
      Matrix computeInitialTransformation( const imaging::VolumeSpatial<T, Buffer>& volume )  const
      {
         const double eps = 1e-3;

         {
            std::stringstream ss;
            ss << "ReflectionPlaneCalculator::computeInitialTransformation" << std::endl;
            volume.print( ss );
            ss << "eps=" << eps << std::endl;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // find initial range
         imaging::LookUpTransformWindowingRGB lut( 100, 8000, 256, 1 );
         lut.createGreyscale();
         lut.detectRange( volume, 0.8, 256 );

         // increase the minimum so that we are not capturing the background values
         const double range = lut.getMax() - lut.getMin();
         lut.reset( lut.getMin() + range * 0.1, lut.getMax() - range * 0.5, lut.getSize() );
         lut.createGreyscale();

         // find the initial plane using biggest ellipsoid axes
         EllipsoidAxesOfInertia ellipsoidAxesCalculator;
         core::vector3d eigenValues;
         core::vector3d outCenterOfMassVoxel;
         Matrix ellipsoidAxes = ellipsoidAxesCalculator.extractOrderedAxis( volume, lut, &eigenValues, &outCenterOfMassVoxel );
         core::vector3f outCenterOfMassf = volume.indexToPosition( core::vector3f( outCenterOfMassVoxel[ 0 ],
                                                                                   outCenterOfMassVoxel[ 1 ],
                                                                                   outCenterOfMassVoxel[ 2 ] ) );
         core::vector3d outCenterOfMass( outCenterOfMassf[ 0 ],
                                         outCenterOfMassf[ 1 ],
                                         outCenterOfMassf[ 2 ] );

         // the two higest axis are defining the initial plane. We simply wants to
         // find the "smallest" rotation that transform the plane's normal to the axis (-1, 0, 0)
         // the third axis is always the normal
         core::vector3d normal( ellipsoidAxes( 0, 2 ), 
                                ellipsoidAxes( 1, 2 ),
                                ellipsoidAxes( 2, 2 ) );
         normal /= normal.norm2();  // just in case!

         // we want to find the smallest rotation, so the closest space axis to the normal is chosen for the alignment
         static const core::vector3d axis[] =
         {
            core::vector3d( -1, 0,  0 ),
            core::vector3d( 0, -1,  0 ),
            core::vector3d( 0,  0, -1 ),
         };

         double maxSimilarity = -1;
         core::vector3d expectedNormal;
         for ( size_t n = 0; n < core::getStaticBufferSize( axis ); ++n )
         {
            const double similarity = fabs( axis[ n ].dot( normal ) );
            if ( similarity > maxSimilarity )
            {
               expectedNormal = axis[ n ];
               maxSimilarity = similarity;
            }
         }

         //const core::vector3d expectedNormal( 0, 0, -1 );

         // now just make sure the normal and expected normal are oriented the same way so the angle
         // we find is the smallest one
         const double sign = normal.dot( expectedNormal );
         if ( sign < 0 )
         {
            normal = core::vector3d( -normal[ 0 ],
                                     -normal[ 1 ],
                                     -normal[ 2 ] );
         }

         Matrix rotation = core::identityMatrix<Matrix>( 4 ); // the normal and expected normal are very close, so we can just return identity rotation
         if ( !core::equal<double>( fabs( sign ), 1, eps ) )
         {
            // if the normal is too close to the expected normal, the normal2 of the plan defined by the vectors (normal, expected normal) will
            // be inaccurate. This normal2 represents the direction for P3 which will not biase the rotation estimation
            core::StaticVector<double, 3> normal2 = core::cross( normal, expectedNormal );
            normal2 /= normal2.norm2();

            // now build a point mapping. It is the simplest way to avoid all angle computations...
            std::vector< core::StaticVector<double, 3> > fromPoints;
            std::vector< core::StaticVector<double, 3> > toPoints;

            // P1: define the rotation center
            fromPoints.push_back( core::vector3d( 0, 0, 0 ) );
            toPoints.push_back( core::vector3d( 0, 0, 0 ) );

            // P2: rotation between the two normals
            fromPoints.push_back( core::vector3d( 0, 0, 0 ) + normal );
            toPoints.push_back( core::vector3d( 0, 0, 0 ) + expectedNormal );

            // P3: force the rotation so that there is only one angle to be updated (the pair of points and the previous ones are parrallele)
            // and the transformation is well defined!
            fromPoints.push_back( core::vector3d( 0, 0, 0 ) + normal + normal2 );
            toPoints.push_back( core::vector3d( 0, 0, 0 ) + expectedNormal + normal2 );

            // our initial transformation
            EstimatorTransformSimilarityIsometric estimator;
            rotation = estimator.compute( fromPoints, toPoints );
         }

         // Now, we have the center of mass and the associated transformation
         // the idea is then to have the center of mass at the center of the volume
         // and rotate the volume according to the new reference coordinate
         //   T0 = tfm volume center of mass->origin
         //   R = tfm rotating the volume so that the normal and expected normal are aligned
         //   T = tfm center of mass -> volume center
         // We get the following initialization: T * T0^-1 R T0
         const core::vector3f centerMm = volume.indexToPosition( core::vector3f( static_cast<float>( volume.getSize()[ 0 ] ) / 2,
                                                                                 static_cast<float>( volume.getSize()[ 1 ] ) / 2,
                                                                                 static_cast<float>( volume.getSize()[ 2 ] ) / 2 ) );
         const Matrix t0 = core::createTranslation4x4( core::vector3d( -outCenterOfMass[ 0 ], -outCenterOfMass[ 1 ], -outCenterOfMass[ 2 ] ) ); 
         const Matrix t0inv = core::createTranslation4x4( outCenterOfMass ); 

         const Matrix t = core::createTranslation4x4( core::vector3d( centerMm[ 0 ] - outCenterOfMass[ 0 ],
                                                                      centerMm[ 1 ] - outCenterOfMass[ 1 ],
                                                                      centerMm[ 2 ] - outCenterOfMass[ 2 ] ) ); 
         const Matrix initTfm = ( t * t0inv * rotation * t0 ).inverse();

         {
            std::stringstream ss;
            ss << " LUT:" << lut.getMin() << "-" << lut.getMax() << std::endl
               << " centerOfMass=" << outCenterOfMass << std::endl
               << " ellipsoidAxes=" << ellipsoidAxes << std::endl
               << " eiv=" << eigenValues << std::endl
               << " normalToAlign=" << normal << std::endl
               << " expectedNormal=" << expectedNormal << std::endl
               << " rotationFound=" << rotation << std::endl
               << " fullInitMatrix=" << initTfm;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         return initTfm;
      }

   private:
      /**
       
       */
      template <class T, class Buffer>
      Matrix findSymmetryPlane( const imaging::VolumeSpatial<T, Buffer>& volume )  const
      {
      }
   };
}
}

class TestReflectionPlane
{
public:
   typedef imaging::VolumeSpatial<f32>                                         Volumef;
   typedef imaging::VolumeSpatial<f32>::Matrix                                 Matrix;

   void createSegment( Volumef& volume, const core::vector3f& orig, const core::vector3f& x, const core::vector3f& y, size_t sx, size_t sy, size_t sz )
   {
      core::vector3f zz = core::cross( x, y );
      zz /= zz.norm2() * 2;
      
      const core::vector3f xx = x / ( x.norm2() * 2 );
      const core::vector3f yy = y / ( y.norm2() * 2 );
      for ( size_t n = 0; n < sz; ++n )
      {
         core::vector3f pz = orig + zz * n;
         for ( size_t dx = 0; dx < sx; ++dx )
         {
            core::vector3f pzx = pz + xx * dx;
            for ( size_t dy = 0; dy < sy; ++dy )
            {
               core::vector3f pzxy = pzx + yy * dy;
               volume( (size_t)pzxy[ 0 ],
                       (size_t)pzxy[ 1 ],
                       (size_t)pzxy[ 2 ] ) = 100 + rand() % 10;
            }
         }
      }
   }

   void testEllipsoidAxis1()
   {
      Volumef v;
      const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/pet.mf2", v );
      ensure( loaded, "can't find the volume" );

      v.setOrigin( core::vector3f( 0, 0, 0 ) );

      imaging::LookUpTransformWindowingRGB lut( 100, 8000, 256, 1 );
      lut.createGreyscale();

      algorithm::EllipsoidAxesOfInertia calculator;
      algorithm::EllipsoidAxesOfInertia::Matrix proj = calculator.extractOrderedAxis( v, lut );

      core::Matrix<double> proj4( 4, 4 );
      for ( size_t y = 0; y < 3; ++y )
      {
         for ( size_t x = 0; x < 3; ++x )
         {
            proj4( y, x ) = proj( y, x );
         }
      }
      proj4( 3, 3 ) = 1;

      std::cout << proj4 << std::endl;

      core::vector3f p1( proj( 0, 0 ), proj( 1, 0 ), proj( 2, 0 ) );
      core::vector3f p2( proj( 0, 1 ), proj( 1, 1 ), proj( 2, 1 ) );
      core::vector3f p3( proj( 0, 2 ), proj( 1, 2 ), proj( 2, 2 ) );

      const double tol = 1e-1;
      // compare with the expected axis. We just ensure the direction is the same
      TESTER_ASSERT( core::equal<double>( fabs( p1[ 0 ] ), 0, tol ) );
      TESTER_ASSERT( core::equal<double>( fabs( p1[ 1 ] ), 0, tol ) );
      TESTER_ASSERT( core::equal<double>( fabs( p1[ 2 ] ), 1, tol ) );

      TESTER_ASSERT( core::equal<double>( fabs( p2[ 0 ] ), 1, tol ) );
      TESTER_ASSERT( core::equal<double>( fabs( p2[ 1 ] ), 0, tol ) );
      TESTER_ASSERT( core::equal<double>( fabs( p2[ 2 ] ), 0, tol ) );

      TESTER_ASSERT( core::equal<double>( fabs( p3[ 0 ] ), 0, tol ) );
      TESTER_ASSERT( core::equal<double>( fabs( p3[ 1 ] ), 1, tol ) );
      TESTER_ASSERT( core::equal<double>( fabs( p3[ 2 ] ), 0, tol ) );

      // now rotate the volume, we expect to find the rotated axis
      core::Matrix<double> tfmMat;
      createRotationMatrix4x4FromEuler( core::vector3d( core::PI / 2, 0, 0 ), tfmMat );
      core::Matrix<double> tfmCenter;
      tfmCenter.import( algorithm::TransformationCreatorAffine::computeCentredTransformation( v ) );
      tfmMat = tfmCenter.inverse() * tfmMat * tfmCenter;
      imaging::TransformationAffine tfm( tfmMat );

      Volumef resampledSpace( v.getSize(), v.getPst(), v.getBackgroundValue() );
      imaging::resampleVolumeTrilinear( v, tfm, resampledSpace );
      imaging::saveSimpleFlatFile( "C:/tmp2/vol_rsp.mf2", resampledSpace );

      core::vector3d eiv;
      const algorithm::EllipsoidAxesOfInertia::Matrix proj2 = calculator.extractOrderedAxis( resampledSpace, lut, &eiv );
      std::cout << proj2 << std::endl;
      std::cout << "eiv=" << eiv << std::endl;

      // display the rotated axis, they should be similar
      // however they are not the same and so it is difficult to compare them
      const core::Matrix<double> tfmMatInv = core::get3x3From4x4( tfmMat );
      std::cout << core::mat3Mulv( tfmMatInv, p1 ) << std::endl;
      std::cout << core::mat3Mulv( tfmMatInv, p2 ) << std::endl;
      std::cout << core::mat3Mulv( tfmMatInv, p3 ) << std::endl;

      const core::vector3f p1b = core::mat3Mulv( tfmMatInv, p1 );
      TESTER_ASSERT( core::equal<double>( fabs( p1b[ 0 ] ), 0, tol ) );
      TESTER_ASSERT( core::equal<double>( fabs( p1b[ 1 ] ), 1, tol ) );
      TESTER_ASSERT( core::equal<double>( fabs( p1b[ 2 ] ), 0, tol ) );
   }

   static bool isColinear( const core::vector3f& x1, const core::vector3f& x2 )
   {
      const core::vector3f x1c = x1 / x1.norm2();
      const core::vector3f x2c = x2 / x2.norm2();
      double sign = 1;

      const double tol = 0.05;
      for ( size_t n = 0; n < 3; ++n )
      {
         if ( fabs( x1c[ n ] ) > tol )
            sign = ( ( x1c[ n ] < 0 && x2c[ n ] < 0 ) || ( x1c[ n ] > 0 && x2c[ n ] > 0 ) ) ? 1 : -1;
      }

      for ( size_t n = 0; n < 3; ++n )
      {
         if ( !core::equal<double>( x1[ n ], x2[ n ] * sign, tol ) )
            return false;
      }

      return true;
   }

   void testEllipsoidAxis2()
   {
      // create a random rotation
      for ( size_t n = 0; n < 15; ++n )
      {
         std::cout << "case=" << n << std::endl;
         Volumef v( core::vector3ui( 256, 256, 256 ), core::identityMatrix<Matrix>( 4 ), 0 );

         core::vector3f orig = v.indexToPosition( core::vector3f( v.getSize()[ 0 ] / 2,
                                                                  v.getSize()[ 1 ] / 2,
                                                                  v.getSize()[ 2 ] / 2 ) );
         Matrix rotation;
         core::createRotationMatrix4x4FromEuler( core::vector3f( core::generateUniformDistributionf( - core::PIf, core::PIf ),
                                                                 core::generateUniformDistributionf( - core::PIf, core::PIf ),
                                                                 core::generateUniformDistributionf( - core::PIf, core::PIf ) ),
                                                 rotation );
         std::cout << "rotation to be recovered (unordered axis)=" << rotation << std::endl;

         core::vector3f x( rotation( 0, 0 ),
                           rotation( 1, 0 ),
                           rotation( 2, 0 ) );
         core::vector3f y( rotation( 0, 1 ),
                           rotation( 1, 1 ),
                           rotation( 2, 1 ) );
         createSegment( v, orig, x, y, 40 + rand() % 5, 60 + rand() % 5, 160 + rand() % 5 );
         imaging::saveSimpleFlatFile( "C:/tmp2/vol_tgt.mf2", v );

         algorithm::EllipsoidAxesOfInertia algo;
         imaging::LookUpTransformWindowingRGB lut( 50, 150, 256 );
         lut.createGreyscale();

         core::vector3d centerOfMass;
         core::vector3d eiv;
         algorithm::EllipsoidAxesOfInertia::Matrix axis = algo.extractOrderedAxis( v, lut, &eiv, &centerOfMass );
         std::cout << "rotation recovered=" << axis << std::endl;

         const core::vector3f centerOfMassExpected = imaging::computeBarycentre( v, lut );
         core::vector3d diffCenterOfMass( centerOfMassExpected[ 0 ] - centerOfMass[ 0 ],
                                          centerOfMassExpected[ 1 ] - centerOfMass[ 1 ],
                                          centerOfMassExpected[ 2 ] - centerOfMass[ 2 ] );
         TESTER_ASSERT( diffCenterOfMass.norm2() < 5 );

         // main axis is Z, followed by y, followed by x
         TESTER_ASSERT( core::isCollinear( x, core::vector3f( axis( 0, 2 ),
                                                              axis( 1, 2 ),
                                                              axis( 2, 2 ) ),
                                           1e-1f ) );
         TESTER_ASSERT( core::isCollinear( y, core::vector3f( axis( 0, 1 ),
                                                       axis( 1, 1 ),
                                                       axis( 2, 1 ) ), 1e-1f ) );
         TESTER_ASSERT( core::isCollinear( core::cross( x, y ), core::vector3f( axis( 0, 0 ),
                                                                         axis( 1, 0 ),
                                                                         axis( 2, 0 ) ), 1e-1f ) );


         std::cout << "diffCenterOfMass=" << diffCenterOfMass << std::endl;
         std::cout << "1CenterOfMass=" << centerOfMass << std::endl;
         std::cout << "2CenterOfMass=" << centerOfMassExpected << std::endl;
      }
   }

   void testInitialTransformation()
   {
      for ( size_t n = 0; n < 20; ++n )
      {
         // load volume
         Volumef v;
         const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/brain.mf2", v );
         ensure( loaded, "can't find the volume" );


         imaging::LookUpTransformWindowingRGB lut( 100, 8000, 256, 1 );
         lut.detectRange( v, 0.8 );
         lut.createGreyscale();

         // first disturb the volume
         Volumef vresampled( v.getSize(), v.getPst().clone() );
         const Matrix centerTfm = algorithm::TransformationCreatorAffine::computeCentredTransformation( v );
         const Matrix rotation = core::getRotation4Zf( core::generateUniformDistribution( -0.6, 0.6 ) );
         const Matrix rotationInv = rotation.inverse();
         const Matrix centerTfmInv = centerTfm.inverse();

         imaging::TransformationAffine tfmRot( centerTfmInv * rotation * centerTfm );
         imaging::resampleVolumeTrilinear( v, tfmRot, vresampled );
         imaging::saveSimpleFlatFile( "C:/tmp2/vol_tgt.mf2", vresampled );

         std::cout << "rotI=" << rotation.inverse() << std::endl;
         std::cout << "rot=" << rotation << std::endl;

         // then try to recover the transformation
         algorithm::ReflectionPlaneCalculator calculator;
         core::Matrix<double> tfmInit = calculator.computeInitialTransformation( vresampled );
         std::cout << "tfmInit=" << tfmInit << std::endl;

         // resample the volume according to initial tfm
         imaging::TransformationAffine tfm( tfmInit );
         Volumef resampledSpace( vresampled.getSize(), vresampled.getPst(), vresampled.getBackgroundValue() );
         imaging::resampleVolumeTrilinear( vresampled, tfm, resampledSpace );
         imaging::saveSimpleFlatFile( "C:/tmp2/vol_rsp.mf2", resampledSpace );

         // finally, do it another time, this time, we must be very close to identity
         core::Matrix<double> tfmInit2 = calculator.computeInitialTransformation( resampledSpace );
         std::cout << "tfmInit2=" << tfmInit2 << std::endl;

         // the rotation must be similar to the rotationInv
         const double eps = 0.05;
         const double epsRot = 0.1;
         for ( size_t y = 0; y < 3; ++y )
         {
            for ( size_t x = 0; x < 3; ++x )
            {
               TESTER_ASSERT( core::equal<double>( tfmInit( y, x ), rotationInv( y, x ), epsRot ) );
            }
         }

         // test translation is minimal
         TESTER_ASSERT( core::equal<double>( tfmInit2( 0, 3 ), 0, 3 ) );
         TESTER_ASSERT( core::equal<double>( tfmInit2( 1, 3 ), 0, 3 ) );
         TESTER_ASSERT( core::equal<double>( tfmInit2( 2, 3 ), 0, 3 ) );
         tfmInit2( 0, 3 ) = 0; tfmInit2( 1, 3 ) = 0; tfmInit2( 2, 3 ) = 0;

         // test rotation is very small
         TESTER_ASSERT( core::identityMatrix<core::Matrix<double>>( 4 ).equal( tfmInit2, eps ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestReflectionPlane);
 TESTER_TEST(testEllipsoidAxis1);
 TESTER_TEST(testEllipsoidAxis2);
 TESTER_TEST(testInitialTransformation);
TESTER_TEST_SUITE_END();
#endif