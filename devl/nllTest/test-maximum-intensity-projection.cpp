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
      typedef typename core::If<typename Volume::value_type, float, core::IsFloatingType<typename Volume::value_type>::value >::type value_type;

   public:
      MaximumIntensityProjection( const Volume& volume ) : _volume( volume ), _boundingBox( core::vector3f( 0, 0, 0 ),
                                                                                            core::vector3f( static_cast<float>( volume.size()[ 0 ] ),
                                                                                                            static_cast<float>( volume.size()[ 1 ] ),
                                                                                                            static_cast<float>( volume.size()[ 2 ] ) ) )
      {
      }

      /**
       @brief Compute a maximum intensity projection on the slice.
       @param slice defines the attributs of the surface we want to project
       @param direction determines the projection direction
       */
      template <class VolumeInterpolator>
      void computeMip( Slice<value_type>& slice, const core::vector3f& direction )
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
         float interpolationStepInMinimeter = std::min( std::min( _volume.getSpacing()[ 0 ], _volume.getSpacing()[ 1 ] ), _volume.getSpacing()[ 2 ] ) * 2;
         dirInStandardSpace /= static_cast<f32>( dirInStandardSpace.norm2() );
         dirInStandardSpace *= interpolationStepInMinimeter;

         // get the starting point of the slice in volume coordinate
         core::vector3f bottomLeftSliceInWorld = slice.sliceToWorldCoordinate( core::vector2f( -static_cast<float>( slice.size()[ 0 ] ) / 2,
                                                                                               -static_cast<float>( slice.size()[ 1 ] ) / 2 ) );
         // translate to volume coordinates
         const core::vector3f startIndex = _volume.positionToIndex( bottomLeftSliceInWorld );
         const core::vector3f dir = _volume.positionToIndex( dirInStandardSpace ) -  _volume.positionToIndex( core::vector3f( 0, 0, 0 ) );
         const core::vector3f SliceOringIndex = _volume.positionToIndex( slice.sliceToWorldCoordinate( core::vector2f( 0, 0 ) ) );
         const core::vector3f dx = _volume.positionToIndex( slice.sliceToWorldCoordinate( core::vector2f( 1, 0 ) ) ) - SliceOringIndex;
         const core::vector3f dy = _volume.positionToIndex( slice.sliceToWorldCoordinate( core::vector2f( 0, 1 ) ) ) - SliceOringIndex;


         core::vector3f intersection1;
         core::vector3f intersection2;
         core::vector3f startLine = startIndex;

         SliceType::DirectionalIterator itLine = slice.getIterator( 0, 0 );
         SliceType::DirectionalIterator itLineNext = itLine;
         SliceType::DirectionalIterator itLineEnd = slice.getIterator( 0, slice.size()[ 1 ] );
         itLineNext.addy();

         VolumeInterpolator interpolator( _volume );
         interpolator.startInterpolation();
         const float background = _volume.getBackgroundValue();
         const float dirNorm = static_cast<float>( dir.norm2() );
         const core::vector3f dirOppose = core::vector3f( -dir[ 0 ], -dir[ 1 ], -dir[ 2 ] );
         for ( ; itLine != itLineEnd; )
         {
            SliceType::DirectionalIterator itPixels = itLine;
            core::vector3f position = startLine;
            for ( ; itPixels != itLineNext; ++itPixels )
            {
               if ( _boundingBox.getIntersection( position, dir, intersection1, intersection2 ) )
               {
                  
                  float s1 = core::sqr( position[ 0 ] - intersection1[ 0 ] ) +
                             core::sqr( position[ 1 ] - intersection1[ 1 ] ) +
                             core::sqr( position[ 2 ] - intersection1[ 2 ] );
                  float s2 = core::sqr( position[ 0 ] - intersection2[ 0 ] ) +
                             core::sqr( position[ 1 ] - intersection2[ 1 ] ) +
                             core::sqr( position[ 2 ] - intersection2[ 2 ] );
                  float d = sqrt( fabs( s2 - s1 ) ) / dirNorm;
                  *itPixels = _getMaxValue( intersection1, ( s1 < s2 ) ? dirOppose : dirOppose, static_cast<ui32>( std::ceil( d ) ), interpolator );
                  // TODO DIRECTION *itPixels = _getMaxValue( intersection1, ( s1 < s2 ) ? dir : dirOppose, static_cast<ui32>( std::ceil( d ) ), interpolator );
               } else {
                  *itPixels = background;
               }

               position += dx;
            }

            itLineNext.addy();
            itLine.addy();
            startLine += dy;
         }
         interpolator.endInterpolation();
      }
   
   /**
    @brief Creates a slice from angle
    */
   template <class VolumeInterpolator>
   Slice<value_type> getAutoOrientedMip( float anglexRadian, ui32 sizex, ui32 sizey )
   {
      const core::vector3f centerPosition = _volume.indexToPosition( core::vector3f( static_cast<float>( _volume.getSize()[ 0 ] ) / 2, -1e5, static_cast<float>( _volume.getSize()[ 2 ] ) / 2 ) );
      //const core::vector3f centerPosition = _volume.indexToPosition( core::vector3f( static_cast<float>( _volume.getSize()[ 0 ] ) / 2, 1e5, static_cast<float>( _volume.getSize()[ 2 ] ) / 2 ) );
      const core::vector3f centerVolume = _volume.indexToPosition( core::vector3f( static_cast<float>( _volume.getSize()[ 0 ] ) / 2,
                                                                                   static_cast<float>( _volume.getSize()[ 1 ] ) / 2,
                                                                                   static_cast<float>( _volume.getSize()[ 2 ] ) / 2 ) );

      core::Matrix<float> tfm;
      core::matrix4x4RotationZ( tfm, anglexRadian );
      tfm = core::mul( _volume.getInversedPst(), tfm );

      // we need to remove the scaling...
      for ( ui32 t = 0; t < 4; ++t )
      {
         float dd = sqrt( core::sqr( tfm( 0, t ) ) + core::sqr( tfm( 1, t ) ) + core::sqr( tfm( 2, t ) ) );
         tfm( 0, t ) /= dd;
         tfm( 1, t ) /= dd;
         tfm( 2, t ) /= dd;
      }
      

      const core::vector3f centerPositionRotated = core::mul4Rot( tfm, centerPosition );
      const core::vector3f zero = _volume.indexToPosition( core::vector3f( 0, 0, 0 ) );
      const core::vector3f dx = _volume.indexToPosition( core::vector3f( 1, 0, 0 ) ) - zero;
      const core::vector3f dy = _volume.indexToPosition( core::vector3f( 0, 0, 1 ) ) - zero;

      const core::vector3f dxRotated = core::mul4Rot( tfm, dx );
      const core::vector3f dyRotated = core::mul4Rot( tfm, dy );

      Slice<value_type> slice( core::vector3ui( sizex, sizey, 1 ),
                               dxRotated,
                               dyRotated,
                               centerPositionRotated,
                               core::vector2f( static_cast<float>( dxRotated.norm2() ), static_cast<float>( dyRotated.norm2() ) ) );
      computeMip<VolumeInterpolator>( slice, core::cross( dxRotated, dyRotated ) );
      return slice;
   }

   private:
      // non copyable
      MaximumIntensityProjection& operator=( const MaximumIntensityProjection& );

   private:
      template <class VolumeInterpolator>
      value_type _getMaxValue( const core::vector3f& start, const core::vector3f& dir, ui32 nbSteps, const VolumeInterpolator& interpolator )
      {
         NLL_ALIGN_16 float pos[ 4 ] =
         {
            start[ 0 ], start[ 1 ], start[ 2 ], 0
         };

         value_type max = _volume.getBackgroundValue();
         for ( ui32 n = 0; n < nbSteps; ++n )
         {
            value_type val = interpolator( pos );
            max = std::max( max, val );

            pos[ 0 ] += dir[ 0 ];
            pos[ 1 ] += dir[ 1 ];
            pos[ 2 ] += dir[ 2 ];
         }

         return max;
      }

   private:
      const Volume&        _volume;
      core::GeometryBox    _boundingBox;
   };
}
}

