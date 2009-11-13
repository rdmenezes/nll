#ifndef NLL_IMAGING_SLICE_FUSION_H_
# define NLL_IMAGING_SLICE_FUSION_H_

namespace nll
{
namespace imaging
{
   template <class Lut>
   struct BlendSliceInfo
   {
      typedef core::Image<f32> Slice;

      BlendSliceInfo( Slice& s, float bf, Lut& l ) : slice( s ), blendFactor( bf ), lut( l )
      {}

      Slice                   slice;
      float                   blendFactor;
      Lut                     lut;
   };

#ifndef NLL_DISABLE_SSE_SUPPORT
   /**
    @ingroup imaging
    @brief Typically used compose the result of several MPR to create a RGB fused slice
    @note we assume the input slices have all the same size and output is already allocated
    */
   template <class Lut>
   void blendSSE( const std::vector< BlendSliceInfo<Lut> >& sliceInfos,
                  nll::core::Image<ui8>& out )
   {
      typedef core::Image<ui8>                                    OutputSlice;
      typedef core::Image<ui8>::iterator                          OutputIterator;
      typedef typename BlendSliceInfo<Lut>::Slice::const_iterator InputIterator;

      if ( !sliceInfos.size() )
         return;
      ensure( out.getNbComponents() == 3, "error must be RGB image" );
      ensure( out.size() == 3 * sliceInfos[ 0 ].slice.size(), "error must be the same size and RGB image" );

      std::vector< InputIterator > inputIterators( sliceInfos.size() );
      std::vector< core::Image<ui32> > indexes( sliceInfos.size() );
      for ( size_t n = 0; n < sliceInfos.size(); ++n )
      {
         inputIterators[ n ] = sliceInfos[ n ].slice.begin();
         indexes[ n ] = core::Image<ui32>( sliceInfos[ n ].slice.sizex(), sliceInfos[ n ].slice.sizey(), 1 );

         // we need to const_cast as the specialized version is not using 'const' iterators
         f32* s = const_cast<float*>( sliceInfos[ n ].slice.begin() );
         f32* e = const_cast<float*>( sliceInfos[ n ].slice.end() );
         sliceInfos[ n ].lut.transformToIndex( s, e, indexes[ n ].begin() );
      }

      ui32 p = 0;
      __declspec(align(16)) int result[ 4 ];
      __declspec(align(16)) float tmpVal[ 4 ] =
      {
         0, 0, 0, 0
      };
      for ( OutputIterator oit = out.begin(); oit != out.end(); oit += 3, ++p )
      {
         __m128 val = _mm_setzero_ps();

         for ( size_t n = 0; n < sliceInfos.size(); ++n )
         {
            const ui32 index = indexes[ n ][ p ];
            const ui8* fi = sliceInfos[ n ].lut[ index ];

            tmpVal[ 0 ] = fi[ 0 ];
            tmpVal[ 1 ] = fi[ 1 ];
            tmpVal[ 2 ] = fi[ 2 ];

            __m128 lutValue = _mm_load_ps( tmpVal );
            __m128 inputValue = _mm_set_ps1( inputIterators[ n ][ p ] );

            inputValue = _mm_mul_ps( lutValue, inputValue );
            val = _mm_add_ps( val, inputValue );
         }

         __m128i truncated = _mm_cvttps_epi32( val );
         _mm_store_si128( (__m128i*)result, truncated );

         oit[ 0 ] = static_cast<ui8>( result[ 0 ] );
         oit[ 1 ] = static_cast<ui8>( result[ 1 ] );
         oit[ 2 ] = static_cast<ui8>( result[ 2 ] );
      }
   }
#endif

   
   // dummy
   template <class Lut>
   void blendDummy( const std::vector< BlendSliceInfo<Lut> >& sliceInfos,
               nll::core::Image<ui8>& out )
   {
      typedef core::Image<ui8>                                    OutputSlice;
      typedef core::Image<ui8>::iterator                          OutputIterator;
      typedef typename BlendSliceInfo<Lut>::Slice::const_iterator InputIterator;

      if ( !sliceInfos.size() )
         return;
      ensure( out.getNbComponents() == 3, "error must be RGB image" );
      ensure( out.size() == 3 * sliceInfos[ 0 ].slice.size(), "error must be the same size and RGB image" );

      std::vector< InputIterator > inputIterators( sliceInfos.size() );
      std::vector< core::Image<ui32> > indexes( sliceInfos.size() );
      for ( size_t n = 0; n < sliceInfos.size(); ++n )
      {
         inputIterators[ n ] = sliceInfos[ n ].slice.begin();
         indexes[ n ] = core::Image<ui32>( sliceInfos[ n ].slice.sizex(), sliceInfos[ n ].slice.sizey(), 1 );

         // we need to const_cast as the specialized version is not using 'const' iterators
         f32* s = const_cast<float*>( sliceInfos[ n ].slice.begin() );
         f32* e = const_cast<float*>( sliceInfos[ n ].slice.end() );
         sliceInfos[ n ].lut.transformToIndex( s, e, indexes[ n ].begin() );
      }

      ui32 p = 0;
      for ( OutputIterator oit = out.begin(); oit != out.end(); oit += 3, ++p )
      {
         float vala = 0;
         float valb = 0;
         float valc = 0;

         for ( size_t n = 0; n < sliceInfos.size(); ++n )
         {
            const ui32 index = indexes[ n ][ p ];
            const ui8* fi = sliceInfos[ n ].lut[ index ];
            vala += inputIterators[ n ][ p ] * fi[ 0 ];
            valb += inputIterators[ n ][ p ] * fi[ 1 ];
            valc += inputIterators[ n ][ p ] * fi[ 2 ];
         }
         oit[ 0 ] = static_cast<ui8>(vala);
         oit[ 1 ] = static_cast<ui8>(valb);
         oit[ 2 ] = static_cast<ui8>(valc);
      }
   }

   template <class Lut>
   void blend( const std::vector< BlendSliceInfo<Lut> >& sliceInfos,
               nll::core::Image<ui8>& out )
   {
# ifndef NLL_DISABLE_SSE_SUPPORT
      if ( core::Configuration::instance().isSupportedSSE2() )
         blendSSE<Lut>( sliceInfos, out );
      else
         blendDummy<Lut>( sliceInfos, out );
#  else
      blendDummy<Lut>( sliceInfos, out );
# endif
   }
   
}
}

#endif
