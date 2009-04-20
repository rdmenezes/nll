#ifndef NLL_GABOR_FILTER_H_
# define NLL_GABOR_FILTER_H_

# include "types.h"
# include "matrix.h"

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a gabor filter (frequency, kernel size, orientation in radian)
    */
   struct   GaborFilterDescriptor
   {
	   f32	frequency;
	   f32   angle;
	   f32   size;
	   GaborFilterDescriptor( f32 f, f32 a, f32 s ) : frequency( f ), angle( a ), size( s ){}
   };

   typedef std::vector<GaborFilterDescriptor>	GaborFilterDescriptors;

   /**
    @ingroup algorithm
    @brief Compute a convolution from the gabor filter specification
    */
   template <class type, class IMapper>
   inline core::Matrix<type, IMapper> cosineGaborFilters( const GaborFilterDescriptor& gabor )
   {
	   typedef core::Matrix<type, IMapper> Convolution;

	   Convolution conv( static_cast<ui32>( gabor.size ), static_cast<ui32>( gabor.size ) );

	   f32 gamma = static_cast<f32>( gabor.size ) / 5.36f;
	   f32 cost = cos( gabor.angle );
	   f32 sint = sin( gabor.angle );
	   for (ui32 ny = 0; ny < gabor.size; ++ny)
		   for (ui32 nx = 0; nx < gabor.size; ++nx)
		   {
			   f32 xc = static_cast<f32>( nx ) - static_cast<f32>( gabor.size ) / 2;
			   f32 yc = static_cast<f32>( ny ) - static_cast<f32>( gabor.size ) / 2;

			   conv(nx, ny) = static_cast<f32>( exp(-(xc * xc + yc * yc) / (2 * gamma * gamma))
				               / ( sqrt( 2 * core::PI * gamma * gamma ) )
				               * cos( 2 * core::PI * gabor.frequency * ( xc * cost + yc * sint ) ) );
		   }
	   return conv;
   }

   /**
    @ingroup algorithm
    @brief Compute a set of filters from their specification
    */
   template <class type, class IMapper>
   inline std::vector<core::Matrix<type, IMapper> > computeGaborFilters(const GaborFilterDescriptors& descs)
   {
	   typedef core::Matrix<type, IMapper> Convolution;
	   typedef std::vector<Convolution> Convolutions;
	   Convolutions convs;
	   for ( ui32 n = 0; n < descs.size(); ++n )
		   convs.push_back(cosineGaborFilters<type, IMapper>( descs[ n ] ) );
	   return convs;
   }
}
}

#endif
