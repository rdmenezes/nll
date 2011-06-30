#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Resample a target volume to an arbitrary source geometry
    @param target the volume that will be resampled
    @param source the volume into wich it will be resampled.
    @param tfm a transformation defined from source to target (easier to see if we transform first the source, and continue as if no transformation...)

    The source must already be allocated.
    */
   template <class T, class Storage, class Interpolator>
   void resampleVolume2( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm )
   {
      typedef VolumeSpatial<T, Storage>   VolumeType;
      typedef core::Matrix<f32>  Matrix;

      if ( !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] ||
           !source.getSize()[ 0 ] || !source.getSize()[ 1 ] || !source.getSize()[ 2 ] )
      {
         throw std::runtime_error( "invalid volume" );
      }

      // compute the transformation target voxel -> source voxel
      Matrix transformation = target.getInvertedPst() * tfm.getAffineMatrix() * source.getPst();

//     Matrix transformation =  source.getInvertedPst() *
//                              tfm.getAffineMatrix() *
//                              target.getPst();
      core::vector3f dx( transformation( 0, 0 ),
                         transformation( 1, 0 ),
                         transformation( 2, 0 ) );
      core::vector3f dy( transformation( 0, 1 ),
                         transformation( 1, 1 ),
                         transformation( 2, 1 ) );
      core::vector3f dz( transformation( 0, 2 ),
                         transformation( 1, 2 ),
                         transformation( 2, 2 ) );

      // we transform the origin (voxel index=(0, 0, 0)) to the correponding index in source
      std::cout << "target=" << std::endl;
      target.getPst().print(std::cout );
      //core::VolumeGeometry geom( tfm.getAffineMatrix() * target.getInvertedPst() );
      core::VolumeGeometry geom( tfm.getAffineMatrix() * target.getInvertedPst() );
      std::cout << "geom=" << std::endl;
      geom.getPst().print( std::cout );
      std::cout << "origin in geom=" << geom.indexToPosition( core::vector3f() );

      Matrix targetOriginTfm = tfm.getAffineMatrix() * target.getPst();
      core::inverse( targetOriginTfm );


      core::vector3f targetOrigin2 = transf4( tfm.getInvertedAffineMatrix() * target.getPst(), core::vector3f( 0, 0, 0 ) );

      Matrix g( 4, 4 );
      for ( ui32 y = 0; y < 3; ++y )
         for ( ui32 x = 0; x < 3; ++x )
            g( y, x ) = (tfm.getAffineMatrix() * target.getPst())(y, x);//transformation( y, x );
      g( 3, 3 ) = 1;
      g( 0, 3 ) = targetOrigin2[ 0 ];
      g( 1, 3 ) = targetOrigin2[ 1 ];
      g( 2, 3 ) = targetOrigin2[ 2 ];

      core::VolumeGeometry geom2( g );
      core::vector3f sourceOrigin = geom2.positionToIndex( source.getOrigin() );
      /*sourceOrigin[ 0 ] = -sourceOrigin[ 0 ];
      sourceOrigin[ 1 ] = -sourceOrigin[ 1 ];
      sourceOrigin[ 2 ] = -sourceOrigin[ 2 ];*/
      
      

      core::vector3f targetOrigin = geom.indexToPosition( core::vector3f() );

      core::vector3f originInTarget = transf4( transformation, core::vector3f( 0, 0, 0 ) );
      core::vector3f originInTargetBAD = sourceOrigin;
      //core::vector3f originInTarget = sourceOrigin; // transf4( transformation, core::vector3f( 0, 0, 0 ) );

      core::vector3f slicePosSrc = originInTarget;
    
      std::cout << "Tfm=" << std::endl;
      transformation.print( std::cout );;

      std::cout << "Orig=" << originInTarget << std::endl;

      std::cout << "dx=" << dx << " dy=" << dy << " dz=" << dz;

      const int sizez = static_cast<int>( source.getSize()[ 2 ] );
      #ifndef NLL_NOT_MULTITHREADED
      # pragma omp parallel for
      #endif
      for ( int z = 0; z < sizez; ++z )
      {
         Interpolator interpolator( target );
         interpolator.startInterpolation();

         typename VolumeType::DirectionalIterator  lineIt = source.getIterator( 0, 0, z );
         core::vector3f linePosSrc = core::vector3f( originInTarget[ 0 ] + z * dz[ 0 ],
                                                     originInTarget[ 1 ] + z * dz[ 1 ],
                                                     originInTarget[ 2 ] + z * dz[ 2 ] );
         for ( ui32 y = 0; y < source.getSize()[ 1 ]; ++y )
         {
            typename VolumeType::DirectionalIterator  voxelIt = lineIt;
            
            NLL_ALIGN_16 float voxelPosSrc[ 4 ] =
            { 
               linePosSrc[ 0 ],
               linePosSrc[ 1 ],
               linePosSrc[ 2 ],
               0
            };

            for ( ui32 x = 0; x < source.getSize()[ 0 ]; ++x )
            {
               *voxelIt = interpolator( voxelPosSrc );

               voxelPosSrc[ 0 ] += dx[ 0 ];
               voxelPosSrc[ 1 ] += dx[ 1 ];
               voxelPosSrc[ 2 ] += dx[ 2 ];
               voxelIt.addx();
            }
            linePosSrc += dy;
            lineIt.addy();
         }
         interpolator.endInterpolation();
      }
   }

   template <class T, class Storage, class Interpolator>
   void resampleVolume2( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source )
   {
      typedef core::Matrix<f32>  Matrix;

      Matrix id = core::identityMatrix<Matrix>( 4 );
      resampleVolume2<T, Storage, Interpolator>( target, source, TransformationAffine( id ) );
   }



   /**
    @ingroup imaging
    @brief Resample a target volume to an arbitrary source geometry. Use a default nearest neighbour interpolation for resampling.

    The source must already be allocated.
    */
   template <class T, class Storage>
   void resampleVolumeNearestNeighbour2( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source )
   {
      resampleVolume2<T, Storage, InterpolatorNearestNeighbour< VolumeSpatial<T, Storage> > >( target, source );
   }

   template <class T, class Storage>
   void resampleVolumeNearestNeighbour2( const VolumeSpatial<T, Storage>& target, VolumeSpatial<T, Storage>& source, const TransformationAffine& tfm )
   {
      resampleVolume2<T, Storage, InterpolatorNearestNeighbour< VolumeSpatial<T, Storage> > >( target, source, tfm );
   }
}
}

