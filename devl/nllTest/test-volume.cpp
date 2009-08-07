#include "stdafx.h"
#include <nll/nll.h>

namespace nll
{
namespace imaging
{
   namespace impl
   {
      /**
       @brief apply a rotation given a transformation (rotation+scale only)
              The matrix must be a 4x4 transformation matrix defined by the volume.
              
              Compute Mv using only the rotational part of M.
       */
      template <class T, class Mapper>
      core::vector3d mul3Rot( const core::Matrix<T, Mapper>& m, const core::vector3d& v )
      {
         assert( m.sizex() == 4 && m.sizey() == 4 );
         return core::vector3d( v[ 0 ] * m( 0, 0 ) + v[ 1 ] * m( 0, 1 ) + v[ 2 ] * m( 0, 2 ),
                                v[ 0 ] * m( 1, 0 ) + v[ 1 ] * m( 1, 1 ) + v[ 2 ] * m( 1, 2 ),
                                v[ 0 ] * m( 2, 0 ) + v[ 1 ] * m( 2, 1 ) + v[ 2 ] * m( 2, 2 ) );
      }
   }


   template <class T, class Buf>
   bool loadSimpleFlatFile( const std::string& filename, VolumeSpatial<T, Buf>& vol )
   {
      typedef VolumeSpatial<T, Buf> Volume;

      // open the file
      std::ifstream file( filename.c_str(), std::ios::binary | std::ios::in );
      if ( !file.good() )
      {
         return false;
      }

      // First of all, work out if we've got the new format that contains more information or not
      unsigned int firstWord = 0;
      file.read( (char*)&firstWord, sizeof( unsigned int ) );
      
      // If new format
      unsigned int width  = 0;
      unsigned int height = 0;
      unsigned int depth  = 0;
      unsigned int dataType = 0;
      if ( firstWord == 0 )
      {
         // Get the version number
         unsigned int formatVersionNumber = 0;
         file.read( (char*)&formatVersionNumber, sizeof( unsigned int ) );
         if ( formatVersionNumber >= 2 )
         {
            // Read datatype flag
            file.read( (char*)&dataType, sizeof( unsigned int ) );
         }

         // Dimensions
         file.read( (char*)&width, sizeof( unsigned int ) );
         file.read( (char*)&height, sizeof( unsigned int ) );
         file.read( (char*)&depth, sizeof( unsigned int ) );
      }
      else
      {
         // Old format - we've actually read the width...
         width = firstWord;
         file.read( (char*)&height, sizeof( unsigned int ) );
         file.read( (char*)&depth, sizeof( unsigned int ) );
      }

      // Spacing
      double colSp = 0;
      double rowSp = 0;
      double sliceSp = 0;
      file.read( (char*)&colSp, sizeof( double ) );
      file.read( (char*)&rowSp, sizeof( double ) );
      file.read( (char*)&sliceSp, sizeof( double ) );

      // Origin
      double originX = 0;
      double originY = 0;
      double originZ = 0;
      file.read( (char*)&originX, sizeof( double ) );
      file.read( (char*)&originY, sizeof( double ) );
      file.read( (char*)&originZ, sizeof( double ) );

      // only with RSI type is handled
      ensure( dataType == 1, "file format not handled" );

      // Create an RSI to maximise dynamic range on each slice
      std::vector< std::pair<double, double> > rsi( depth );

      core::Matrix<double> id( 3, 3 );
      for ( ui32 n = 0; n < 3; ++n )
         id( n, n ) = 1;
      core::Matrix<double> pst = Volume::createPatientSpaceTransform( id, core::vector3d( originX, originY, originZ ), core::vector3d( colSp, rowSp, sliceSp ) );

      for ( unsigned int k = 0; k < depth; ++k )
      {
         double slope = 0;
         double intercept = 0;
         file.read( (char*)&slope, sizeof( double ) );
         file.read( (char*)&intercept, sizeof( double ) );
         rsi[k] = std::make_pair( slope, intercept );
      }

      Volume output( core::vector3ui( width,
                                      height,
                                      depth ),
                     pst );

      for ( unsigned int k = 0; k < depth; ++k )
      {
         for ( unsigned int j = 0; j < height; ++j )
         {
            for ( unsigned int i = 0; i < width; ++i )
            {
               ensure( !file.eof(), "unexpected eof" );
               unsigned short value = 0;
               file.read( (char*)&value, sizeof( unsigned short ) );
               output( i, j, k ) = static_cast<double>( value ) * rsi[ k ].first + rsi[ k ].second;
              // std::cout << "val=" << output( i, j, k ) << std::endl;
            }
         }
      }

      vol = output;
      return true;
   }

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

