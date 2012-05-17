#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace imaging
{
   class VolumeTransformationMapperTest
   {
   public:
      /**
       @brief Map a resampled coordinate system to a target transformed coordinate system
       @param target the <target> volume transformed by inverse of <tfm>
       @param tfm a <source> to <target> affine transformation, consequently, the target volume will be moved by the inverse of <tfm>
       @param resampled the volume to map the coordinate from

       Basically, for each voxel of the resampled, it finds the corresponding voxel in the transformed target volume.

       To do this efficiently, we compute the mapping of the resampled index to target index. First, the target volume is moved by inv<tfm> (so that we have a
       transformation target->source). Then we find the index corresponding to the resampled origin in the space of the moved target. Finally compose with the
       resampled PST to get the axis+scaling of the total transformation

       it will call Processor( const DirectionalIterator& resampledPosition, const float* targetPosition )
       */
      template <class Processor, class T, class Storage>
      void run( Processor& procOrig, const VolumeSpatial<T, Storage>& target, const TransformationAffine& tfm, VolumeSpatial<T, Storage>& resampled ) const
      {
         typedef VolumeSpatial<T, Storage>   VolumeType;
         typedef core::Matrix<f32>  Matrix;

         if ( !target.getSize()[ 0 ] || !target.getSize()[ 1 ] || !target.getSize()[ 2 ] ||
              !resampled.getSize()[ 0 ] || !resampled.getSize()[ 1 ] || !resampled.getSize()[ 2 ] )
         {
            return;
         }

         // (1) compute the transformation index target->position MM with affine target->source TFM applied
         core::Matrix<float> targetOriginTfm = tfm.getInvertedAffineMatrix() * target.getPst();

         // compute the origin of the resampled in the geometric space (1)
         const bool success = core::inverse( targetOriginTfm );
         ensure( success, "not affine!" );
         const core::vector3f originInTarget = core::transf4( targetOriginTfm, resampled.getOrigin() );

         // finally get the axis direction resampled voxel -> target voxel
         const core::Matrix<float> orientation = targetOriginTfm * resampled.getPst();
         const core::vector3f dx( orientation( 0, 0 ),
                                  orientation( 1, 0 ),
                                  orientation( 2, 0 ) );
         const core::vector3f dy( orientation( 0, 1 ),
                                  orientation( 1, 1 ),
                                  orientation( 2, 1 ) );
         const core::vector3f dz( orientation( 0, 2 ),
                                  orientation( 1, 2 ),
                                  orientation( 2, 2 ) );

         // now fast resampling loop
         const int sizez = static_cast<int>( resampled.getSize()[ 2 ] );
         procOrig.start();

         #if !defined(NLL_NOT_MULTITHREADED)
         # pragma omp parallel for
         #endif
         for ( int z = 0; z < sizez; ++z )
         {
            Processor proc = procOrig;
            proc.startSlice( z );

            typename VolumeType::DirectionalIterator  lineIt = resampled.getIterator( 0, 0, z );
            core::vector3f linePosSrc = core::vector3f( originInTarget[ 0 ] + z * dz[ 0 ],
                                                        originInTarget[ 1 ] + z * dz[ 1 ],
                                                        originInTarget[ 2 ] + z * dz[ 2 ] );
            for ( ui32 y = 0; y < resampled.getSize()[ 1 ]; ++y )
            {
               typename VolumeType::DirectionalIterator  voxelIt = lineIt;
               
               NLL_ALIGN_16 float voxelPosSrc[ 4 ] =
               { 
                  linePosSrc[ 0 ],
                  linePosSrc[ 1 ],
                  linePosSrc[ 2 ],
                  0
               };

               for ( ui32 x = 0; x < resampled.getSize()[ 0 ]; ++x )
               {
                  proc.process( voxelIt, voxelPosSrc );

                  voxelPosSrc[ 0 ] += dx[ 0 ];
                  voxelPosSrc[ 1 ] += dx[ 1 ];
                  voxelPosSrc[ 2 ] += dx[ 2 ];
                  voxelIt.addx();
               }
               linePosSrc += dy;
               lineIt.addy();
            }
            proc.endSlice( z );
         }
         procOrig.end();
      }
   };
}
}