class TestVolumeResampling
{
public:
   typedef  imaging::VolumeSpatial<int>  Volume;

   void fillVolume( Volume& v )
   {
      ui32 n = 1;
      for ( ui32 z = 0; z < v.size()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < v.size()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < v.size()[ 0 ]; ++x )
            {
               v( x, y, z ) = n++;
            }
         }
      }
   }

   void print( Volume& v )
   {
      for ( i32 y = v.size()[ 1 ] - 1; y >= 0; --y )
      {
         for ( ui32 x = 0; x < v.size()[ 0 ]; ++x )
         {
            std::cout << v( x, y, 0 ) << " ";
         }
         std::cout << std::endl;
      }
   }

   // basic test checking the volume is correctly resampled, just with different origins
   void testResamplingOriginNoTfm()
   {
      int dx = 1;
      int dy = 2;
      int dz = 0;

      const core::vector3f origingResampled( 10, 15, 20 );
      const core::vector3f origing( origingResampled[ 0 ] + dx,
                                    origingResampled[ 1 ] + dy,
                                    origingResampled[ 2 ] + dz );
      Volume resampled(  core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origingResampled ) );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      fillVolume( vol );
      resampleVolumeNearestNeighbour2( vol, resampled );

      print( resampled );

      // expected: simple translation of the volume
      // sourceOriginComparedToTarget=(-1, -2, 0)
      TESTER_ASSERT( resampled( 1, 2, 0 ) == 1 );
      TESTER_ASSERT( resampled( 2, 2, 0 ) == 2 );
      TESTER_ASSERT( resampled( 3, 2, 0 ) == 3 );

      TESTER_ASSERT( resampled( 1, 3, 0 ) == 6 );
      TESTER_ASSERT( resampled( 2, 3, 0 ) == 7 );
      TESTER_ASSERT( resampled( 3, 3, 0 ) == 8 );
   }

   void testResamplingRotPstVol()
   {
      const core::vector3f origingResampled( -5, -5, 0 );
      const core::vector3f origing( 2, 1, 0 );
      Volume resampled(  core::vector3ui( 10, 10, 10 ), core::createTranslation4x4( origingResampled ) );

      core::Matrix<float> volPst( 4, 4 );
      core::matrix4x4RotationZ( volPst, core::PIf / 2 );
      volPst = core::createTranslation4x4( origing ) * volPst;
      Volume vol( core::vector3ui( 5, 5, 5 ), volPst );



      fillVolume( vol );
      resampleVolumeNearestNeighbour2( vol, resampled );

      print( resampled );

      // expected: rotation of the volume on the left on its origin (ie, due to the PST only)
      // sourceOriginComparedToTarget=(-6, 7, 0)
      TESTER_ASSERT( resampled( 7, 6, 0 ) == 1 );
      TESTER_ASSERT( resampled( 7, 7, 0 ) == 2 );
      TESTER_ASSERT( resampled( 7, 8, 0 ) == 3 );
      TESTER_ASSERT( resampled( 6, 6, 0 ) == 6 );
      TESTER_ASSERT( resampled( 6, 7, 0 ) == 7 );
      TESTER_ASSERT( resampled( 6, 8, 0 ) == 8 );
   }

   void testResamplingRotPstResampled()
   {
      const core::vector3f origingResampled( 4, 2, 0 );
      const core::vector3f origing( 1, 1, 0 );
      
      core::Matrix<float> volPst( 4, 4 );
      core::matrix4x4RotationZ( volPst, core::PIf / 2 );
      volPst = core::createTranslation4x4( origingResampled ) * volPst;
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );
      Volume resampled(  core::vector3ui( 10, 10, 10 ), volPst );

      fillVolume( vol );
      resampleVolumeNearestNeighbour2( vol, resampled );

      print( resampled );

      // expected: rotation of the resampled volume on the left on the point (4,2)
      // x axis is (0 1 0)
      // y axis is (-1 0 0 )
      TESTER_ASSERT( resampled( 0, 0, 0 ) == 9 );
      TESTER_ASSERT( resampled( 1, 0, 0 ) == 14 );
      TESTER_ASSERT( resampled( 2, 0, 0 ) == 19 );
      TESTER_ASSERT( resampled( 0, 1, 0 ) == 8 );
      TESTER_ASSERT( resampled( 1, 1, 0 ) == 13 );
      TESTER_ASSERT( resampled( 2, 1, 0 ) == 18);
   }

   void testResamplingTrans()
   {
      const core::vector3f origingResampled( 10, 15, 0 );
      const core::vector3f origing( 13, 16, 0 );
      const core::vector3f tfmMat( 2, 2, 0 );
      Volume resampled(  core::vector3ui( 10, 10, 10 ), core::createTranslation4x4( origingResampled ) );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      imaging::TransformationAffine tfm( core::createTranslation4x4( tfmMat ) );

      // expected: vol is shifted on by (-2,-2): tfm is defined from source->target,
      // but we are resampling the target, meaning we inverse the tfm to have the target->source
      fillVolume( vol );
      resampleVolumeNearestNeighbour2( vol, resampled, tfm );

      print( resampled );

      TESTER_ASSERT( resampled( 1, 0, 0 ) == 6 );
      TESTER_ASSERT( resampled( 2, 0, 0 ) == 7 );
      TESTER_ASSERT( resampled( 3, 0, 0 ) == 8 );
      TESTER_ASSERT( resampled( 1, 1, 0 ) == 11 );
      TESTER_ASSERT( resampled( 2, 1, 0 ) == 12 );
      TESTER_ASSERT( resampled( 3, 1, 0 ) == 13 );
   }

   void testResamplingRot()
   {
      const core::vector3f origingResampled( -5, -5, 0 );
      const core::vector3f origing( 3, 1, 0 );
      const core::vector3f tfmTrans( 1, 0, 0 );
      Volume resampled(  core::vector3ui( 10, 10, 10 ), core::createTranslation4x4( origingResampled ) );
      Volume vol( core::vector3ui( 5, 5, 5 ), core::createTranslation4x4( origing ) );

      core::Matrix<float> tfmMat( 4, 4 );
      core::matrix4x4RotationZ( tfmMat, -core::PIf / 2 );
      tfmMat = core::createTranslation4x4( tfmTrans ) * tfmMat;

      imaging::TransformationAffine tfm( tfmMat );

      // expected: the vol is rotated on the origin (0, 0, 0) then translated by (-2, -4, 0)
      fillVolume( vol );
      resampleVolumeNearestNeighbour2( vol, resampled, tfm );

      print( resampled );


      TESTER_ASSERT( resampled( 2, 4, 0 ) == 1 );
      TESTER_ASSERT( resampled( 2, 5, 0 ) == 2 );
      TESTER_ASSERT( resampled( 2, 6, 0 ) == 3 );
      TESTER_ASSERT( resampled( 1, 4, 0 ) == 6 );
      TESTER_ASSERT( resampled( 1, 5, 0 ) == 7 );
      TESTER_ASSERT( resampled( 1, 6, 0 ) == 8 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeResampling);
/*TESTER_TEST( testResamplingOriginNoTfm );
TESTER_TEST( testResamplingRotPstVol );
TESTER_TEST( testResamplingRotPstResampled );
TESTER_TEST( testResamplingTrans );*/
TESTER_TEST( testResamplingRot );
TESTER_TEST_SUITE_END();
#endif
