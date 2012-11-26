/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
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
      MapperLutColor( size_t size, size_t components ) : _size( size ), _components( components ), _container( ( size + 1 ) * components, true )
      {
         _index = VectorIndex( size + 1 );
         for ( size_t n = 0; n < size; ++n )
         {
            _index[ n ] = &_container[ n * components ];
         }
         _index[ size ] = _index[ size - 1 ];
      }

      /**
       @brief Set value for an index.
       @param index the index where the value will be stored
       @param value a multidimentional value, that must have the same size than specified in constructor
              The value is copied.
       */
      void set( size_t index, const T* value )
      {
         for ( size_t n = 0; n < _components; ++n )
            _index[ index ][ n ] = value[ n ];
      }

      /**
       @brief return a pointer on the multidimensional value pointed by index.
       */
      const T* operator[]( size_t index ) const
      {
         return _index[ index ];
      }

      /**
       @brief returns the number of components of a point
       */
      size_t getNbComponents() const
      {
         return _components;
      }

      /**
       @brief Returns the number of indexes that can be adressed
       */
      size_t getSize() const
      {
         return _size;
      }

   private:
      size_t        _size;
      size_t        _components;
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
      set( size_t index, T* value )
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
      LookUpTransform( const TMapper& mapper, float minIntensity, float maxIntensity ) : _mapper( mapper ), _min( minIntensity ), _max( maxIntensity ), _interval( maxIntensity - minIntensity )
      {
         ensure( _interval > 0, "must be >0" );
         _ratio = static_cast<float>( _mapper.getSize() - 1 ) / _interval;
      }

      /**
       @brief Automatically detect the range of the LUT so that <ratioSelection> % of the voxels are selected
       */
      template <class Volume>
      void detectRange( const Volume& v, double ratioSelection, size_t nbBins = 256 )
      {
         {
            std::stringstream ss;
            ss << "LUT range detection: ratio= " << ratioSelection << " nbBins=" << nbBins;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

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

         {
            std::stringstream ss;
            ss << " min voxel= " << min << std::endl;
            ss << " max voxel= " << max << std::endl;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // do binning
         std::vector<size_t> bins( nbBins );
         const double range = ( static_cast<double>( max ) - static_cast<double>( min ) + 1 ) / nbBins;
         size_t nbVoxels = 0;
         for ( typename Volume::const_iterator it = v.begin(); it != v.end(); ++it )
         {
            const size_t bin = ( *it >= max ) ? ( nbBins - 1 ) : ( ( *it <= min ) ? ( 0 ) : ( static_cast<size_t>( ( *it - min ) / range ) ) );
            //const size_t bin = static_cast<size_t>( ( *it - min ) / range );
            ++bins[ bin ];
            ++nbVoxels;
         }

         {
            std::stringstream ss;
            ss << " histogram:" << std::endl;
            for ( size_t n = 0; n < nbBins; ++n )
            {
               ss << "  bin[" << n << "]=" << bins[ n ] << std::endl;
            }
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // select the range that select the specified ratio of voxel
         // sort the bins by size and greedily add bins to the selected bins
         //const size_t nbVoxels = v.size()[ 0 ] * v.size()[ 1 ] * v.size()[ 2 ];


         double selectedVoxels = 0;

         const size_t nbGaussians = 2;
         nll::algorithm::HistogramFitGaussian fitter;
         fitter.fit( bins, nbGaussians, 30, 1000 );

         {
            std::stringstream ss;
            ss << " gaussians found:" << std::endl;
            for ( size_t n = 0; n < nbGaussians; ++n )
            {
               ss << "  mean" << n << " =" << fitter.getGaussians()[ n ].mean[ 1 ] << std::endl;
            }
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         // always take the mean of the highest gaussian
         size_t gaussianIndex;
         if ( fitter.getGaussians()[ 1 ].mean[ 1 ] > fitter.getGaussians()[ 0 ].mean[ 1 ] )
         {
            gaussianIndex = 1;
         } else {
            gaussianIndex = 0;
         }
         int index = (int)fitter.getGaussians()[ gaussianIndex ].mean[ 1 ];
         index = NLL_BOUND( index, 0, (int)bins.size() - 1 );
         int inc1 = 0;
         int inc2 = 0;

         selectedVoxels += bins[ index ];
         while ( selectedVoxels / nbVoxels < ratioSelection )
         {
            if ( ( index - inc1 ) >= 0 )
            {
               selectedVoxels += bins[ index - inc1 ];
               ++inc1;
            }

            if ( ( index + inc2 ) < bins.size() )
            {
               selectedVoxels += bins[ index + inc2 ];
               ++inc2;
            }
         }

         _min = static_cast<float>( min + ( index - inc1 ) * range );
         _max = static_cast<float>( min + ( index + inc2 ) * range );
         _interval = _max - _min;
         _ratio = static_cast<float>( _mapper.getSize() - 1 ) / _interval;

         {
            std::stringstream ss;
            ss << " LUT min=" << _min << " max=" << _max << std::endl;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }
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
         if ( value >= _max )
            return (ui32)_mapper.getSize() - 1;
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
# if NLL_INSTRUCTION_SET >= 2
      template <>
      void transformToIndex<float*, ui32*>( float* start, float* end, ui32* output ) const
      {
         transformToIndex<const float*>( start, end, output );
      }

      template <>
      void transformToIndex<const float*, ui32*>( const float* start, const float* end, ui32* output ) const
      {
         assert( start < end );
         const size_t nbVectorizedLoops = ( end - start ) / 4;
         const float* endVectorized = start + nbVectorizedLoops * 4;

         vectorized::Vec4f min( _min );
         vectorized::Vec4f max( _max );
         vectorized::Vec4f ratio( _ratio );

         //( value - _min ) * _ratio
         for( ; start != endVectorized; start += 4, output += 4 )
         {
            vectorized::Vec4f vals;
            vals.load( start );

            vectorized::Vec4fb lower = vals < min;
            vectorized::Vec4fb higher = vals > max;

            const bool is1 = vectorized::horizontal_or( lower );
            const bool is2 = vectorized::horizontal_or( higher );
            if ( is1 || is2 )
            {
               // if one is outside the domain, don't bother...
               output[ 0 ] = transformToIndex( start[ 0 ] );
               output[ 1 ] = transformToIndex( start[ 1 ] );
               output[ 2 ] = transformToIndex( start[ 2 ] );
               output[ 3 ] = transformToIndex( start[ 3 ] );
               continue;
            } else {
               // all are in range, so continue...
               vals = ( vals - min ) * ratio;
               vectorized::Vec4i indexes = vectorized::truncatei( vals );
               indexes.store( output );
            }
         }

         for ( ; start != end; ++start )
         {
            *output++ = transformToIndex( *start );
         }
      }
# endif

      /**
       @brief Returns the number of mapper indexes
       */
      size_t getSize() const
      {
         return _mapper.getSize();
      }

      /**
       @brief Returns the number of components each value in the LUT has
       */
      size_t getNbComponents() const
      {
         return _mapper.getNbComponents();
      }

      /**
       @brief Set a new value for the specified index. Internally, the value is copied.
       */
      void set( size_t index, const T* value )
      {
         _mapper.set( index, value );
      }

      value_type getMin() const
      {
         return _min;
      }

      value_type getMax() const
      {
         return _max;
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

    This is using <LookUpTransform> as a backend and adding some common utility functions
    */
   class LookUpTransformWindowingRGB
   {
   public:
      typedef float                         value_type;

   protected:
      typedef MapperLutColor<value_type>  LutMapper;
      typedef LookUpTransform<value_type, MapperLutColor<value_type> > Lut;

   public:
      LookUpTransformWindowingRGB( float minIntensity, float maxIntensity, size_t size, size_t nbComponents = 3 ) :
         _lut( LutMapper( size, nbComponents ), minIntensity, maxIntensity )
      {}

      void reset( float minIntensity, float maxIntensity, size_t size, size_t nbComponents = 3 )
      {
         _lut = Lut( LutMapper( size, nbComponents ), minIntensity, maxIntensity );
      }

      const value_type* transform( float value ) const
      {
         return _lut.transform( value );
      }

      value_type getMin() const
      {
         return _lut.getMin();
      }

      value_type getMax() const
      {
         return _lut.getMax();
      }


      /**
       @brief Do the same than <code>transform</code> but instead do it on a sequence and returns
              the index in the output iterator. In the case InputIterator=float* and
              OutputIterator=size_t*, it will be optimized with SSE & multithreading

              On a dualcore-SSE, 10x increase compare to the default implementation
       */
      template <class InputIterator, class OutputIterator>
      void transformToIndex( InputIterator start, InputIterator end, OutputIterator output )
      {
         _lut.transformToIndex( start, end, output );
      }

      ui32 transformToIndex( value_type val ) const
      {
         return _lut.transformToIndex( val );
      }

      size_t getSize() const
      {
         return _lut.getSize();
      }

      size_t getNbComponents() const
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

      void createGreyscale( value_type highestValue = 256.0f )
      {
         core::Buffer1D<value_type> vals( _lut.getNbComponents() );
         const ui32 nb = static_cast<ui32>( _lut.getSize() );
         for ( ui32 n = 0; n < nb; ++n )
         {
            for ( size_t i = 0; i < _lut.getNbComponents(); ++i )
            {
               vals[ i ] = static_cast<value_type>( highestValue / _lut.getSize() * n );
            }
            set( n, vals.getBuf() );
         }
      }

      void createGreyscaleInverted()
      {
         core::Buffer1D<value_type> vals( _lut.getNbComponents() );
         const ui32 nb = static_cast<ui32>( _lut.getSize() );
         for ( ui32 n = 0; n < nb; ++n )
         {
            for ( size_t i = 0; i < _lut.getNbComponents(); ++i )
            {
               vals[ i ] = static_cast<value_type>( 256.0 / _lut.getSize() * ( _lut.getSize() - n - 1 ) );
            }
            set( n, vals.getBuf() );
         }
      }

      template <class Volume>
      void detectRange( const Volume& v, double ratioSelection, size_t nbBins = 256 )
      {
         _lut.detectRange( v, ratioSelection, nbBins );
      }

      /**
       @brief Create a gaussian spike, useful to display the edges of the volumes
       @param stdDevRatio the stddev to be used, sepficied relative to the lut size (e.g., 0.2 means the stddev will be 0.2 * lutSize)
       @param meanRatio the mean to be used, sepficied relative to the lut size (e.g., with 0.5, the means will be 0.2 * lutSize)
       */
      void createColorVolcano( const value_type* baseColor, double stdDevRatio = 0.2, double meanRatio = 0.5 )
      {
         // create the gaussian function
         core::Matrix<double> variance( 1, 1 );
         variance( 0, 0 ) = core::sqr( _lut.getSize() * stdDevRatio );
         const core::Buffer1D<double> mean = core::make_buffer1D<double>( _lut.getSize() * meanRatio );
         core::ProbabilityDistributionFunctionGaussian<double> pdf( variance, mean );

         const double scaling = 1 / pdf.eval( mean ); // we want the mean to be the max

         // create the LUT
         core::Buffer1D<value_type> vals( _lut.getNbComponents() );
         const ui32 nb = static_cast<ui32>( _lut.getSize() );
         for ( ui32 n = 0; n < nb; ++n )
         {
            const double ratio = pdf.eval( core::make_buffer1D<double>( (double)n ) );
            for ( size_t i = 0; i < _lut.getNbComponents(); ++i )
            {
               vals[ i ] = static_cast<value_type>( scaling * ratio * baseColor[ i ] );
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
         const ui32 nb = static_cast<ui32>( _lut.getSize() );
         for ( ui32 n = 0; n < nb; ++n )
         {
            for ( size_t i = 0; i < _lut.getNbComponents(); ++i )
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
