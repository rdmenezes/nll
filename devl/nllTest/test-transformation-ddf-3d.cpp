#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Defines a dense Deformable field transformation

    We can see the transformation as follow:
    - start from a point in MM = pa
    - transform <pa> using the affine component = pm
    - transform <pm> in index using the DDF PST = pi
    - interpolate the DDF at <pi> to get the deformable displacement = pd
    - return pm + pd

    Internally, the Ddf stores an affine transformation and a PST for the displacement field.
    The DDF PST is used to position the DDF grid, while the affine transformation is moving this grid

    As an example, an index x in the DDF is transformed by the DDF's PST, returning a point in mm, this point is further transformed by the affine transformation to get
    a final position in MM
    */
   class TransformationDenseDeformableField : public Transformation
   {
   public:
      typedef VolumeSpatial<core::vector3f>   Ddf;
      typedef InterpolatorTriLinearDummy<Ddf> DdfInterpolator;

      /**
       @brief The most basic constructor
       @param ddfPst the affine transformation mapping the DDF index in MM
       */
      template <class T>
      TransformationDenseDeformableField( const core::Matrix<T>& source2target, const core::Matrix<T>& ddfPst, const core::vector3ui& ddfSize )
      {
         core::StaticVector<float, 3> initVal;
         _ddf = Ddf( ddfSize, ddfPst, initVal );
         _source2target.import( source2target );

         _source2targetInv.import( source2target );
         const bool success = core::inverse( _source2targetInv );
         ensure( success, "non affine tfm" );
      }

      /**
       @brief clone the transformation
       */
      virtual Transformation* clone() const
      {
         TransformationDenseDeformableField* ddf = new TransformationDenseDeformableField( _source2target, _ddf.getPst(), _ddf.getSize() );
         return ddf;
      }

      /**
       @brief get the affine part of the transformation
       */
      virtual const Matrix& getAffineMatrix() const
      {
         return _source2target;
      }

      /**
       @brief get the inverse of the affine transformation
       */
      virtual const Matrix& getInvertedAffineMatrix() const
      {
         return _source2targetInv;
      }

      const Ddf& getDdf() const
      {
         return _ddf;
      }

      /**
       @brief transform a point defined in MM, returns the deformable displacement at this position in MM
       */
      virtual core::vector3f transformDeformableOnly( const nll::core::vector3f& p ) const
      {
         const core::vector3f index = core::transf4( _ddf.getInvertedPst(), p );
         DdfInterpolator interpolator( _ddf );
         return interpolator( index.getBuf() );
      }

      /**
       @brief transform a point defined in source geometry in MM to the new target in MM
       */
      virtual core::vector3f transform( const nll::core::vector3f& p ) const
      {
         const core::vector3f paffine = core::transf4( _source2target, p );
         const core::vector3f def = transformDeformableOnly( paffine );
         return core::vector3f( paffine[ 0 ] + def[ 0 ],
                                paffine[ 1 ] + def[ 1 ],
                                paffine[ 2 ] + def[ 2 ] );
      }

      /**
       @brief helper method create a DDF

       The most common use case is to create a DDF mapping a target volume which has an affine source2target transformation associated. Given this, create a DDF
       which will fully map this target volume

       @note internally, we will create a DDF with size + 1 as the trilinear interpolator will not behave correctly at the volume boundary
       */
      static TransformationDenseDeformableField create( const core::Matrix<float>& source2TargetTfm, const core::Matrix<float>& targetPst, const core::vector3f& targetSizeMm, const core::vector3ui ddfSize )
      {
         typedef Ddf::Matrix  Matrix;

         // compute the DDF spacing and corresponding tfm
         const core::vector3f ddfSpacing( targetSizeMm[ 0 ] / ddfSize[ 0 ],
                                          targetSizeMm[ 1 ] / ddfSize[ 1 ],
                                          targetSizeMm[ 2 ] / ddfSize[ 2 ] );
         const Matrix ddfSpacingTfm = core::createScaling4x4( ddfSpacing );

         const Matrix ddfPst = targetPst * ddfSpacingTfm;
         return TransformationDenseDeformableField( source2TargetTfm, ddfPst, ddfSize );
      }

   protected:
      Ddf         _ddf;
      Matrix      _source2target;
      Matrix      _source2targetInv;
   };
}
}

class TestTransformationDdf3d
{
public:
   void testSimpleMapping()
   {
      typedef core::Matrix<float>   Matrix;
      Matrix pst = core::createTransformationAffine3D( core::vector3f( 10, 5, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 1, 1, 1 ) );
      Matrix tfm = core::createTransformationAffine3D( core::vector3f( 1, 0, 0 ), core::vector3f( core::PI / 2, 0, 0 ), core::vector3f( 0, 0, 0 ), core::vector3f( 1, 1, 1 ) );
      core::vector3f sizeMm( 20, 20, 20 );
      core::vector3ui ddfSize( 10, 10, 10 );

      imaging::TransformationDenseDeformableField ddf = imaging::TransformationDenseDeformableField::create( tfm, pst, sizeMm, ddfSize );

      {
         const core::vector3f p = ddf.getDdf().getOrigin();
         const core::vector3f ptfm = ddf.transform(p);
         const core::vector3f ptfmRef = core::transf4( tfm, p );
         TESTER_ASSERT( (ptfmRef-ptfm).norm2() < 1e-4 );    // no DDF = 0, so we expect to be just an affine tfm
      }

      ddf.getAffineMatrix().print( std::cout );
      std::cout << "p=" << ddf.transform( core::vector3f( 10, 5, 0 ) ) << std::endl;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTransformationDdf3d);
TESTER_TEST(testSimpleMapping);
TESTER_TEST_SUITE_END();
#endif