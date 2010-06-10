#ifndef NLL_IMAGING_SLICE_FUSION_H_
# define NLL_IMAGING_SLICE_FUSION_H_

# pragma warning( push )
# pragma warning( disable:4127 ) // conditional expression constant // intended!

// There is no real benefits against default compilation with SSE2 support
// so the feature is disabled but SSE implementation kept for trials only...
# define DISABLE_SSE_BLENDING_OPTIM

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Holds necessary infos to blend slice such as blending factor, LUT & slice.
    */
   template <class Lut, class InputType>
   struct BlendSliceInfo
   {
      typedef imaging::Slice<InputType>      Slice;

      BlendSliceInfo( Slice& s, float bf, Lut& l ) : slice( s ), blendFactor( bf ), lut( l )
      {}

      BlendSliceInfo& operator=( const BlendSliceInfo& rhs )
      {
         slice = rhs.slice;
         blendFactor = rhs.blendFactor;
         lut = rhs.lut;
         return *this;
      }

      Slice&                  slice;
      float                   blendFactor;
      Lut&                    lut;
   };

   /**
    @ingroup imaging
    @brief Defines blending information for floating type based slice
    */
   template <class Lut>
   struct BlendSliceInfof : public BlendSliceInfo<Lut, f32>
   {
      typedef  BlendSliceInfo<Lut, f32>      Base;

      BlendSliceInfof( typename Base::Slice& s, float bf, Lut& l ) : Base( s, bf, l )
      {}
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
   void blendSSE( const std::vector< BlendSliceInfo<Lut> >& sliceInfosf,
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

      // cache the lut address & values in an array // else strange code is generated with VS2005SP1
      core::Buffer1D<const Lut*> luts( static_cast<ui32>( sliceInfos.size() ) );
      std::vector<__m128> blendFactors( sliceInfos.size() );
      for ( ui32 n = 0; n < sliceInfos.size(); ++n )
      {
         luts[ n ] = &sliceInfos[ n ].lut;
         blendFactors[ n ] = _mm_set_ps1( sliceInfos[ n ].blendFactor );
      }

      __declspec(align(16)) int result[ 4 ];
      const ui32 nbSlices = static_cast<ui32>( sliceInfos.size() );
      for ( OutputIterator oit = out.begin(); oit != out.end(); oit += 3 )
      {
         __m128 val = _mm_setzero_ps();

         for ( ui32 n = 0; n < nbSlices; ++n )
         {
            // we mainly need this for compilation only... it can be instanciated with any type
            // but should never be run with a type different than float
            const typename Lut::value_type* buf = luts[ n ]->transform( *inputIterators[ n ]++ );
            const __m128 lutValue = _mm_load_ps( reinterpret_cast<const float*>( buf ) );
            val = _mm_add_ps( val, _mm_mul_ps( lutValue, blendFactors[ n ] ) );
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

   
   /**
    @ingroup imaging
    @brief Blend slices. All sizes, origin, spacing of input/output slices must match
    */
   template <class Lut, class OutType>
   void blendDummy( const std::vector< BlendSliceInfof<Lut> >& sliceInfos, Slice<OutType>& out )
   {
      typedef typename Slice<OutType>::value_type                  OutputType;
      typedef typename Slice<OutType>::iterator                    OutputIterator;
      typedef typename BlendSliceInfof<Lut>::Slice::const_iterator InputIterator;

      if ( !sliceInfos.size() )
         return;

      // cache the lut address & values in an array // else strange code is generated with VS2005SP1
      core::Buffer1D<const Lut*> luts( static_cast<ui32>( sliceInfos.size() ) );
      core::Buffer1D<float> blendFactors( static_cast<ui32>( sliceInfos.size() ) );
      std::vector< InputIterator > inputIterators( sliceInfos.size() );
      for ( ui32 n = 0; n < sliceInfos.size(); ++n )
      {
         luts[ n ] = &sliceInfos[ n ].lut;
         blendFactors[ n ] = sliceInfos[ n ].blendFactor;
         inputIterators[ n ] = sliceInfos[ n ].slice.begin();

         // check the slices are correctly used

         // same scaling
         assert( core::equal<float>( sliceInfos[ n ].slice.getSpacing()[ 0 ], out.getSpacing()[ 0 ] ) );
         assert( core::equal<float>( sliceInfos[ n ].slice.getSpacing()[ 1 ], out.getSpacing()[ 1 ] ) );

         // same plane
         assert( core::equal( sliceInfos[ n ].slice.getNormal()[ 0 ], out.getNormal()[ 0 ], 1e-4f ) );
         assert( core::equal( sliceInfos[ n ].slice.getNormal()[ 1 ], out.getNormal()[ 1 ], 1e-4f ) );
         assert( core::equal( sliceInfos[ n ].slice.getNormal()[ 2 ], out.getNormal()[ 2 ], 1e-4f ) );
         
         // transform slice coordinate to output coordinate
         //core::vector3f offset = sliceInfos[ n ].slice.getOrigin() - out.getOrigin();

         // check minmax is in the output slice
         //core::vector3f min = sliceInfos[ n ].slice.getOrigin() - sliceInfos[ n ].slice.getAxisX() * out.getSpacing()[ 0 ] / 2 - sliceInfos[ n ].slice.getAxisY() * out.getSpacing()[ 1 ] / 2;
         //core::vector3f max = sliceInfos[ n ].slice.getOrigin() + sliceInfos[ n ].slice.getAxisX() * out.getSpacing()[ 0 ] / 2 + sliceInfos[ n ].slice.getAxisY() * out.getSpacing()[ 1 ] / 2;
         //assert( out.contains( min ) && out.contains( max ) );
      }

      const ui32 nbSlices = static_cast<ui32>( sliceInfos.size() );
      for ( OutputIterator oit = out.begin(); oit != out.end(); oit += 3 )
      {
         float vala = 0;
         float valb = 0;
         float valc = 0;

         for ( ui32 n = 0; n < nbSlices; ++n )
         {
            const typename Lut::value_type* buf = luts[ n ]->transform( *inputIterators[ n ]++ );
            const float blending = blendFactors[ n ];

            vala += buf[ 0 ] * blending;
            valb += buf[ 1 ] * blending;
            valc += buf[ 2 ] * blending;

         }
         oit[ 0 ] = static_cast<OutputType>( vala );
         oit[ 1 ] = static_cast<OutputType>( valb );
         oit[ 2 ] = static_cast<OutputType>( valc );
      }
   }

   /**
    @brief Blend slices into one slice.
    @param sliceInfos contains the info relative to the slice, LUT and blend factor.
           Each slice must at least contain the portion defined in the output slice (xmin, ymin, xmax, ymax)
           The slice must have the same level of scaling than the output slice
    @param out the slice already allocated.
    */
   template <class Lut, class OutType>
   void blendDummyPart( const std::vector< BlendSliceInfof<Lut> >& sliceInfos, Slice<OutType>& out, const ui32 xmin, const ui32 ymin, const ui32 xmax, const ui32 ymax )
   {
      typedef typename BlendSliceInfof<Lut>::Slice::value_type     InputType;
      typedef typename Slice<OutType>::value_type                  OutputType;
      typedef typename Slice<OutType>::iterator                    OutputIterator;
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
         typename Slice<OutType>::DirectionalIterator oit = out.getIterator( xmin, ny );
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

   /**
    @ingroup imaging
    @brief Blend slices.
    */
   template <class Lut, class OutType>
   void blend( const std::vector< BlendSliceInfof<Lut> >& sliceInfos,
               Slice<OutType>& out )
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