class TestVolumeMapper
{
   typedef core::Matrix<float>            Matrix;
   typedef imaging::VolumeSpatial<float>  Volume;
   typedef imaging::InterpolatorTriLinear<Volume>  Interpolator;

public:
   void testSimpleMapping()
   {
      /*
      Matrix pstTarget = core::createTransformationAffine3D( core::vector3f( 1, 2, 3 ),
                                                             core::vector3f( 0.1, 0, 0 ),
                                                             core::vector3f( 0, 0, 0 ),
                                                             core::vector3f( 1, 1.5, 0.75 ) );

      Matrix pstResampled = core::createTransformationAffine3D( core::vector3f( 1.5, -1, -2 ),
                                                                core::vector3f( 0, 0.2, 0 ),
                                                                core::vector3f( 0, 0, 0 ),
                                                                core::vector3f( 1.5, 1, 0.95 ) );

      Matrix affineTfm = core::createTransformationAffine3D( core::vector3f( 3, -2, -3 ),
                                                                core::vector3f( 0, 0, -0.1 ),
                                                                core::vector3f( 0, 0, 0 ),
                                                                core::vector3f( 1, 1.1, 1.1 ) );
                                                                */
       Matrix pstTarget = core::createTransformationAffine3D(core::vector3f( -10, -20, -30 ),
                                                             core::vector3f( 0.1, 0, 0 ),
                                                             core::vector3f( 0, 0, 0 ),
                                                             core::vector3f( 1, 1.05, 0.95 ) );
       Matrix pstResampled = core::createTransformationAffine3D(core::vector3f( 1, -2, -3 ),
                                                             core::vector3f( 0, 0.1, 0 ),
                                                             core::vector3f( 0, 0, 0 ),
                                                             core::vector3f( 1.1, 1, 0.95 ) );
       Matrix affineTfm = core::createTransformationAffine3D(core::vector3f(-5, -0, -1 ),
                                                             core::vector3f( 0, 0, -0.15 ),
                                                             core::vector3f( 0, 0, 0 ),
                                                             core::vector3f( 1, 1.1, 1.03 ) );
       pstTarget.print( std::cout );


      Volume resampled( core::vector3ui( 256, 512, 300 ), pstResampled );
      Volume resampled2( core::vector3ui( 256, 512, 300 ), pstResampled );
      Volume target( core::vector3ui( 256, 512, 300 ), pstTarget );

      for ( Volume::iterator it = target.begin(); it != target.end(); ++it )
      {
         *it = core::generateUniformDistribution( 100, 500 );
      }

      

      imaging::VolumeTransformationProcessorResampler<Volume, Interpolator> resampler( target, resampled );
      imaging::VolumeTransformationMapperTest mapper;
      mapper.run( resampler, target, affineTfm, resampled );

      /*
      resampled.getStorage().print( std::cout );

      {
         const float p1 = resampled( 0, 0, 0 );
         TESTER_ASSERT( core::equal<float>( p1, 1, 1e-4 ) );
      }

      {
         const float p1 = resampled( 0, 0, 0 );
         TESTER_ASSERT( core::equal<float>( p1, 1, 1e-4 ) );
      }
      */
      
      imaging::VolumeTransformationProcessorResampler<Volume, Interpolator> resampler2( target, resampled2 );
      imaging::VolumeTransformationMapper mapper2;
      mapper2.run( resampler2, target, affineTfm, resampled2 );

      /*
      resampled2.getStorage().print( std::cout );
      */
      for ( ui32 n = 0; n < resampled.getStorage().size(); ++n )
      {
         const float diff = fabs( resampled.getStorage()[ n ] - resampled2.getStorage()[ n ] );
         if (diff > 0.1)
         {
            std::cout << "ERROR=" << diff  << " index=" << n << std::endl;
            //break;
         }
      }
      


   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeMapper);
TESTER_TEST(testSimpleMapping);
TESTER_TEST_SUITE_END();
#endif