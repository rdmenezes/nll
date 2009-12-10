#include "stdafx.h"
#include <nll/nll.h>

namespace
{
   template <class Volume>
   void createBlob( Volume& volume,
                    const nll::core::vector3ui& min,
                    const nll::core::vector3ui& max,
                    typename Volume::value_type val )
   {
      for ( unsigned z = min[ 2 ]; z < max[ 2 ]; ++z )
      {
         for ( unsigned y = min[ 1 ]; y < max[ 1 ]; ++y )
         {
            for ( unsigned x = min[ 0 ]; x < max[ 0 ]; ++x )
            {
               volume( x, y, z ) = val;
            }
         }
      }
   }
}

class TestMaximumIntensityProjection
{
public:
   void testAutoMipSyntheticData()
   {
      typedef nll::imaging::VolumeSpatial<float>            Volume;
      //typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume>   Interpolator;
      typedef nll::imaging::Slice<Volume::value_type>       Slice;

      Volume::Matrix id = nll::core::identityMatrix<Volume::Matrix>( 4 );
      Volume volume( nll::core::vector3ui( 128, 128, 64 ), id );
      createBlob( volume, nll::core::vector3ui( 60, 60, 30 ), nll::core::vector3ui( 68, 68, 34 ), 255 );
      createBlob( volume, nll::core::vector3ui( 0, 0, 0 ), nll::core::vector3ui( 10, 10, 10 ), 128 );
      createBlob( volume, nll::core::vector3ui( 117, 117, 53 ), nll::core::vector3ui( 127, 127, 63 ), 100 );

      std::cout << "automip1" << std::endl;
      nll::imaging::MaximumIntensityProjection<Volume>   mipCreator( volume );

      int n = 0;
      for ( double f = nll::core::PI / 50 * 0; f < nll::core::PI; f += nll::core::PI / 15, ++n )
      {
         nll::core::Timer t;
         Slice slice = mipCreator.getAutoOrientedMip<Interpolator>( (float)f, 256, 256, 1.0f, 1.0f );
         std::cout << "slice t=" << t.getCurrentTime() << std::endl;
         nll::imaging::LookUpTransformWindowingRGB lut( 0, 256, 256, 3 );

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
         nll::core::writeBmp( i, NLL_TEST_PATH "data/synth-mip1-256-auto" + nll::core::val2str(n) + ".bmp" );
      }
   }

   void testAutoMipSyntheticDataTranslation()
   {
      typedef nll::imaging::VolumeSpatial<float>            Volume;
      //typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume>   Interpolator;
      typedef nll::imaging::Slice<Volume::value_type>       Slice;

      Volume::Matrix tfm = nll::core::identityMatrix<Volume::Matrix>( 4 );
      tfm( 0, 3 ) = 400;
      tfm( 1, 3 ) = 500;
      tfm( 2, 3 ) = -200;
      Volume volume( nll::core::vector3ui( 128, 128, 64 ), tfm );
      createBlob( volume, nll::core::vector3ui( 60, 60, 30 ), nll::core::vector3ui( 68, 68, 34 ), 255 );
      createBlob( volume, nll::core::vector3ui( 0, 0, 0 ), nll::core::vector3ui( 10, 10, 10 ), 128 );
      createBlob( volume, nll::core::vector3ui( 117, 117, 53 ), nll::core::vector3ui( 127, 127, 63 ), 100 );

      std::cout << "automip2" << std::endl;
      nll::imaging::MaximumIntensityProjection<Volume>   mipCreator( volume );

      int n = 0;
      for ( double f = nll::core::PI / 50 * 0; f < nll::core::PI; f += nll::core::PI / 15, ++n )
      {
         nll::core::Timer t;
         Slice slice = mipCreator.getAutoOrientedMip<Interpolator>( (float)f, 256, 256, 1.0f, 1.0f );
         std::cout << "slice t=" << t.getCurrentTime() << std::endl;
         nll::imaging::LookUpTransformWindowingRGB lut( 0, 256, 256, 3 );

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
         nll::core::writeBmp( i, NLL_TEST_PATH "data/synth-mip2-256-auto" + nll::core::val2str(n) + ".bmp" );
      }
   }

