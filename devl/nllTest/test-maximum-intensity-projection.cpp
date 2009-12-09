#include "stdafx.h"
#include <nll/nll.h>



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
      for ( float f = nll::core::PI / 50 * 0; f < 2 * nll::core::PI; f += nll::core::PI / 50, ++n )
      {
         nll::core::Timer t;
         Slice slice = mipCreator.getAutoOrientedMip<Interpolator>( f, 256, 256, 1.0f, 1.0f );
         std::cout << "slice t=" << t.getCurrentTime() << std::endl;
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
TESTER_TEST(testAutoMip);
TESTER_TEST(testMip);
TESTER_TEST(testMipBig);
TESTER_TEST_SUITE_END();
#endif
