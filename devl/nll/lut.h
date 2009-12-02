#ifndef NLL_LUT_H_
# define NLL_LUT_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief A lut buffer.
    @note Allocator is used to allocate the buffer. One extra block will always be allocated to allow a specific
          optimization
    */
   template <class T, class Allocator = std::allocator<T> >
   class MapperLutColor
   {
      typedef core::Buffer1D<T, core::IndexMapperFlat1D, Allocator>  Vector;
      typedef core::Buffer1D<T*>                                     VectorIndex;

   public:
      /**
       @brief Init the mapper.
       @param size the number of index to generate
       @param components the number of components the mapper will store for each index
       @note one extra block is allocated to enable sse optimization more efficiently
       */
      MapperLutColor( ui32 size, ui32 components ) : _size( size ), _components( components ), _container( size * components + 1, true )
      {
         _index = VectorIndex( size );
         for ( ui32 n = 0; n < size; ++n )
         {
            _index[ n ] = &_container[ n * components ];
         }
      }

      /**
       @brief Set value for an index.
       @param index the index where the value will be stored
       @param value a multidimentional value, that must have the same size than specified in constructor
              The value is copied.
       */
      void set( ui32 index, const T* value )
      {
         for ( ui32 n = 0; n < _components; ++n )
            _index[ index ][ n ] = value[ n ];
      }

      /**
       @brief return a pointer on the multidimensional value pointed by index.
       */
      const T* operator[]( ui32 index ) const
      {
         return _index[ index ];
      }

      /**
       @brief returns the number of components of a point
       */
      ui32 getNbComponents() const
      {
         return _components;
      }

      /**
       @brief Returns the number of indexes that can be adressed
       */
      ui32 getSize() const
      {
         return _size;
      }

   private:
      ui32        _size;
      ui32        _components;
      Vector      _container;
      VectorIndex _index;
   };

   /**
    @ingroup imaging
    @brief Look up table

    The LUT maps a 'double' to a multidimentional value. The 'double' is scaled and transformed to an index. Each
    index has a mapped value.

    TMapper should provide
      copy constructor
      const T* operator[](index) const
      getSize() const 
      getNbComponents() const
      set( ui32 index, T* value )
    */
   template <class T, class TMapper>
   class LookUpTransform
   {
   public:
      typedef T      value_type;

      /**
       @brief Init the look up table.
       @param mapper the mapper to be used. It is internally copied.
       @param minIntensity the minimal intensity to be displayed
       @param maxIntensity the maximal intensity to be displayed
       */
      LookUpTransform( const TMapper& mapper, float minIntensity, float maxIntensity ) : _mapper( mapper ), _min( minIntensity ), _max( maxIntensity ), _interval( maxIntensity - minIntensity + 1 )
      {
         ensure( _interval > 0, "must be >0" );
         _ratio = static_cast<float>( _mapper.getSize() ) / _interval;
      }

      /**
       @brief Transform a double to multidimentional value
       */
      const T* transform( float value ) const
      {
         return _mapper[ transformToIndex( value ) ];
      }

      const T* operator[]( ui32 index ) const
      {
         ensure( index < _mapper.getSize(), "error out of bound" );
         return _mapper[ index ];
      }

      ui32 transformToIndex( float value ) const
      {
         if ( value < _min )
            return 0;
         if ( value > _max )
            return _mapper.getSize() - 1;
         return (ui32)( ( value - _min ) * _ratio );
      }

      template <class InputIterator, class OutputIterator>
      void transformToIndex( InputIterator first, InputIterator last, OutputIterator output ) const
      {
         for ( ; first != last; ++first )
         {
            *output++ = transformToIndex( *first );
         }
      }

      template <>
      void transformToIndex( float* first, float* last, ui32* output ) const
      {
         std::cout << "specialized" << std::endl;
         size_t size = last - first;

# ifndef NLL_NOT_MULTITHREADED
         // finally the multithreaded version doesn't not improve the time performance
         // usuall it is a very small fraction of time for blending a frame and it costs
         // more to create the threads and synchronize them, it is only worth it when
         // the number of indexes to convert is huge
         //#pragma omp parallel
# endif
         {
# ifndef NLL_NOT_MULTITHREADED
            const int numberOfThreads = omp_get_num_threads();
				const int threadNumber = omp_get_thread_num();

            ui32* o = output + size * threadNumber / numberOfThreads;
            const float* i = first  + size * threadNumber / numberOfThreads;
            const float* l = first  + size * ( threadNumber + 1 ) / numberOfThreads;
            std::cout << "thread=" << threadNumber << " size=" << l - i << std::endl;

#  ifdef NLL_DISABLE_SSE_SUPPORT
            transformSingleThreaded( i, l, o );
#   else
            if ( core::Configuration::instance().isSupportedSSE2() )
            {
               const unsigned int rm = _MM_GET_ROUNDING_MODE();
					_MM_SET_ROUNDING_MODE(_MM_ROUND_DOWN);

               transformSingleThreadedSSE( i, l, o );

               _MM_SET_ROUNDING_MODE( rm );
            }
            else
               transformSingleThreaded( i, l, o );
#  endif
#  else
            transformSingleThreaded( first, last, output );
# endif
         }
      }

      void transformSingleThreaded( const float* first, const float* last, ui32* output ) const
      {
         for ( ; first != last; ++first )
         {
            *output++ = transformToIndex( *first );
         }
      }

# ifndef NLL_DISABLE_SSE_SUPPORT
      void transformSingleThreadedSSE( const float* first, const float* last, ui32* output ) const
      {
         // manually transform the non 16bit aligned elements
         const float* p = first;
         while ( ( ( ( reinterpret_cast<size_t> ( p ) ) & 15 ) != 0 ) )
			{
				if ( p == last )
				{
					return;
				}
            *output++ = transformToIndex( *p++ );
			}


         // we need to padd 4 (we loop 4 by 4)
         const float* end16 = last - 4;

         // start vectorisation
         const __m128 invslope = _mm_set_ps1( static_cast<float>( _ratio ) );
         const __m128 intercept = _mm_set_ps1( static_cast<float>( _min ) );
         const __m128 minimumValue = _mm_setzero_ps();
			const __m128 maximumValue = _mm_set1_ps( static_cast< float >( _mapper.getSize() - 1 ) );

         __declspec(align(16)) int index[ 4 ];
         while ( p < end16 )
         {
            const __m128 rawValue = _mm_load_ps( p ); // load 4 values
            __m128 value = _mm_mul_ps( _mm_sub_ps( rawValue, intercept ), invslope );
            const int lowCompMask = _mm_movemask_ps( _mm_cmplt_ps( value, minimumValue ) );
				const int highCompMask = _mm_movemask_ps( _mm_cmpge_ps( value, maximumValue ) );
            
            __m128i floored = _mm_cvtps_epi32( value );
            _mm_store_si128( (__m128i*)index, floored );
            
            *output++ = ( lowCompMask & 0x01 ) ? 0 : (highCompMask & 0x1 ) ? _mapper.getSize() : index[ 0 ];
            *output++ = ( lowCompMask & 0x02 ) ? 0 : (highCompMask & 0x2 ) ? _mapper.getSize() : index[ 1 ];
            *output++ = ( lowCompMask & 0x03 ) ? 0 : (highCompMask & 0x3 ) ? _mapper.getSize() : index[ 2 ];
            *output++ = ( lowCompMask & 0x04 ) ? 0 : (highCompMask & 0x4 ) ? _mapper.getSize() : index[ 3 ];            
            p += 4;
         }

         for ( ; p != last; ++p )
         {
            *output++ = transformToIndex( *p );
         }
      }
# endif

      /**
       @brief Returns the number of mapper indexes
       */
      ui32 getSize() const
      {
         return _mapper.getSize();
      }

      /**
       @brief Returns the number of components each value in the LUT has
       */
      ui32 getNbComponents() const
      {
         return _mapper.getNbComponents();
      }

      /**
       @brief Set a new value for the specified index. Internally, the value is copied.
       */
      void set( ui32 index, const T* value )
      {
         _mapper.set( index, value );
      }

   protected:
      TMapper  _mapper;
      float   _min;
      float   _max;
      float   _interval;
      float   _ratio;
   };

   /**
    @ingroup imaging
    @brief Implementation of a LUT for double->RGB values
    */
   class LookUpTransformWindowingRGB
   {
   public:
      typedef float                         value_type;

   protected:
      typedef MapperLutColor<value_type>  LutMapper;
      typedef LookUpTransform<value_type, MapperLutColor<value_type> > Lut;

   public:
      LookUpTransformWindowingRGB( float minIntensity, float maxIntensity, ui32 size, ui32 nbComponents = 3 ) :
         _lut( LutMapper( size, nbComponents ), minIntensity, maxIntensity )
      {}

      void reset( float minIntensity, float maxIntensity, ui32 size, ui32 nbComponents = 3 )
      {
         _lut = Lut( LutMapper( size, nbComponents ), minIntensity, maxIntensity );
      }

      const value_type* transform( float value ) const
      {
         return _lut.transform( value );
      }

      /**
       @brief Do the same than <code>transform</code> but instead do it on a sequence and returns
              the index in the output iterator. In the case InputIterator=float* and
              OutputIterator=ui32*, it will be optimized with SSE & multithreading

              On a dualcore-SSE, 10x increase compare to the default implementation
       */
      template <class InputIterator, class OutputIterator>
      void transformToIndex( InputIterator start, InputIterator end, OutputIterator output )
      {
         _lut.transformToIndex( start, end, output );
      }

      ui32 getSize() const
      {
         return _lut.getSize();
      }

      ui32 getNbComponents() const
      {
         return _lut.getNbComponents();
      }

      void set( ui32 index, const value_type* value )
      {
         _lut.set( index, value );
      }

      void createGreyscale()
      {
         core::Buffer1D<value_type> vals( _lut.getNbComponents() );
         for ( ui32 n = 0; n < _lut.getSize(); ++n )
         {
            for ( ui32 i = 0; i < _lut.getNbComponents(); ++i )
            {
               vals[ i ] = static_cast<value_type>( 256.0 / _lut.getSize() * n );
            }
            set( n, vals.getBuf() );
         }
      }

      /**
       @param baseColor an index that must contain getNbComponents() components
       */ 
      void createColorScale( const value_type* baseColor )
      {
         core::Buffer1D<value_type> vals( _lut.getNbComponents() );
         for ( ui32 n = 0; n < _lut.getSize(); ++n )
         {
            for ( ui32 i = 0; i < _lut.getNbComponents(); ++i )
            {
               double ratio = (double)n / _lut.getSize();
               vals[ i ] = static_cast<value_type>( (  ( ratio ) * baseColor[ i ] + ( 1 - ratio ) * 255 ) / _lut.getSize() * n );
            }
            set( n, vals.getBuf() );
         }
      }

   protected:
      Lut      _lut;
   };
}
}

#endif
