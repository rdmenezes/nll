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
    @brief Given a weighted set of points, compute the ellipsoid fitting the best the data
    */
   class EllipsoidAxesOfIntertia
   {
   public:
      typedef core::Matrix<double>  Matrix;

      /**
       @brief Compute the second moments of a volume transformed by a LUT

       Returns the matrix
          | m_200 m_110 m_101 |
          | m_110 m_020 m_011 |
          | m_101 m_011 m_002 |

          m_pqr = integral( f(x,y,z) (x - xc)^p * (y - yc)^q * (z - zc)^r dxdydz )
          with (xc, yc, zc) the center of mass, (p,q,r) having the values in the above matrix

       @return the moment matrix
       */
      template <class T, class Buffer, class Lut>
      Matrix processSecondMoments( const imaging::Volume<T, Buffer>& volume, const Lut& lut, core::vector3d* outCenterOfMassVoxel = 0 ) const
      {
         if ( lut.getMin() < volume.getBackgroundValue() )
         {
            nllWarning( "processSecondMoments will be biased by the shape of the volume if the lut has a min below background value" );
         }

         const core::vector3f centerOfMassf = imaging::computeBarycentreVoxel( volume, lut );
         const core::vector3d centerOfMass( centerOfMassf[ 0 ], centerOfMassf[ 1 ], centerOfMassf[ 2 ] );
         const core::vector3d size( (double)volume.size()[ 0 ],
                                    (double)volume.size()[ 1 ],
                                    (double)volume.size()[ 2 ] );

         if ( outCenterOfMassVoxel )
         {
            *outCenterOfMassVoxel = centerOfMass;
         }

         double xx = 0;
         double yy = 0;
         double zz = 0;
         double xy = 0;
         double xz = 0;
         double yz = 0;

         // compute the second moments
         double accumRatio = 0;
         const double normCte = lut.getSize() * size[ 0 ] / 2; // to avoid overflow in accum
         for ( double z = 0; z < size[ 2 ]; ++z )
         {
            for ( double y = 0; y < size[ 1 ]; ++y )
            {
               for ( double x = 0; x < size[ 0 ]; ++x )
               {
                  const T value = volume( x, y, z );
                  const double valueTfm = static_cast<double>( lut.transform( volume( x, y, z ) )[ 0 ] );
                  const double ratio = valueTfm / normCte;

                  xx += ratio * core::sqr( x - centerOfMass[ 0 ] );
                  yy += ratio * core::sqr( y - centerOfMass[ 1 ] );
                  zz += ratio * core::sqr( z - centerOfMass[ 2 ] );
                  xy += ratio * ( x - centerOfMass[ 0 ] ) * ( y - centerOfMass[ 1 ] );
                  xz += ratio * ( x - centerOfMass[ 0 ] ) * ( z - centerOfMass[ 2 ] );
                  yz += ratio * ( y - centerOfMass[ 1 ] ) * ( z - centerOfMass[ 2 ] );
                  accumRatio += ratio;
               }
            }
         }

         Matrix moments( 3, 3 );
         moments( 0, 0 ) = xx / accumRatio;
         moments( 1, 1 ) = yy / accumRatio;
         moments( 2, 2 ) = zz / accumRatio;

         moments( 0, 1 ) = xy / accumRatio;
         moments( 1, 0 ) = xy / accumRatio;

         moments( 0, 2 ) = xz / accumRatio;
         moments( 2, 0 ) = xz / accumRatio;

         moments( 1, 2 ) = yz / accumRatio;
         moments( 2, 1 ) = yz / accumRatio;

         return moments;
      }

      /**
       @brief Extract the ellipsoid basis
       @return the ellipsoid basis, 1 basis vector = 1 column
       @param outEigenValues if not null, the eigenvalues will be returned
       */
      Matrix extractOrderedAxis( const Matrix& moments, core::vector3d* outEigenValues = 0 ) const
      {
         core::Buffer1D<double> ev;
         Matrix v;
         Matrix u;
         const bool success = core::svd( moments, u, ev, v );
         ensure( success, "problem in the moment matrix" );
         ensure( v.sizex() == 3 && v.sizey() == 3, "problem with moment matrix: some eigen values are null or too small" );

         if ( outEigenValues )
         {
            (*outEigenValues)[ 0 ] = ev[ 0 ];
            (*outEigenValues)[ 1 ] = ev[ 1 ];
            (*outEigenValues)[ 2 ] = ev[ 2 ];
         }

         return v;
      }

      /**
       @brief Extract the ellipsoid basis
       @return the ellipsoid basis, 1 basis vector = 1 column
       @param outEigenValues if not null, the eigenvalues will be returned
       */
      template <class T, class Buffer, class Lut>
      Matrix extractOrderedAxis( const imaging::Volume<T, Buffer>& volume, const Lut& lut, core::vector3d* outEigenValues = 0, core::vector3d* outCenterOfMassVoxel = 0 ) const
      {
         Matrix moments = processSecondMoments( volume, lut, outCenterOfMassVoxel );
         return extractOrderedAxis( moments, outEigenValues );
      }
   };

   /**
    @brief Find the symmetry plane of a volume
    @see Evaluation of the symmetry plane in 3D MR brain images,
         Alexander V. Tuzikov, Olivier Colliot, Isabelle Bloch
         http://perso.telecom-paristech.fr/~bloch/papers/PRL03_Olivier.pdf
    */
   class ReflectionPlaneCalculator
   {
   public:
      typedef EllipsoidAxesOfIntertia::Matrix   Matrix;

   public:
      template <class T, class Buffer>
      core::GeometryPlane process( const imaging::VolumeSpatial<T, Buffer>& volume )  const
      {
         core::GeometryPlane plane;
         return plane;
      }

      /**
       @brief Find the intial transform that rotates the volumes following the ellipsoid axes and having the center of mass
              centered on the volume
       */
      template <class T, class Buffer>
      Matrix computeInitialTransformation( const imaging::VolumeSpatial<T, Buffer>& volume )  const
      {
         // find initial range
         imaging::LookUpTransformWindowingRGB lut( 100, 8000, 256, 1 );
         lut.createGreyscale();
         lut.detectRange( volume, 0.8, 256 );

         // increase the minimum so that we are not capturing the background values
         const double range = lut.getMax() - lut.getMin();
         lut.reset( lut.getMin() + range * 0.2, lut.getMax(), lut.getSize() );
         lut.createGreyscale();

         // find the initial plane using biggest ellipsoid axes
         EllipsoidAxesOfIntertia ellipsoidAxesCalculator;
         core::vector3d eigenValues;
         core::vector3d outCenterOfMassVoxel;
         Matrix ellipsoidAxes = ellipsoidAxesCalculator.extractOrderedAxis( volume, lut, &eigenValues, &outCenterOfMassVoxel );
         core::vector3f outCenterOfMassf = volume.indexToPosition( core::vector3f( outCenterOfMassVoxel[ 0 ],
                                                                                   outCenterOfMassVoxel[ 1 ],
                                                                                   outCenterOfMassVoxel[ 2 ] ) );
         core::vector3d outCenterOfMass( outCenterOfMassf[ 0 ],
                                         outCenterOfMassf[ 1 ],
                                         outCenterOfMassf[ 2 ] );


         // Now, we have the center of mass and the associate transformation
         // the idea is then to have the center of mass at the center of the volume
         // and rotate the volume according to the new reference coordinate
         //   T0 = tfm volume center of mass->origin
         //   R = tfm rotating the volume so that the ellipsoid axes are matching system {orign, (1, 0, 0), (0, 1, 0), (0, 0, 1)}
         //   T = tfm center of mass -> volume center
         // We get the following initialization: T * T0^-1 R T0
         //Matrix rot = core::identityMatrix<Matrix>( 4 );
         
         std::cout << ellipsoidAxes << std::endl;
         Matrix rot( 4, 4 );

         // rotate by several 90 degrees rotations. We are expecting the Y axis, followed by Z and finally X
         // so rewrite the matrix in [X Y Z] format
         const size_t index[] =
         {
            1, 2, 0
         };

         for ( size_t y = 0; y < 3; ++y )
         {
            for ( size_t x = 0; x < 3; ++x )
            {
               rot( y, index[ x ] ) = ellipsoidAxes( y, x );
            }
         }
         rot( 3, 3 ) = 1;

         // force the orientation to be in the "positive" way
         // so that we are keeping standard orientation (there is no guaranty on the orientation, so choose an arbitrary one)
         core::vector3d sign;
         for ( size_t x = 0; x < 3; ++x )
         {
            sign[ x ] = core::sign( rot( x, x ) );
         }

         for ( size_t y = 0; y < 3; ++y )
         {
            for ( size_t x = 0; x < 3; ++x )
            {
               rot( y, x ) *= sign[ x ];
            }
         }
         
         const core::vector3f centerMm = volume.indexToPosition( core::vector3f( static_cast<float>( volume.getSize()[ 0 ] ) / 2,
                                                                                 static_cast<float>( volume.getSize()[ 1 ] ) / 2,
                                                                                 static_cast<float>( volume.getSize()[ 2 ] ) / 2 ) );

         const Matrix t0 = core::createTranslation4x4( core::vector3d( -outCenterOfMass[ 0 ], -outCenterOfMass[ 1 ], -outCenterOfMass[ 2 ] ) ); 
         const Matrix t0inv = core::createTranslation4x4( outCenterOfMass ); 

         const Matrix t = core::createTranslation4x4( core::vector3d( centerMm[ 0 ] - outCenterOfMass[ 0 ],
                                                                      centerMm[ 1 ] - outCenterOfMass[ 1 ],
                                                                      centerMm[ 2 ] - outCenterOfMass[ 2 ] ) ); 
         return ( t * t0inv * rot.inverse() * t0 ).inverse();
      }
   };
}
}

