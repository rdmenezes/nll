/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
       @brief Automatically detect the range of the LUT so that <ratioSelection> % of the voxels are selected
       */
      template <class Volume>
      void detectRange( const Volume& v, double ratioSelection, ui32 nbBins = 1000 )
      {
         ensure( ratioSelection >= 0 && ratioSelection <= 1 && nbBins > 2, "invalid parameters" );

         // compute the range so we can do binning
         typename Volume::value_type min = std::numeric_limits<typename Volume::value_type>::max();
         typename Volume::value_type max = std::numeric_limits<typename Volume::value_type>::min();
         for ( typename Volume::const_iterator it = v.begin(); it != v.end(); ++it )
         {
            if ( *it > max )
            {
               max = *it;
            }
            if ( *it < min )
            {
               min = *it;
            }
         }

         // do binning
         std::vector<size_t> bins( nbBins );
         const double range = ( static_cast<double>( max ) - static_cast<double>( min ) + 1 ) / nbBins;
         for ( typename Volume::const_iterator it = v.begin(); it != v.end(); ++it )
         {
            const ui32 bin = static_cast<ui32>( ( *it - min ) / range );
            ++bins[ bin ];
         }

         // select the range that select the specified ratio of voxel
         // sort the bins by size and greedily add bins to the selected bins
         const size_t nbVoxels = v.size()[ 0 ] * v.size()[ 1 ] * v.size()[ 2 ];
         double selectedVoxels = 0;

         typedef std::vector< std::pair< size_t, ui32 > > SortedContainer;
         SortedContainer sortedBins( bins.size() );
         for ( ui32 n = 0; n < nbBins; ++n )
         {
            sortedBins[ n ] = std::pair<size_t, ui32>( bins[ n ], n );
         }
         std::sort( sortedBins.rbegin(), sortedBins.rend() );

         // compute the last bin that falls within the ratio
         SortedContainer::const_iterator it = sortedBins.begin();
         for ( ; it != sortedBins.end() && selectedVoxels / nbVoxels < ratioSelection; ++it )
         {
            selectedVoxels += it->first;
         }

         // compute the min-max index
         ui32 minIndex = nbBins;
         ui32 maxIndex = 0;
         for ( SortedContainer::const_iterator it2 = sortedBins.begin(); it2 != it; ++it2 )
         {
            if ( it2->second > maxIndex )
            {
               maxIndex = it2->second;
            }

            if ( it2->second < minIndex )
            {
               minIndex = it2->second;
            }
         }

         _min = static_cast<float>( min + minIndex * range );
         _max = static_cast<float>( min + maxIndex * range );
         _interval = _max - _min + 1;
         _ratio = static_cast<float>( _mapper.getSize() ) / _interval;
      }

      

      /**
       @brief Transform a float to multidimentional value
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

// if we disable SSE, this is pointless to have a specialized template
// additionally, this is not standard compliant (the specialized template should be outside class, which will force a full spacialization.. Mapper.. which we don't want...)
#  ifndef NLL_DISABLE_SSE_SUPPORT
      template <>
      void transformToIndex( float* first, float* last, ui32* output ) const
      {
         std::cout << "specialized" << std::endl;
         size_t size = last - first;

#  ifndef NLL_NOT_MULTITHREADED
         // finally the multithreaded version doesn't not improve the time performance
         // usuall it is a very small fraction of time for blending a frame and it costs
         // more to create the threads and synchronize them, it is only worth it when
         // the number of indexes to convert is huge
         #pragma omp parallel
#  endif
         {
#  ifndef NLL_NOT_MULTITHREADED
            const int numberOfThreads = omp_get_num_threads();
				const int threadNumber = omp_get_thread_num();

            ui32* o = output + size * threadNumber / numberOfThreads;
            const float* i = first  + size * threadNumber / numberOfThreads;
            const float* l = first  + size * ( threadNumber + 1 ) / numberOfThreads;
            std::cout << "thread=" << threadNumber << " size=" << l - i << std::endl;

#   ifdef NLL_DISABLE_SSE_SUPPORT
            transformSingleThreaded( i, l, o );
#    else
            if ( core::Configuration::instance().isSupportedSSE2() )
            {
               const unsigned int rm = _MM_GET_ROUNDING_MODE();
					_MM_SET_ROUNDING_MODE(_MM_ROUND_DOWN);

               transformSingleThreadedSSE( i, l, o );

               _MM_SET_ROUNDING_MODE( rm );
            }
            else
               transformSingleThreaded( i, l, o );
#   endif
#   else
            transformSingleThreaded( first, last, output );
#  endif
         }
      }

      void transformSingleThreaded( const float* first, const float* last, ui32* output ) const
      {
         for ( ; first != last; ++first )
         {
            *output++ = transformToIndex( *first );
         }
      }
# endif

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

      const value_type* get( ui32 index ) const
      {
         return _lut[ index ];
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

      template <class Volume>
      void detectRange( const Volume& v, double ratioSelection, ui32 nbBins = 1000 )
      {
         _lut.detectRange( v, ratioSelection, nbBins );
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
