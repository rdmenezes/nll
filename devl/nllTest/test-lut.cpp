#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>

// alignment float* alignedArray = (array + 15) & (~0x0F);
const unsigned size = 2048 * 4;

namespace nll
{
namespace imaging
{
   
}
}

class TestLut
{
public:
   typedef nll::imaging::MapperLutColor<unsigned char>               Mapper;
   typedef nll::imaging::LookUpTransform<unsigned char, Mapper>      Lut;

   void testBlending()
   {
      //nll::core::Configuration::instance().disableSSE();
      typedef nll::f32 type;
      typedef nll::imaging::MapperLutColor<type, nll::core::Allocator16ByteAligned<type> > ColorMapper;
      typedef nll::imaging::LookUpTransform<type, ColorMapper>   Lut;
      typedef nll::imaging::Slice<type>   Slice;

      ColorMapper mapper( 256, 3 );
      Lut lut( mapper, 10, 100 );
      const nll::ui32 sizex = 1024;//*4;
      const nll::ui32 sizey = 1024;//*4;
      Slice t1( nll::core::vector3ui( sizex, sizey, 1 ),
                nll::core::vector3f( 1.0f, 0, 0),
                nll::core::vector3f( 0, 1.0f, 0),
                nll::core::vector3f( 0, 0, 0),
                nll::core::vector2f( 1.0f, 1.0f ) );
      Slice t2( nll::core::vector3ui( sizex, sizey, 1 ),
                nll::core::vector3f( 1.0f, 0, 0),
                nll::core::vector3f( 0, 1.0f, 0),
                nll::core::vector3f( 0, 0, 0),
                nll::core::vector2f( 1.0f, 1.0f ) );
      nll::imaging::Slice<nll::ui8> out( nll::core::vector3ui( sizex, sizey, 3 ),
                nll::core::vector3f( 1.0f, 0, 0),
                nll::core::vector3f( 0, 1.0f, 0),
                nll::core::vector3f( 0, 0, 0),
                nll::core::vector2f( 1.0f, 1.0f ) );

      std::vector< nll::imaging::BlendSliceInfof<Lut> > infos;
      infos.push_back( nll::imaging::BlendSliceInfof<Lut>( t1, 0.5, lut ) );
      infos.push_back( nll::imaging::BlendSliceInfof<Lut>( t2, 0.5, lut ) );

/*
      nll::core::Timer t;
      nll::imaging::blend( infos, out );
      std::cout << "blending=" << t.getCurrentTime() << std::endl;
*/
      nll::core::Timer tt;
      nll::imaging::blendDummy( infos, out );
      std::cout << "blendingDummy=" << tt.getCurrentTime() << std::endl;
      std::cout << "val=" << *out.getIterator(0, 0) << std::endl;
   }

   void testBlendingPart()
   {
      //nll::core::Configuration::instance().disableSSE();
      typedef nll::f32 type;
      typedef nll::imaging::MapperLutColor<type, nll::core::Allocator16ByteAligned<type> > ColorMapper;
      typedef nll::imaging::LookUpTransform<type, ColorMapper>   Lut;
      typedef nll::imaging::Slice<type>   Slice;

      ColorMapper mapper( 256, 3 );
      Lut lut( mapper, 10, 100 );
      const nll::ui32 sizex = 1024*4;
      const nll::ui32 sizey = 1024*4;
      Slice t1( nll::core::vector3ui( sizex, sizey, 1 ),
                nll::core::vector3f( 1.0f, 0, 0),
                nll::core::vector3f( 0, 1.0f, 0),
                nll::core::vector3f( 0, 0, 0),
                nll::core::vector2f( 1.0f, 1.0f ) );
      Slice t2( nll::core::vector3ui( sizex, sizey, 1 ),
                nll::core::vector3f( 1.0f, 0, 0),
                nll::core::vector3f( 0, 1.0f, 0),
                nll::core::vector3f( 0, 0, 0),
                nll::core::vector2f( 1.0f, 1.0f ) );
      nll::imaging::Slice<nll::ui8> out( nll::core::vector3ui( sizex, sizey, 3 ),
                nll::core::vector3f( 1.0f, 0, 0),
                nll::core::vector3f( 0, 1.0f, 0),
                nll::core::vector3f( 0, 0, 0),
                nll::core::vector2f( 1.0f, 1.0f ) );

      std::vector< nll::imaging::BlendSliceInfof<Lut> > infos;
      infos.push_back( nll::imaging::BlendSliceInfof<Lut>( t1, 0.5, lut ) );
      infos.push_back( nll::imaging::BlendSliceInfof<Lut>( t2, 0.5, lut ) );

      nll::core::Timer tt;
      nll::imaging::blendDummyPart( infos, out, 0, 0, sizex/2, sizey/2 );
      nll::imaging::blendDummyPart( infos, out, 0, sizey/2, sizex/2, sizey );
      nll::imaging::blendDummyPart( infos, out, sizex/2, sizey/2, sizex, sizey );
      nll::imaging::blendDummyPart( infos, out, sizex/2, 0, sizex, sizey/2 );
      
      std::cout << "blendingDummyPart=" << tt.getCurrentTime() << std::endl;
      std::cout << "val=" << *out.getIterator(0, 0) << std::endl;
   }