class TestReflectionPlane
{
public:
   typedef imaging::VolumeSpatial<f32>                                         Volumef;

   void testEllipsoidAxis1()
   {
      Volumef v;
      const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/pet.mf2", v );
      ensure( loaded, "can't find the volume" );

      v.setOrigin( core::vector3f( 0, 0, 0 ) );

      imaging::LookUpTransformWindowingRGB lut( 100, 8000, 256, 1 );
      lut.createGreyscale();

      algorithm::EllipsoidAxesOfIntertia calculator;
      algorithm::EllipsoidAxesOfIntertia::Matrix proj = calculator.extractOrderedAxis( v, lut );

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
      const algorithm::EllipsoidAxesOfIntertia::Matrix proj2 = calculator.extractOrderedAxis( resampledSpace, lut, &eiv );
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

   void testInitialTransformation()
   {
      // load volume
      Volumef v;
      const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/brain5.mf2", v );
      ensure( loaded, "can't find the volume" );

      //v.setOrigin( core::vector3f( 0, 0, 0 ) );

      imaging::LookUpTransformWindowingRGB lut( 100, 8000, 256, 1 );
      lut.createGreyscale();

      // compute initial transform
      algorithm::ReflectionPlaneCalculator calculator;
      core::Matrix<double> tfmInit = calculator.computeInitialTransformation( v );

      // resample the volume according to initial tfm
      imaging::TransformationAffine tfm( tfmInit );
      Volumef resampledSpace( v.getSize(), v.getPst(), v.getBackgroundValue() );
      imaging::resampleVolumeTrilinear( v, tfm, resampledSpace );
      imaging::saveSimpleFlatFile( "C:/tmp2/vol_rsp.mf2", resampledSpace );
      imaging::saveSimpleFlatFile( "C:/tmp2/vol_tgt.mf2", v );

      // finally, redo an initialization, this time we must have exactly identity
      core::Matrix<double> tfmInit2 = calculator.computeInitialTransformation( resampledSpace );
      std::cout << "tfmInit2=" << tfmInit2 << std::endl;

      // resample the volume according to initial tfm
      imaging::TransformationAffine tfm2( tfmInit2 );
      Volumef resampledSpace2( v.getSize(), v.getPst(), v.getBackgroundValue() );
      imaging::resampleVolumeTrilinear( resampledSpace, tfm2, resampledSpace2 );
      imaging::saveSimpleFlatFile( "C:/tmp2/vol_rsp2.mf2", resampledSpace2 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestReflectionPlane);
 //TESTER_TEST(testEllipsoidAxis1);
 TESTER_TEST(testInitialTransformation);
TESTER_TEST_SUITE_END();
#endif