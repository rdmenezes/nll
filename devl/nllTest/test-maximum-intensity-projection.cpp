#include "stdafx.h"
#include <nll/nll.h>

namespace nll
{
namespace imaging
{
   /**
    @brief Compute the maximum intensity projection of a volume
    @note volume must be a type of spatial volume
    */
   template <class Volume>
   class MaximumIntensityProjection
   {
   public:
      MaximumIntensityProjection( const Volume& volume ) : _volume( volume )
      {
      }

      template <class VolumeInterpolator>
      void computeMip( Slice<typename Volume::value_type>& slice, const core::vector3f& direction, float interpolationStepInMinimeter = 0.2 )
      {
         typedef Slice<typename Volume::value_type>   SliceType;

         ensure( slice.size()[ 2 ] == 1, "only single valued slice is handled" );
         ensure( slice.size()[ 0 ] && slice.size()[ 1 ], "slice must not be empty" );
         if ( !_volume.getSize()[ 0 ] || !_volume.getSize()[ 1 ] || !_volume.getSize()[ 2 ] )
         {
            for ( SliceType::iterator it = slice.begin(); it != slice.end(); ++it )
               *it = _volume.getBackgroundValue();
         }


         // normalized vector director
         core::vector3f dirInStandardSpace = direction;
         dirInStandardSpace /= ( static_cast<f32>( dirInStandardSpace.norm2() ) * interpolationStepInMinimeter );

         // get the starting point of the slice in volume coordinate
         core::vector3f start = _volume.positionToIndex( slice.sliceToWorldCoordinate( core::vector2f( -static_cast<float>( slice.size()[ 0 ] ) / 2,
                                                                                                       -static_cast<float>( slice.size()[ 1 ] ) / 2 ) ) );

         // translate to volume coordinates
         const core::vector3f startIndex = _volume.positionToIndex( start );
         const core::vector3f dir = _volume.positionToIndex( start + dirInStandardSpace ) - startIndex;
         const core::vector3f SliceOringIndex = _volume.positionToIndex( slice.sliceToWorldCoordinate( core::vector2f( 0, 0 ) ) );
         const core::vector3f dx = _volume.positionToIndex( slice.sliceToWorldCoordinate( core::vector2f( 1, 0 ) ) ) - SliceOringIndex;
         const core::vector3f dy = _volume.positionToIndex( slice.sliceToWorldCoordinate( core::vector2f( 1, 0 ) ) ) - SliceOringIndex;


         SliceType::DirectionalIterator itLine = slice.getIterator( 0, 0 );
         SliceType::DirectionalIterator itLineNext = itLine.addy();
         for ( ; itLine != itLineNext; )
         {
            SliceType::DirectionalIterator itPixels = itLine;
            itLineNext.addy();
            itLine.addy();

            float max = _volume.getBackgroundValue();
            for ( ; itPixels != itLine; ++itPixels )
            {
               
               *itPixels = max;
            }
         }
      }

   private:
      // non copyable
      MaximumIntensityProjection& operator=( const MaximumIntensityProjection& );

   private:
      const Volume&  _volume;
   };
}
}

class TestMaximumIntensityProjection
{
public:
   void testMip()
   {
      typedef nll::imaging::VolumeSpatial<float>            Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::Slice<Volume::value_type>       Slice;

      const std::string volname = NLL_TEST_PATH "data/medical/pet-NAC.mf2";
      Volume volume;
      bool loaded = nll::imaging::loadSimpleFlatFile( volname, volume );
      TESTER_ASSERT( loaded );

      nll::imaging::MaximumIntensityProjection<Volume>   mipCreator( volume );

      Slice slice( nll::core::vector3ui( 256, 256, 1 ),
                   nll::core::vector3f( 1, 0, 0 ), 
                   nll::core::vector3f( 0, 1, 0 ),
                   nll::core::vector3f( 0, 0, 0 ),
                   nll::core::vector2f( 0, 0 ) );

      mipCreator.computeMip<Interpolator>( slice, nll::core::vector3f( 0, 0, 1 ) );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMaximumIntensityProjection);
TESTER_TEST(testMip);
TESTER_TEST_SUITE_END();
#endif
