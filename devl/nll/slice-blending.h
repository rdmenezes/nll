#ifndef NLL_IMAGING_SLICE_FUSION_H_
# define NLL_IMAGING_SLICE_FUSION_H_

# pragma warning( push )
# pragma warning( disable:4127 ) // conditional expression constant // intended!

// There is no real benefits against default compilation with SSE2 support
// so the feature is disabled but SSE implementation kept for trials only...
//# define DISABLE_SSE_BLENDING_OPTIM

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

#ifndef DISABLE_SSE_BLENDING_OPTIM
#ifndef NLL_DISABLE_SSE_SUPPORT
   /**
    @ingroup imaging
    @brief Typically used compose the result of several MPR to create a RGB fused slice
    @note we assume the input slices have all the same size and output is already allocated

    - Lut buffer is a float buffer
    - Lut buffer is 16 bytes aligned
    - Lut is allocating 1 extra bloc (so we can load 4 values directly)
    */
   template <class Lut>
   void blendSSE( const std::vector< BlendSliceInfo<Lut> >& sliceInfos,
                  nll::core::Image<ui8>& out )
   {
      std::cout << "SSE" << std::endl;
      typedef core::Image<ui8>                                    OutputSlice;
      typedef core::Image<ui8>::iterator                          OutputIterator;
      typedef typename BlendSliceInfo<Lut>::Slice::const_iterator InputIterator;

      if ( !sliceInfos.size() )
         return;
      ensure( out.getNbComponents() == 3, "error must be RGB image" );
      ensure( out.size() == 3 * sliceInfos[ 0 ].slice.size(), "error must be the same size and RGB image" );
      const int isApplicable = core::Equal<f32, typename Lut::value_type>::value;
      ensure( isApplicable, "should only be run with float LUT" );

      std::vector< InputIterator > inputIterators( sliceInfos.size() );
      for ( size_t n = 0; n < sliceInfos.size(); ++n )
      {
         inputIterators[ n ] = sliceInfos[ n ].slice.begin();
      }

      __declspec(align(16)) int result[ 4 ];
      for ( OutputIterator oit = out.begin(); oit != out.end(); oit += 3 )
      {
         __m128 val = _mm_setzero_ps();

         for ( size_t n = 0; n < sliceInfos.size(); ++n )
         {
            // we mainly need this for compilation only... it can be instanciated with any type
            // but should never be run with a type different than float
            const typename Lut::value_type* buf = sliceInfos[ n ].lut.transform( *inputIterators[ n ] );
            const float intensity = sliceInfos[ n ].blendFactor;

            const __m128 lutValue = _mm_load_ps( reinterpret_cast<const float*>( buf ) );
            __m128 inputValue = _mm_set_ps1( intensity );

            inputValue = _mm_mul_ps( lutValue, inputValue );
            val = _mm_add_ps( val, inputValue );

            ++inputIterators[ n ];
         }

         __m128i truncated = _mm_cvttps_epi32( val );
         _mm_store_si128( (__m128i*)result, truncated );

         oit[ 0 ] = static_cast<ui8>( result[ 0 ] );
         oit[ 1 ] = static_cast<ui8>( result[ 1 ] );
         oit[ 2 ] = static_cast<ui8>( result[ 2 ] );
      }
   }
#endif
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
      }

      ui32 p = 0;
      for ( OutputIterator oit = out.begin(); oit != out.end(); oit += 3, ++p )
      {
         float vala = 0;
         float valb = 0;
         float valc = 0;

         for ( size_t n = 0; n < sliceInfos.size(); ++n )
         {
            const typename Lut::value_type* buf = sliceInfos[ n ].lut.transform( *inputIterators[ n ] );
            const float intensity = sliceInfos[ n ].blendFactor;

            vala += buf[ 0 ] * intensity;
            valb += buf[ 1 ] * intensity;
            valc += buf[ 2 ] * intensity;

            ++inputIterators[ n ];
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
#ifdef DISABLE_SSE_BLENDING_OPTIM
      blendDummy<Lut>( sliceInfos, out );
# else
#  ifndef NLL_DISABLE_SSE_SUPPORT
      if ( core::Configuration::instance().isSupportedSSE2() && core::Equal<float, typename Lut::value_type>::value )
         blendSSE<Lut>( sliceInfos, out );
      else
         blendDummy<Lut>( sliceInfos, out );
#   else
      blendDummy<Lut>( sliceInfos, out );
#  endif
# endif
   }
   
}
}

# pragma warning( pop )
#endif
