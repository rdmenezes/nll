#include "stdafx.h"
#include <nll/nll.h>

namespace nll
{
namespace imaging
{
   /**
    @Trilinear interpolator of a volume.

    Volume must be of a volume type or derived.
    */
   template <class Volume>
   class InterpolatorTriLinear
   {
   public:
      typedef Volume   VolumeType;

   public:
      /**
       @brief Construct an interpolator for the volume v. 

       v must remain valid until the end of the calls to the interpolator
       */
      InterpolatorTriLinear( const VolumeType& v ) : _volume( v )
      {}

      /**
       @brief (x, y, z) must be an index. It returns background if the point is outside the volume
       */
      double operator()( double x, double y, double z ) const
      {
         if ( !_volume.inside( x, y, z ) )
            _volume.getBackgroundValue();

         const ui32 ix = (ui32)x;
         const ui32 iy = (ui32)y;
         const ui32 iz = (ui32)z;

         const double dx = x - ix;
         const double dy = y - iy;
         const double dz = z - iz;

         const double v000 = _getValue( ix,     iy,     iz );
         const double v001 = _getValue( ix,     iy,     iz + 1 );
         const double v010 = _getValue( ix,     iy + 1, iz );
         const double v100 = _getValue( ix + 1, iy,     iz );
         const double v011 = _getValue( ix,     iy + 1, iz + 1 );
         const double v110 = _getValue( ix + 1, iy + 1, iz );
         const double v101 = _getValue( ix + 1, iy,     iz + 1 );
         const double v111 = _getValue( ix + 1, iy + 1, iz + 1 );

         const double val = v000 * ( 1 - dx ) * ( 1 - dy ) * ( 1 - dz ) +
                            v100 * dx * ( 1 - dy ) * ( 1 - dz ) +
                            v010 * ( 1 - dx ) * dy * ( 1 - dz ) +
                            v001 * ( 1 - dx ) * ( 1 - dy ) * dz +
                            v101 * dx * ( 1 - dy ) * dz +
                            v011 * ( 1 - dx ) * dy * dz +
                            v110 * dx * dy * ( dz - 1 ) +
                            v111 * dx * dy * z;

         return val;
      }

   protected:
      inline double _getValue( ui32 x, ui32 y, ui32 z ) const
      {
         if ( _volume.inside( x, y, z ) )
            return _volume( x, y, z );
         return _volume.getBackgroundValue();
      }

   protected:
      /// non copiable
      InterpolatorTriLinear& operator=( const InterpolatorTriLinear& );

   protected:
      const VolumeType& _volume;
   };

   /**
    @ingroup imaging
    @brief Multiplanar reconstruction of a volume

    Volume must be a spatial volume as we need to know its position and orientation in space

    Extract a slice according to a plane.
    */
   template <class Volume, class Interpolator3D>
   class Mpr
   {
   public:
      typedef Volume          VolumeType; 
      typedef Interpolator3D  Interpolator;
      typedef core::Image<double, core::IndexMapperRowMajorFlat2DColorRGBnMask> Slice;

   public:
      /**
       @brief set the size of the plane to be reconstructed in voxels
       */
      Mpr( const VolumeType& volume, f64 sxInVoxels, f64 syInVoxels ) :
         _volume( volume ), _voxelsx( sxInVoxels ), _voxelsy( syInVoxels )
      {}

      Slice getSlice( const core::vector3d& point, const core::vector3d& normal ) const
      {
         // the slice has a speficied size, it needs to be resampled afterward if necesary
         Slice slice( static_cast<ui32>( _voxelsx ),
                      static_cast<ui32>( _voxelsx )
                      false );

         // align the normal of the MPR with the volume's coordinates system
         // TODO do it!
         core::vector3d n( normal[ 0 ], normal[ 1 ], normal[ 2 ] );

         // normalize in the case of spacing != 1
         n.div( n.norm2() );

         // compute the slopes
         double dx = n[ 0 ];
         double dy = n[ 1 ];
         double dz = n[ 2 ];

         // set up the interpolator
         Interpolator interpolator( _volume );

         // reconstruct the slice
         double px = point[ 0 ];
         double py = point[ 1 ];
         double pz = point[ 2 ];
         for ( f64 y = 0; y < _voxelsy; ++y )
            for ( f64 x = 0; x < _voxelsy; ++x )
            {
               slice( x, y, 0 ) = interpolator( px, py, pz );
               px += dx;
               py += dy;
               pz += dz;
            }
         return slice;
      }

   protected:
      const VolumeType& _volume;
      f64               _voxelsx;
      f64               _voxelsy;
   };
}
}