      /**
       @param point a point in mm
       @param ax x-axis of the plane
       @param ay y-axis of the plane
       */
      Slice getSlice( const core::vector3d& point, const core::vector3d& ax, const core::vector3d& ay ) const
      {
         // the slice has a speficied size, it needs to be resampled afterward if necesary
         Slice slice( static_cast<ui32>( core::round( _voxelsx ) ),
                      static_cast<ui32>( core::round( _voxelsy ) ),
                      1,
                      false );

         // compute the slopes
         core::vector3d dx = impl::mul3Rot( _volume.getInversedPst(), ax );
         dx.div( dx.norm2() );
         core::vector3d dy = impl::mul3Rot( _volume.getInversedPst(), ay );
         dy.div( dy.norm2() );

         // set up the interpolator
         Interpolator interpolator( _volume );

         // transform the point to voxel
         core::vector3d pointVoxel = _volume.positionToIndex( point );

         // reconstruct the slice
         double startx = pointVoxel[ 0 ];
         double starty = pointVoxel[ 1 ];
         double startz = pointVoxel[ 2 ];
         for ( ui32 y = 0; y < _voxelsy; ++y )
         {
            double px = startx;
            double py = starty;
            double pz = startz;
            for ( ui32 x = 0; x < _voxelsx; ++x )
            {
               //std::cout << "check=" << px << " " << py << " " << pz << " val=" << interpolator( px, py, pz ) << std::endl;
               slice( x, y, 0 ) = interpolator( px, py, pz );
               px += dx[ 0 ];
               py += dx[ 1 ];
               pz += dx[ 2 ];
            }

            startx += dy[ 0 ];
            starty += dy[ 1 ];
            startz += dy[ 2 ];
         }
         return slice;
      }

   protected:
      Mpr& operator=( const Mpr& );

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
      // PETtest-NAC.mf2
      //
      const std::string volname = "N:/MCL/Mirada Test Data Library (MTDL)/CardiacRegistration/Thresholding/PETtest-CT.mf2";
      typedef nll::imaging::VolumeSpatial<double>           Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator>       Mpr;

      Volume volume;
      nll::imaging::loadSimpleFlatFile( volname, volume );

      std::cout << "loaded" << std::endl;
      Mpr mpr( volume, 512, 512 );
      Mpr::Slice slice = mpr.getSlice( nll::core::vector3d( -250, -250, 42 ),
                                       nll::core::vector3d( 1, 0, 0 ),
                                       nll::core::vector3d( 0, 1, 0 ) );

      nll::core::Image<nll::i8> bmp( slice.sizex(), slice.sizey(), 1 );
      for ( unsigned y = 0; y < bmp.sizex(); ++y )
         for ( unsigned x = 0; x < bmp.sizex(); ++x )
            bmp( x, y, 0 ) = (nll::i8)NLL_BOUND( ( (double)slice( x, y, 0 ) + 20000 ) / 100, 0, 255 );
      nll::core::extend( bmp, 3 );
      nll::core::writeBmp( bmp, "c:/temp/a.bmp" );
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
TESTER_TEST(testInterpolatorTriLinear);
*/
TESTER_TEST(testMpr);
TESTER_TEST_SUITE_END();
//#endif