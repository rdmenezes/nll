#ifndef NLL_IMAGE_RESAMPLE_H_
# define NLL_IMAGE_RESAMPLE_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief resample an image using a specific interpolator

    @param Interpolator interpolator used for resampling
    */
   template <class T, class IMapper, class Interpolator>
   void rescale( Image<T, IMapper>& img, ui32 newSizeX, ui32 newSizeY )
   {
      f64 dxsize = static_cast<f64> ( img.sizex() - 0 ) / newSizeX;
		f64 dysize = static_cast<f64> ( img.sizey() - 0 ) / newSizeY;
      Image<T, IMapper> i( newSizeX, newSizeY, img.getNbComponents() );

      Interpolator interpolator( img );
	   for ( ui32 y = 0; y < newSizeY; ++y )
         for ( ui32 x = 0; x < newSizeX; ++x )
            for ( ui32 c = 0; c < img.getNbComponents(); ++c )
               i( x, y, c ) = static_cast<T> ( interpolator.interpolate( (x + 0) * dxsize, (y + 0) * dysize, c ) );
      img = i;
   }

   /**
    @ingroup core
    @brief resample an image using a bilinear interpolation.
    */
   template <class T, class IMapper>
   inline void rescaleBilinear( Image<T, IMapper>& img, ui32 newSizeX, ui32 newSizeY )
   {
      rescale<T, IMapper, InterpolatorLinear2D<T, IMapper> >( img, newSizeX, newSizeY );
   }

   /**
    @ingroup core
    @brief resample an image using a nearest neighbor interpolation.
    */
   template <class T, class IMapper>
   inline void rescaleNearestNeighbor( Image<T, IMapper>& img, ui32 newSizeX, ui32 newSizeY )
   {
      rescale<T, IMapper, InterpolatorNearestNeighbor2D<T, IMapper> >( img, newSizeX, newSizeY );
   }

   /**
    @ingroup core
    @brief resample an image a weighted grid.
    
    The resampled image is the mean of all pixels in a specific cell of this grid.
    */
   template <class T, class IMapper>
   void rescaleFast( Image<T, IMapper>& img, ui32 newSizeX, ui32 newSizeY )
   {
      Image<T, IMapper> i( newSizeX, newSizeY, img.getNbComponents() );
		f64 dxsize = static_cast<f64> ( img.sizex() ) / newSizeX;
		f64 dysize = static_cast<f64> ( img.sizey() ) / newSizeY;

		assert( dxsize * dysize ); // "error: image too small"
      f64 divsize = ( ( ( dxsize < 1 ) ? 1 : dxsize ) *
                      ( ( dysize < 1 ) ? 1 : dysize ) );
      for ( ui32 c = 0; c < img.getNbComponents(); ++c )
		   for ( ui32 y = 0; y < newSizeY; ++y )
            for ( ui32 x = 0; x < newSizeX; ++x )
			   {
				   f64 val = 0;
				   for ( ui32 dx = 0; dx < dxsize; ++dx )
					   for ( ui32 dy = 0; dy < dysize; ++dy )
						   val += img(static_cast<ui32>( x * dxsize )  + dx, static_cast<ui32>( y * dysize ) + dy, c );
               val /= divsize;
               i( x, y, c ) = static_cast<T>( NLL_BOUND( val, ( T )Bound<T>::min, ( T )Bound<T>::max ) );
			   }
		img = i;
   }
}
}
#endif