class TestVolume
{
public:
   /**
    Test basic voxel buffer index
    */
   void testBuffer1()
   {
      typedef nll::imaging::VolumeMemoryBuffer<double>   Buffer;

      Buffer buffer( 5, 10, 15 );
      double s = 0;
      for ( Buffer::iterator it = buffer.begin(); it != buffer.end(); ++it )
         *it = ++s;

      s = 0;
      for ( Buffer::const_iterator it = buffer.begin(); it != buffer.end(); ++it )
         TESTER_ASSERT( nll::core::equal( ++s, *it ) );

      buffer( 1, 2, 3 ) = 5.5;
      TESTER_ASSERT( nll::core::equal( buffer( 1, 2, 3 ), 5.5 ) );

      TESTER_ASSERT( buffer.getSize() == nll::core::vector3ui( 5, 10, 15 ) );

      // test read/write of a buffer
      std::stringstream str;
      buffer.write( str );

      Buffer buffer2;
      buffer2.read( str );

      for ( Buffer::iterator it = buffer.begin(), it2 = buffer2.begin(); it != buffer.end(); ++it, ++it2 )
         TESTER_ASSERT( nll::core::equal( *it, *it2 ) );
   }

   /**
    Test test volume sharing memory
    */
   void testVolume1()
   {
      typedef nll::imaging::Volume<int>   Volume;

      Volume vol;
      vol = Volume( 10, 5, 30 );
      vol( 0, 0, 0 ) = -5;
      TESTER_ASSERT( vol.size() == nll::core::vector3ui( 10, 5, 30 ) );
      TESTER_ASSERT( vol( 0, 0, 0 ) == -5 );

      Volume vol2( 40, 50, 10 );
      vol2( 0, 0, 0 ) = -15;
      TESTER_ASSERT( vol2( 0, 0, 0 ) == -15 );

      vol2 = vol;
      TESTER_ASSERT( vol2( 0, 0, 0 ) == -5 );

      Volume vol3;
      vol3.clone( vol2 );
      TESTER_ASSERT( vol3( 0, 0, 0 ) == -5 );

      vol3( 0, 0, 0 ) = 10;
      TESTER_ASSERT( vol3( 0, 0, 0 ) == 10 );
      TESTER_ASSERT( vol2( 0, 0, 0 ) == -5 );
      TESTER_ASSERT( vol( 0, 0, 0 ) == -5 );
   }

   void testVolumeIterator()
   {
      typedef nll::imaging::Volume<int>   Volume;
      Volume buffer( 5, 10, 15 );
      int s = 0;
      for ( Volume::iterator it = buffer.begin(); it != buffer.end(); ++it )
         *it = ++s;

      s = 0;
      for ( Volume::const_iterator it = buffer.begin(); it != buffer.end(); ++it )
         TESTER_ASSERT( nll::core::equal( ++s, *it ) );
   }

   void testVolumeSpatial1()
   {
      typedef nll::imaging::VolumeSpatial<double>  Vol;

      Vol volume;

      Vol::Matrix pst;
      Vol::Matrix rot3x3 = nll::core::identity<double, Vol::Matrix::IndexMapper>( 3 );

      pst = Vol::createPatientSpaceTransform( rot3x3, nll::core::vector3d( -10, 5, 30 ), nll::core::vector3d( 10, 20, 30 ) );
      volume = Vol( nll::core::vector3ui( 10, 20, 30 ), pst, 1 );
      TESTER_ASSERT( nll::core::equal<double>( pst( 0, 3 ), -10 ) );
      TESTER_ASSERT( nll::core::equal<double>( pst( 1, 3 ), 5 ) );
      TESTER_ASSERT( nll::core::equal<double>( pst( 2, 3 ), 30 ) );

      TESTER_ASSERT( nll::core::equal<double>( pst( 0, 0 ), 10 ) );
      TESTER_ASSERT( nll::core::equal<double>( pst( 1, 1 ), 20 ) );
      TESTER_ASSERT( nll::core::equal<double>( pst( 2, 2 ), 30 ) );

      TESTER_ASSERT( volume.getOrigin() == nll::core::vector3d( -10, 5, 30 ) );
      TESTER_ASSERT( volume.getSpacing() == nll::core::vector3d( 10, 20, 30 ) );

      // test read write
      std::stringstream ss;
      volume.write( ss );

      Vol volume2;
      volume2.read( ss );
      TESTER_ASSERT( volume2.getBackgroundValue() == volume.getBackgroundValue() );
      TESTER_ASSERT( volume2.getOrigin() == volume.getOrigin() );
      TESTER_ASSERT( volume2.getSpacing() == volume.getSpacing() );
      TESTER_ASSERT( volume2.getSize() == volume.getSize() );

      for ( Vol::iterator it = volume.begin(), it2 = volume2.begin(); it != volume.end(); ++it, ++it2 )
         TESTER_ASSERT( *it == *it2 );

      // inside/outside
      TESTER_ASSERT( !volume.inside( -1, 10, 10 ) );
      TESTER_ASSERT( !volume.inside( 50, 10, 10 ) );
      TESTER_ASSERT( volume.inside( 5, 11, 11 ) );
   }