class TestMaximumIntensityProjection
{
public:
   void testAutoMip()
   {
      typedef nll::imaging::VolumeSpatial<float>            Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::Slice<Volume::value_type>       Slice;

      const std::string volname = NLL_TEST_PATH "data/medical/pet-NAC.mf2";
      Volume volume;
      bool loaded = nll::imaging::loadSimpleFlatFile( volname, volume );
      TESTER_ASSERT( loaded );


      std::cout << "automip" << std::endl;
      nll::imaging::MaximumIntensityProjection<Volume>   mipCreator( volume );

      int n = 0;
      for ( float f = 0; f < nll::core::PI; f += nll::core::PI / 50, ++n )
      {
         std::cout << "slice" << std::endl;
         //Slice slice = mipCreator.getAutoOrientedMip<Interpolator>( nll::core::PI/2, 128, 81 );
         Slice slice = mipCreator.getAutoOrientedMip<Interpolator>( f, 128, 81 );

         nll::imaging::LookUpTransformWindowingRGB lut( -1000, 10000, 256, 3 );
         lut.createGreyscale();
         nll::core::Image<nll::ui8> i( slice.size()[ 0 ], slice.size()[ 1 ], 3 );
         for ( unsigned y = 0; y < i.sizey(); ++y )
         {
            for ( unsigned x = 0; x < i.sizex(); ++x )
            {
               const nll::f32* col = lut.transform( slice( x, y, 0 ) );
               i( x, y, 0 ) = (nll::ui8)col[ 0 ];
               i( x, y, 1 ) = (nll::ui8)col[ 1 ];
               i( x, y, 2 ) = (nll::ui8)col[ 2 ];
            }
         }
         nll::core::writeBmp( i, NLL_TEST_PATH "data/mip1-256-auto" + nll::core::val2str(n) + ".bmp" );
      }
   }

