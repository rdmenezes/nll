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
      MaximumIntensityProjection( const Volume& volume ) : _volume( volume ), _boundingBox( core::vector3f( 0, 0, 0 ),
                                                                                            core::vector3f( static_cast<float>( volume.size()[ 0 ] ),
                                                                                                            static_cast<float>( volume.size()[ 1 ] ),
                                                                                                            static_cast<float>( volume.size()[ 2 ] ) ) )
      {
      }

      template <class VolumeInterpolator>
      void computeMip( Slice<typename Volume::value_type>& slice, const core::vector3f& direction, float interpolationStepInMinimeter = 0.2 )
      {
         std::cout << "computeMip" << std::endl;
         ui32 n = 0;

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


         core::vector3f intersection1;
         core::vector3f intersection2;

         SliceType::DirectionalIterator itLine = slice.getIterator( 0, 0 );
         SliceType::DirectionalIterator itLineNext = itLine;
         SliceType::DirectionalIterator itLineEnd = slice.getIterator( 0, slice.size()[ 1 ] );
         itLineNext.addy();

         for ( ; itLine != itLineEnd; )
         {
            SliceType::DirectionalIterator itPixels = itLine;
            core::vector3f position = startIndex;

            float max = _volume.getBackgroundValue();
            for ( ; itPixels != itLineNext; ++itPixels )
            {
               if ( _boundingBox.getIntersection( position, dir, intersection1, intersection2 ) )
               {
                  *itPixels = max;
               } else
                  *itPixels = max;
               ++n;
            }

            itLineNext.addy();
            itLine.addy();
         }

         std::cout << "pixels=" << n << std::endl;
      }

   private:
      // non copyable
      MaximumIntensityProjection& operator=( const MaximumIntensityProjection& );

   private:
      const Volume&        _volume;
      core::GeometryBox    _boundingBox;
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
                   nll::core::vector2f( 1, 1 ) );

      nll::core::Timer t1;
      mipCreator.computeMip<Interpolator>( slice, nll::core::vector3f( 0, 0, 1 ) );
      std::cout << slice( 0, 0, 0 ) << std::endl;
      std::cout << "Time=" << t1.getCurrentTime() << std::endl;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMaximumIntensityProjection);
TESTER_TEST(testMip);
TESTER_TEST_SUITE_END();
#endif