   void testAutoMipSyntheticDataRotation()
   {
      typedef nll::imaging::VolumeSpatial<float>            Volume;
      //typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume>   Interpolator;
      typedef nll::imaging::Slice<Volume::value_type>       Slice;

      Volume::Matrix tfm = nll::core::identityMatrix<Volume::Matrix>( 4 );
      nll::core::matrix4x4RotationZ( tfm, (float)nll::core::PI/2 );
      tfm( 0, 3 ) = 400;
      tfm( 1, 3 ) = 500;
      tfm( 2, 3 ) = -200;
      Volume volume( nll::core::vector3ui( 128, 128, 64 ), tfm );
      createBlob( volume, nll::core::vector3ui( 60, 60, 30 ), nll::core::vector3ui( 68, 68, 34 ), 255 );
      createBlob( volume, nll::core::vector3ui( 0, 0, 0 ), nll::core::vector3ui( 10, 10, 10 ), 128 );
      createBlob( volume, nll::core::vector3ui( 117, 117, 53 ), nll::core::vector3ui( 127, 127, 63 ), 100 );

      std::cout << "automip3" << std::endl;
      nll::imaging::MaximumIntensityProjection<Volume>   mipCreator( volume );

      int n = 0;
      for ( double f = nll::core::PI / 50 * 0; f < nll::core::PI; f += nll::core::PI / 15, ++n )
      {
         nll::core::Timer t;
         Slice slice = mipCreator.getAutoOrientedMip<Interpolator>( (float)f, 256, 256, 1.0f, 1.0f );
         std::cout << "slice t=" << t.getCurrentTime() << std::endl;
         nll::imaging::LookUpTransformWindowingRGB lut( 0, 256, 256, 3 );

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
         nll::core::writeBmp( i, NLL_TEST_PATH "data/synth-mip3-256-auto" + nll::core::val2str(n) + ".bmp" );
      }
   }

   void testAutoMip()
   {
      typedef nll::imaging::VolumeSpatial<float>            Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      //typedef nll::imaging::InterpolatorNearestNeighbour<Volume>   Interpolator;
      typedef nll::imaging::Slice<Volume::value_type>       Slice;

      const std::string volname = NLL_TEST_PATH "data/medical/pet.mf2";
      //const std::string volname = NLL_TEST_PATH "data/medical/pet-NAC.mf2";
      Volume volume;
      bool loaded = nll::imaging::loadSimpleFlatFile( volname, volume );
      TESTER_ASSERT( loaded );


      std::cout << "automip=" << volume.getSize()[ 0 ] << " " << volume.getSize()[ 1 ] << " " << volume.getSize()[ 2 ] << std::endl;
      nll::imaging::MaximumIntensityProjection<Volume>   mipCreator( volume );

      int n = 0;
      for ( double f = nll::core::PI / 50 * 0; f < 2 * nll::core::PI; f += nll::core::PI / 50, ++n )
      {
         nll::core::Timer t;
         Slice slice = mipCreator.getAutoOrientedMip<Interpolator>( (float)f, 256, 512, 2.0f, 2.0f );
         std::cout << "slice t=" << t.getCurrentTime() << std::endl;
         nll::imaging::LookUpTransformWindowingRGB lut( -1000, 15000, 256, 3 );

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
                   nll::core::vector3f( 0, 0, -650 ),
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
TESTER_TEST(testAutoMipSyntheticDataRotation);
TESTER_TEST(testAutoMipSyntheticDataTranslation);
TESTER_TEST(testAutoMipSyntheticData);
TESTER_TEST(testAutoMip);
TESTER_TEST(testMip);
TESTER_TEST(testMipBig);
TESTER_TEST_SUITE_END();
#endif