   void testMip()
   {
      typedef nll::imaging::VolumeSpatial<float>            Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::Slice<Volume::value_type>       Slice;

      
      const std::string volname = NLL_TEST_PATH "data/medical/pet-NAC.mf2";
      Volume volume;
      bool loaded = nll::imaging::loadSimpleFlatFile( volname, volume );
      TESTER_ASSERT( loaded );

      std::cout << "size=" << volume.getSize()[ 0 ] << std::endl;
      std::cout << "size=" << volume.getSize()[ 1 ] << std::endl;
      std::cout << "size=" << volume.getSize()[ 2 ] << std::endl;
      

      Volume::Matrix id = nll::core::identityMatrix<Volume::Matrix>( 4 );
      //Volume volume( nll::core::vector3ui( 256, 256, 256 ), id );

      nll::imaging::MaximumIntensityProjection<Volume>   mipCreator( volume );

      unsigned size = 4;
      Slice slice( nll::core::vector3ui( 128 * size, 128 * size, 1 ),
                   nll::core::vector3f( 1, 0, 0 ), 
                   nll::core::vector3f( 0, 1, 0 ),
                   nll::core::vector3f( -297-155, -297-155, -650 ),
                   nll::core::vector2f( 2.67f/size, 2.67f/size ) );

      nll::core::Timer t1;
      mipCreator.computeMip<Interpolator>( slice, nll::core::vector3f( 0, 0, 1 ) );
      std::cout << "Time MIP 256*256=" << t1.getCurrentTime() << std::endl;

      nll::imaging::LookUpTransformWindowingRGB lut( -1000, 10000, 256, 3 );
      lut.createGreyscale();
      nll::core::Image<nll::ui8> i( slice.size()[ 0 ], slice.size()[ 1 ], 3 );
      for ( unsigned y = 0; y < i.sizey(); ++y )
      {
         for ( unsigned x = 0; x < i.sizex(); ++x )
         {
            const nll::f32* col = lut.transform( slice( x, y, 0 ) );
            i( x, y, 0 ) = (nll::ui8)col[ 0 ];
            i( x, y, 1 ) = (nll::ui8)col[ 1 ];
            i( x, y, 2 ) = (nll::ui8)col[ 2 ];
         }
      }
      nll::core::writeBmp( i, NLL_TEST_PATH "data/mip1-256.bmp" );
      std::cout << slice( 0, 0, 0 ) << std::endl;
   }

   void testMipBig()
   {
      typedef nll::imaging::VolumeSpatial<float>            Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::Slice<Volume::value_type>       Slice;


      Volume::Matrix id = nll::core::identityMatrix<Volume::Matrix>( 4 );
      Volume volume( nll::core::vector3ui( 256, 1024, 1024 ), id );

      nll::imaging::MaximumIntensityProjection<Volume>   mipCreator( volume );

      Slice slice( nll::core::vector3ui( 1024, 1024, 1 ),
                   nll::core::vector3f( 0, 1, 0 ), 
                   nll::core::vector3f( 0, 0, 1 ),
                   nll::core::vector3f( -10, 512, 512 ),
                   nll::core::vector2f( 1, 1 ) );

      nll::core::Timer t1;
      mipCreator.computeMip<Interpolator>( slice, nll::core::vector3f( 1, 0, 0 ) );
      std::cout << "Time MIP 1024*1024=" << t1.getCurrentTime() << std::endl;
      std::cout << slice( 0, 0, 0 ) << std::endl;
   }


};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestMaximumIntensityProjection);
TESTER_TEST(testAutoMip);
//TESTER_TEST(testMip);
//TESTER_TEST(testMipBig);
TESTER_TEST_SUITE_END();
#endif
