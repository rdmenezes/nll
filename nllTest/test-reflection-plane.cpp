#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"
#include "utils.h"

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
      Matrix processSecondMoments( const imaging::VolumeSpatial<T, Buffer>& volume, const Lut& lut ) const
      {
         const core::vector3f centerOfMassf = imaging::computeBarycentre( volume, lut );
         const core::vector3d centerOfMass( centerOfMassf[ 0 ], centerOfMassf[ 1 ], centerOfMassf[ 2 ] );
         const core::vector3d size( (double)volume.getSize()[ 0 ],
                                    (double)volume.getSize()[ 1 ],
                                    (double)volume.getSize()[ 2 ] );

         double xx = 0;
         double yy = 0;
         double zz = 0;
         double xy = 0;
         double xz = 0;
         double yz = 0;

         // compute the second moments
         double accumRatio = 0;
         for ( double z = 0; z < size[ 0 ]; ++z )
         {
            for ( double y = 0; y < size[ 1 ]; ++y )
            {
               for ( double x = 0; x < size[ 2 ]; ++x )
               {
                  const T value = volume( x, y, z );
                  const double ratio = lut.transform( volume( x, y, z ) ) / lut.size();   // to avoid overflow

                  xx += ratio * core::sqr( x - centerOfMass[ 0 ] );
                  yy += ratio * core::sqr( y - centerOfMass[ 1 ] );
                  zz += ratio * core::sqr( z - centerOfMass[ 2 ] );
                  xy += ratio * ( x - centerOfMass[ 0 ] ) * ( y - centerOfMass[ 1 ] );
                  xz += ratio * ( x - centerOfMass[ 0 ] ) * ( z - centerOfMass[ 2 ] );
                  yz += ratio * ( y - centerOfMass[ 1 ] ) * ( z - centerOfMass[ 2 ] );
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
      }

      /**
       @brief Extract the ellipsoid basis
       @return the ellipsoid basis, 1 basis vector = 1 column
       @param outEigenValues if not null, the eigenvalues will be returned
       */
      Matrix extractOrderedAxis( const Matrix& moments, core::vector3d* outEigenValues ) const
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
      template <class T, class Buffer>
      core::GeometryPlane process( const imaging::VolumeSpatial<T, Buffer>& volume )  const
      {
      }
   };
}
}

class TestReflectionPlane
{
public:
   void testBasic()
   {
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestReflectionPlane);
 TESTER_TEST(testBasic);
TESTER_TEST_SUITE_END();
#endif