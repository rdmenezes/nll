#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace imaging
{
   /**
    @brief Efficiently iterate on all resampled voxels and compute the corresponding position in the transformed
           target volume

    //
    //
    BEWARE: the input processor will be local to each slice (each slice will have its own processor object),
    this is because when multithreaded we want to have local objects (else huge drop in performance x6)

    => processors must be thread safe! INCLUDING //// operator=  ////
    //
    //

    start() and end() methods will be called on the original processor
    startSlice(), endSlice(), process() will be called on the local processor
    */
   class VolumeTransformationMapperDdf
   {
   public:
      /**
       @brief Map a resampled coordinate system to a target transformed coordinate system
       @param target the <target> volume transformed by inverse of <tfm>
       @param tfm a <source> to <target> affine transformation, consequently, the target volume will be moved by the inverse of <tfm>
       @param resampled the volume to map the coordinate from

       The transformation can be seen as follow:
	    - compute invert(tfm) so we have a target->source transform
	    - compose the invert(tfm) * TargetPst so we have a index->MM transform (2) (i.e., as we don't have the "source space", so we equivalently move the target by inv(affine))
	    - compute the resampled origin in the space (2)
	    - finally compute the vector director using (2) and the resampled PST
       Then for each voxel in resampled space, find its equivalent index in the target volume.

       it will call Processor( const DirectionalIterator& resampledPosition, const float* targetPosition )
       */
      template <class Processor, class T, class Storage>
      void run( Processor& procOrig, const VolumeSpatial<T, Storage>& target, const TransformationDenseDeformableField& tfm, VolumeSpatial<T, Storage>& resampled ) const
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


class TestTransformationMapperDdf3D
{
   typedef core::Matrix<float>            Matrix;
   typedef imaging::VolumeSpatial<float>  Volume;

public:
   void testSimple()
   {
      // volume set up
      core::vector3ui volumeSize( 20, 30, 40 );
      core::vector3f sizeMm( 20, 30, 40 );
      core::vector3f spacing( sizeMm[ 0 ] / volumeSize[ 0 ],
                              sizeMm[ 1 ] / volumeSize[ 1 ],
                              sizeMm[ 2 ] / volumeSize[ 2 ] );
      core::vector3f origin( 10, 5, 0 );


      Matrix pst = core::createTransformationAffine3D( origin, core::vector3f( 0, 0, 0 ), core::vector3f( 0, 0, 0 ), spacing );
      Volume v( volumeSize, pst );

      core::vector3f originResampled( 15, 5, 0 );
      Matrix pstResampled = core::createTransformationAffine3D( origin, core::vector3f( 0, 0, 0 ), core::vector3f( 0, 0, 0 ), spacing );
      Volume resampled( volumeSize, pstResampled );

      // DDF set up
      Matrix tfm = core::createTransformationAffine3D( core::vector3f( 1, 0, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 1, 1, 1 ) );
      core::vector3ui ddfSize( 10, 15, 20 );
      imaging::TransformationDenseDeformableField ddf = imaging::TransformationDenseDeformableField::create( tfm, pst, sizeMm, ddfSize );

      // run
      imaging::VolumeTransformationMapperDdf transformationMapper;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTransformationMapperDdf3D);
TESTER_TEST(testSimple);
TESTER_TEST_SUITE_END();
#endif