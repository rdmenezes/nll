#ifndef NLL_IMAGE_CONVOLUTION_H_
# define NLL_IMAGE_CONVOLUTION_H_

# include "image.h"

namespace nll
{
namespace core
{
   /// 32 bits convolution
   typedef Matrix<f32> Convolutionf;

   /**
    @ingroup core
    @brief build a 3x3 gaussian convolution
   */
   inline Convolutionf buildGaussian()
	{
		Convolutionf convolution(3, 3);
		
		for (ui32 n = 0; n < 9; ++n)
			convolution[n] = 1.0f / 9.0f;
		return convolution;
	}

   /**
    @ingroup core
    @brief build a 35x35 gaussian convolution
   */
	inline Convolutionf buildGaussian35x35()
	{
		Convolutionf convolution(35, 35);
		
		for (ui32 n = 0; n < 35*35; ++n)
			convolution[n] = 1.0f / (35*35);
		return convolution;
	}

   /**
    @ingroup core
    @brief convolve an image
   */
	template <class type, class mapper, class allocator, class Convolution>
	void convolve(Image<type, mapper, allocator>& img, const Convolution& convolution)
	{
		i32 midx = convolution.sizex() / 2;
		i32 midy = convolution.sizey() / 2;
	
		Image<type, mapper, allocator> tmp(img.sizex(), img.sizey(), img.getNbComponents(), false, img.getAllocator());

		for (ui32 y = midy; y < img.sizey() - midy; ++y)
			for (ui32 x = midx; x < img.sizex() - midx; ++x)
			{
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
				{
					f64 tt = 0;
					ui32 ddx = x - midx;
					ui32 ddy = y - midy;
					for (ui32 dy = 0; dy < convolution.sizey(); ++dy)
						for (ui32 dx = 0; dx < convolution.sizex(); ++dx)
							tt += convolution(dx, dy) * img(ddx + dx , ddy + dy, c);
               tt = NLL_BOUND(tt, (type)Bound<type>::min, (type)Bound<type>::max);
					tmp(x, y, c) = static_cast<type> (tt);
				}
			}
		img = tmp;
	}

   /**
    @ingroup core
    @brief convolve an image adding a null border of the size of the convolution
   */
	template <class type, class mapper, class allocator, class Convolution>
	void convolveBorder(Image<type, mapper, allocator>& img, const Convolution& convolution)
	{
		addBorder(img, convolution.sizex(), convolution.sizey());
		convolve(img, convolution);
		subBorder(img, convolution.sizex(), convolution.sizey());
	}
}
}

#endif
