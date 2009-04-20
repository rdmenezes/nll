#ifndef NLL_GABOR_H_
# define NLL_GABOR_H_

# include "gabor-filter.h"

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a facility that hold a bank of gabor filters. Compute an image filtered with it
    */
   template <class ImageType, class ImageMapper, class T>
   class GaborFilters
   {
   public:
      typedef core::Image<ImageType, ImageMapper>   Image;
      typedef core::Matrix<nll::f64>                Convolution;
      typedef std::vector<Convolution>              Convolutions;
      typedef core::Image<T, ImageMapper>           ImageOutput;

   public:
      /**
       @brief Construct an empy gabor bank
       */
      GaborFilters(){}

      /**
       @brief Construct a gabor bank from a list of descriptor
       */
      GaborFilters( const GaborFilterDescriptors& descs )
      {
         _gabors = computeGaborFilters<nll::f64, Convolution::IndexMapper>( descs );
      }

      /**
       @brief add a new filter to the bank
       */
      void addFilter( const GaborFilterDescriptor& desc )
      {
         // create a vector of 1 element to use the same interface // TODO change it!
         GaborFilterDescriptors descs;
         descs.push_back( desc );
         _gabors.push_back( computeGaborFilters<nll::f64, Convolution::IndexMapper>( descs )[ 0 ] );
      }

      /**
       @brief add a new filter to the bank
       */
      void addFilter( const Convolution& convo )
      {
         _gabors.push_back( convo );
      }

      /**
       @brief return the i th filter
       */
      const Convolution& at( ui32 index )
      {
         return _gabors[ index ];
      }

      /**
       @brief return the number of filters
       */
      ui32 size()
      {
         return static_cast<ui32>( _gabors.size() );
      }

      /**
       @brief return the i th filter
       */
      const Convolution& operator[]( ui32 index ){ return at( index ); }

      /**
       @brief return the set of filters
       */
      const Convolutions& getConvolutions()
      {
         return _gabors;
      }

      /**
       @brief convolve the image with all the filters, returns the concatenantion of the convolutions
       */
      ImageOutput convolve( const Image& i ) const
      {
         assert( _gabors.size() );
         assert( i.getNbComponents() == 1 );
         ImageOutput output( i.sizex(), i.sizey(), static_cast<ui32>( _gabors.size() ) );
         for ( ui32 n = 0; n < _gabors.size(); ++n )
         {
            Image conv( i );
            core::convolveBorder( conv, _gabors[ n ] );
            for ( ui32 ny = 0; ny < i.sizey(); ++ny )
               for ( ui32 nx = 0; nx < i.sizex(); ++nx )
                  output( nx, ny, n ) = conv( nx, ny, 0 );
         }
         return output;
      }

      /**
       @brief clear the filter bank
       */
      void clear()
      {
         _gabors.clear();
      }

      /**
       @brief write the bank to a stream
       */
      void write( std::ostream& o ) const
      {
         ui32 size = static_cast<ui32>( _gabors.size() );
         core::write<ui32>( size, o );
         for ( ui32 n = 0; n < size; ++n )
            _gabors[ n ].write( o );
      }

      /**
       @brief read the bank from a stream
       */
      void read( std::istream& i )
      {
         ui32 size = static_cast<ui32>( _gabors.size() );
         core::read<ui32>( size, i );
         for ( ui32 n = 0; n < size; ++n )
            _gabors[ n ].read( i );
      }

   private:
      Convolutions            _gabors;
   };
}
}

#endif