   void testIndexToPos()
   {
      for ( unsigned nn = 0; nn < 100; ++nn )
      {
         nll::core::vector3ui size( 40, 50, 35 );
         typedef nll::imaging::VolumeSpatial<double>  Volume; 

         // define a random rotation matrix around x axis
         // define a random translation vector in [0..99]
         // define a random spacing [2, 0.01]
         double a = static_cast<double>( rand() ) / RAND_MAX * 3.141 / 2;

         nll::core::vector3d spacingT( rand() % 2 / (double)10 + 0.01,
                                       rand() % 2 / (double)10 + 0.01,
                                       rand() % 2 / (double)10 + 0.01 );
         nll::core::vector3d origin( rand() % 100,
                                     rand() % 100,
                                     rand() % 100 );
         nll::core::Matrix<double> rotation( 3, 3 );
         rotation( 0, 0 ) = 1;
         rotation( 1, 1 ) = cos( a );
         rotation( 2, 1 ) = sin( a );
         rotation( 1, 2 ) = -sin( a );
         rotation( 2, 2 ) = cos( a );

         nll::core::Matrix<double> pst = Volume::createPatientSpaceTransform( rotation, origin, spacingT );

         // create the wrapper
         Volume image( size, pst );
         for ( Volume::iterator it = image.begin(); it != image.end(); ++it )
            *it = rand() % 5000;

         nll::core::vector3d spacing = image.getSpacing();
         for ( unsigned n = 0; n < 100; ++n )
         {
            nll::core::vector3d index( rand() % ( size[ 0 ] - 1 ),
                                       rand() % ( size[ 1 ] - 1 ),
                                       rand() % ( size[ 2 ] - 1 ) );

            nll::core::vector3d position = image.indexToPosition( index );
            nll::core::vector3d indexTransf = image.positionToIndex( position );
            TESTER_ASSERT( fabs( index[ 0 ] - indexTransf[ 0 ] ) < 1e-8 );
            TESTER_ASSERT( fabs( index[ 1 ] - indexTransf[ 1 ] ) < 1e-8 );
            TESTER_ASSERT( fabs( index[ 2 ] - indexTransf[ 2 ] ) < 1e-8 );
         }
      }
   }

   void testInterpolator()
   {
      typedef nll::imaging::Volume<double>  Volume;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume> Interpolator;

      Volume volume( 30, 30, 30, -1 );
      Interpolator interpolator( volume );

      TESTER_ASSERT( interpolator( -1, 0, 0 ) == -1 );

      volume( 10, 10, 10 ) = 10;
      volume( 9, 10, 10 ) = 11;
      volume( 11, 10, 10 ) = 17;
      volume( 10, 10, 11 ) = 12;
      volume( 10, 9, 10 ) = 13;
      volume( 10, 11, 10 ) = 14;
      volume( 10, 10, 11 ) = 15;
      volume( 10, 10, 9 ) = 16;
      TESTER_ASSERT( interpolator( 10, 10, 10 ) == 10 );
      TESTER_ASSERT( interpolator( 10.4, 10.4, 10.4 ) == 10 );
      TESTER_ASSERT( interpolator( 9.6, 9.6, 9.6 ) == 10 );
      TESTER_ASSERT( interpolator( 10.5, 9.6, 9.6 ) == 17 );
   }

   void testInterpolatorTriLinear()
   {
      typedef nll::imaging::Volume<double>  Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume> Interpolator;

      Volume volume( 30, 30, 30, -1 );
      Interpolator interpolator( volume );

      volume( 5, 5, 5 ) = 10;
      volume( 6, 5, 5 ) = 11;
      volume( 5, 6, 5 ) = 20;
      volume( 5, 5, 6 ) = 40;

      TESTER_ASSERT( interpolator( 5.5, 5, 5 ) == 10.5 );
      TESTER_ASSERT( interpolator( 5, 5.5, 5 ) == 15 );
      TESTER_ASSERT( interpolator( 5, 5, 5.5 ) == 25 );

      TESTER_ASSERT( interpolator( -10, 0, 0 ) == -1 );
      TESTER_ASSERT( interpolator( 5, 5, 5 ) == 10 );
   }

   
   void testMpr()
   {
      typedef nll::imaging::VolumeSpatial<double>           Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator>       Mpr;

      nll::core::Matrix<double> identity = nll::core::identity<double, nll::core::Matrix<double>::IndexMapper>( 4 );
      Volume volume( nll::core::vector3ui( 30, 30, 30 ),
                     identity );

      volume( 5, 5, 5 ) = 10;
      volume( 6, 5, 5 ) = 11;
      volume( 5, 6, 5 ) = 20;
      volume( 5, 5, 6 ) = 40;

   }
};

//#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolume);
/*
TESTER_TEST(testBuffer1);
TESTER_TEST(testVolume1);
TESTER_TEST(testVolumeIterator);
TESTER_TEST(testVolumeSpatial1);
TESTER_TEST(testIndexToPos);
TESTER_TEST(testInterpolator);
*/
TESTER_TEST(testInterpolatorTriLinear);
TESTER_TEST_SUITE_END();
//#endif