   void simpleTest()
   {
      nll::imaging::LookUpTransformWindowingRGB lut( 0, 99, 10, 3 );

      lut.createGreyscale();

      {
         const float* v1 = lut.transform( -10 );
         TESTER_ASSERT( v1[ 0 ] == 0 );
         TESTER_ASSERT( v1[ 1 ] == 0 );
         TESTER_ASSERT( v1[ 2 ] == 0 );
      }

      {
         const float* v1 = lut.transform( 9 );
         TESTER_ASSERT( v1[ 0 ] == 0 );
         TESTER_ASSERT( v1[ 1 ] == 0 );
         TESTER_ASSERT( v1[ 2 ] == 0 );
      }

      {
         const float* v1 = lut.transform( 10 );
         TESTER_ASSERT( fabs( v1[ 0 ] - ( 10.0 / 100 * 256 ) ) < 1e-6 );
         TESTER_ASSERT( fabs( v1[ 1 ] - ( 10.0 / 100 * 256 ) ) < 1e-6 );
         TESTER_ASSERT( fabs( v1[ 2 ] - ( 10.0 / 100 * 256 ) ) < 1e-6 );
      }

      nll::imaging::LookUpTransformWindowingRGB lut2( 0, 255, 256, 1 );
      lut2.createGreyscale();
      TESTER_ASSERT( *lut2.transform( 10 ) == 10 );
      TESTER_ASSERT( *lut2.transform( 0 ) == 0 );
      TESTER_ASSERT( *lut2.transform( 255 ) == 255 );
      TESTER_ASSERT( *lut2.transform( 127 ) == 127 );
   }

   Lut createLut()
   {
      Mapper mapperLut( 256, 3 );
      Lut lut( mapperLut, -100, 1001 );
      return lut;
   }

   void testTransformComp()
   {
      srand( 0 );
      Lut lut = createLut();
      nll::core::Image<float> input( size, size, 1 );
      nll::core::Image<nll::ui32> output( size, size, 1 );
      nll::core::Image<nll::ui32> output2( size, size, 1 );
      for ( nll::core::Image<float>::iterator it = input.begin(); it != input.end(); ++it )
      {
         *it = (float)(rand() % 500);
      }

      nll::core::Image<float>::DirectionalIterator it = input.beginDirectional();
      nll::core::Image<nll::ui32>::DirectionalIterator out = output.beginDirectional();

      
      nll::core::Timer t1;
      for ( unsigned n = 0; n < input.sizex() * input.sizex(); ++n, ++it, ++out )
      {
         *out = lut.transformToIndex( *it );
      }
      std::cout << "LUT Time no multithreaded SSE=" << t1.getCurrentTime() << std::endl;
      

      
      nll::core::Timer t2;
      lut.transformToIndex( input.begin(), input.end(), output2.begin() );
      std::cout << "LUT Time with multithreaded SSE=" << t2.getCurrentTime() << std::endl;
      
      for ( unsigned n = 0; n < output.size(); ++n )
      {
         if ( output[ n ] != output2[ n ] )
            std::cout << "error val=" << output[ n ] << " " << output2[ n ] << std::endl;
         //TESTER_ASSERT( output[ n ] == output2[ n ] );
      }
   }

   void testLutDetectRange()
   {
      typedef nll::imaging::MapperLutColor<unsigned> Mapper;

      const unsigned nbValue = 10;
      unsigned values[] = 
      {
         0, 0, 1, 2, 3, 4, 3, 2, 0, 0
      };

      nll::imaging::Volume<double> volume( nbValue, 1, 1, 0 );
      for ( unsigned n = 0; n < nbValue; ++n )
      {
         volume( n, 0, 0 ) = values[ n ];
      }

      Mapper mapper( nbValue, 1 );
      nll::imaging::LookUpTransform<unsigned, Mapper> lut( mapper, 0, 1 );
      
      lut.detectRange( volume, 0.8, 5 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestLut);
TESTER_TEST(testLutDetectRange);
TESTER_TEST(testTransformComp);
TESTER_TEST(simpleTest);
TESTER_TEST(testBlending);
TESTER_TEST(testBlendingPart);
TESTER_TEST_SUITE_END();
#endif
