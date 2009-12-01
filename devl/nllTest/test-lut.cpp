#include "stdafx.h"
#include <nll/nll.h>
#include <sstream>

// alignment float* alignedArray = (array + 15) & (~0x0F);
const unsigned size = 2048 * 4;

namespace nll
{
namespace imaging
{
   /**
    @brief Blend slices into one slice.
    @param sliceInfos contains the info relative to the slice, LUT and blend factor.
           Each slice must at least contain the portion defined in the output slice (xmin, ymin, xmax, ymax)
           The slice must have the same level of scaling
           than the output slice
    @param out the slice already allocated.
    */
   template <class Lut, class OutType>
   void blendDummyPart( const std::vector< BlendSliceInfof<Lut> >& sliceInfos, Slice<OutType>& out, const ui32 xmin, const ui32 ymin, const ui32 xmax, const ui32 ymax )
   {
      typedef typename BlendSliceInfof<Lut>::Slice::value_type     InputType;
      typedef typename Slice<OutType>::value_type                  OutputType;
      typedef Slice<OutType>::iterator                             OutputIterator;
      typedef typename BlendSliceInfof<Lut>::Slice::ConstDirectionalIterator InputIterator;

      if ( !sliceInfos.size() )
         return;

      // cache the lut address & values in an array // else strange code is generated with VS2005SP1
      core::Buffer1D<const Lut*> luts( static_cast<ui32>( sliceInfos.size() ) );
      core::Buffer1D<float> blendFactors( static_cast<ui32>( sliceInfos.size() ) );
      std::vector< InputIterator > inputIterators;
      std::vector< InputIterator > inputIteratorsStartLine;
      for ( ui32 n = 0; n < sliceInfos.size(); ++n )
      {
         luts[ n ] = &sliceInfos[ n ].lut;
         blendFactors[ n ] = sliceInfos[ n ].blendFactor;
         inputIterators.push_back( sliceInfos[ n ].slice.getIterator( xmin, ymin ) );
         inputIteratorsStartLine.push_back( inputIterators[ n ] );

         // check the slices are correctly used

         // same scaling
         assert( core::equal<float>( sliceInfos[ n ].slice.getSpacing()[ 0 ], out.getSpacing()[ 0 ] ) );
         assert( core::equal<float>( sliceInfos[ n ].slice.getSpacing()[ 1 ], out.getSpacing()[ 1 ] ) );

         // same plane
         assert( core::equal( sliceInfos[ n ].slice.getNormal()[ 0 ], out.getNormal()[ 0 ], 1e-4f ) );
         assert( core::equal( sliceInfos[ n ].slice.getNormal()[ 1 ], out.getNormal()[ 1 ], 1e-4f ) );
         assert( core::equal( sliceInfos[ n ].slice.getNormal()[ 2 ], out.getNormal()[ 2 ], 1e-4f ) );
         
         // transform slice coordinate to output coordinate
         core::vector3f offset = sliceInfos[ n ].slice.getOrigin() - out.getOrigin();

         // check minmax is in the output slice
         core::vector3f min = offset - sliceInfos[ n ].slice.getAxisX() * out.getSpacing()[ 0 ] - sliceInfos[ n ].slice.getAxisY() * out.getSpacing()[ 1 ];
         core::vector3f max = offset + sliceInfos[ n ].slice.getAxisX() * out.getSpacing()[ 0 ] + sliceInfos[ n ].slice.getAxisY() * out.getSpacing()[ 1 ];
         assert( out.contains( min ) && out.contains( max ) );
      }

      const ui32 nbSlices = static_cast<ui32>( sliceInfos.size() );
      for ( ui32 ny = ymin; ny < ymax; ++ny )
      {
         Slice<OutType>::DirectionalIterator oit = out.getIterator( xmin, ny );
         for ( ui32 n = 0; n < sliceInfos.size(); ++n )
         {
            inputIteratorsStartLine[ n ].addcol();
            inputIterators[ n ] = inputIteratorsStartLine[ n ];
         }

         for ( ui32 nx = xmin; nx < xmax; ++nx )
         {
            float vala = 0;
            float valb = 0;
            float valc = 0;

            for ( ui32 n = 0; n < nbSlices; ++n )
            {
               // get the value: we are not using the postfix operator++ as it involves copying the iterator a lot
               InputIterator& it = inputIterators[ n ];
               const InputType val = *it;
               ++it;

               const typename Lut::value_type* buf = luts[ n ]->transform( val );
               const float blending = blendFactors[ n ];

               vala += buf[ 0 ] * blending;
               valb += buf[ 1 ] * blending;
               valc += buf[ 2 ] * blending;

            }
            oit.pickcol( 0 ) = static_cast<OutputType>( vala );
            oit.pickcol( 1 ) = static_cast<OutputType>( valb );
            oit.pickcol( 2 ) = static_cast<OutputType>( valc );

            oit.addcol();
         }
      }
   }
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
         TESTER_ASSERT( v1[ 0 ] == (int)( 10.0 / 100 * 256 ) );
         TESTER_ASSERT( v1[ 1 ] == (int)( 10.0 / 100 * 256 ) );
         TESTER_ASSERT( v1[ 2 ] == (int)( 10.0 / 100 * 256 ) );
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
         TESTER_ASSERT( output[ n ] == output2[ n ] );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestLut);
TESTER_TEST(testBlendingPart);
//TESTER_TEST(testTransformComp);
//TESTER_TEST(simpleTest);
TESTER_TEST(testBlending);
TESTER_TEST_SUITE_END();
#endif
