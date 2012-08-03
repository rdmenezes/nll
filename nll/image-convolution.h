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
		
		for (size_t n = 0; n < 9; ++n)
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
		
		for (size_t n = 0; n < 35*35; ++n)
			convolution[n] = 1.0f / (35*35);
		return convolution;
	}

   /**
    @ingroup core
    @brief convolve an image

    Note tha the border convolution.size() / 2 will be removed
   */
	template <class type, class mapper, class allocator, class Convolution>
	Image<type, mapper, allocator> convolve( const Image<type, mapper, allocator>& img, const Convolution& convolution )
	{
		i32 midx = static_cast<i32>( convolution.sizex() ) / 2;
		i32 midy = static_cast<i32>( convolution.sizey() ) / 2;
	
		Image<type, mapper, allocator> tmp(img.sizex(), img.sizey(), img.getNbComponents(), false, img.getAllocator());

		for (size_t y = midy; y < img.sizey() - midy; ++y)
			for (size_t x = midx; x < img.sizex() - midx; ++x)
			{
				for (size_t c = 0; c < img.getNbComponents(); ++c)
				{
					f64 tt = 0;
					size_t ddx = x - midx;
					size_t ddy = y - midy;
					for (size_t dy = 0; dy < convolution.sizey(); ++dy)
						for (size_t dx = 0; dx < convolution.sizex(); ++dx)
							tt += convolution(dx, dy) * img(ddx + dx , ddy + dy, c);
               tt = NLL_BOUND(tt, std::numeric_limits<type>::min(), std::numeric_limits<type>::max());
					tmp(x, y, c) = static_cast<type> (tt);
				}
			}
		return tmp;
	}

   /**
    @ingroup core
    @brief convolve an image adding a null border of the size of the convolution
   */
	template <class type, class mapper, class allocator, class Convolution>
	Image<type, mapper, allocator> convolveBorder( const Image<type, mapper, allocator>& img, const Convolution& convolution )
	{
      Image<type, mapper, allocator> i;
		i = addBorder(img, convolution.sizex(), convolution.sizey() );
		i = convolve( i, convolution );
		i = subBorder( i, convolution.sizex(), convolution.sizey() );
      return i;
	}
}
}

#endif
