#ifndef NLL_IMAGE_SNR_H_
# define NLL_IMAGE_SNR_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief compute the PSNR of 2 images

    @param signalDynamic max possible value of the pixel (char=256...)
    */
   template <class T, class Mapper, class Allocator>
   double psnr( const Image<T, Mapper, Allocator>& orig, const Image<T, Mapper, Allocator>& reconstructed, const T signalDynamic = Bound<T>::max )
   {
      assert( orig.sizex() == reconstructed.sizex() );
      assert( orig.sizey() == reconstructed.sizey() );
      assert( orig.getNbComponents() == reconstructed.getNbComponents() );

      double eqm = 0;
      for ( ui32 ny = 0; ny < orig.sizey(); ++ny )
         for ( ui32 nx = 0; nx < orig.sizex(); ++nx )
            // only computed on luminance values
            // TODO : bad cast
            eqm += ( generic_norm2<T*, f64>( (T*)orig.point( nx, ny ), (T*)reconstructed.point( nx, ny ), orig.getNbComponents() ) / orig.getNbComponents() );
      eqm /= orig.sizex() * orig.sizey(); 
      return 10 * log10( signalDynamic * signalDynamic / eqm );
   }
}
}

#endif
