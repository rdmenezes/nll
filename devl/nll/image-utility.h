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

#ifndef NLL_IMAGE_UTILITY_H_
# define NLL_IMAGE_UTILITY_H_

# include "image.h"

# pragma warning( push )
# pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data


namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Compare 2 pixels.
    @return true if the 2 pixels are sematically equal
    */
   template <class type>
   bool isColorEqual(const type* t1, const type* t2, ui32 nbComponents)
   {
      for ( ui32 n = 0; n < nbComponents; ++n )
         if ( !equal( t1[ n ], t2[ n ] ) )
            return false;
      return true;
   }

   /**
    @ingroup core
    @brief extract a specified channel of an image.
    */
   template <class type, class mapper, class allocator>
	inline Image<type, mapper, allocator> extractChannel(const Image<type, mapper, allocator>& img, ui32 channel)
	{
		assert(channel < img.getNbComponents());
		assert(img.getBuf());
		if (!img.getBuf())
			return Image<type, mapper, allocator>();
		Image<type, mapper, allocator> i(img.sizex(), img.sizey(), 1, false, img.getAllocator());
		for (ui32 ny = 0; ny < img.sizey(); ++ny)
			for (ui32 nx = 0; nx < img.sizex(); ++nx)
				i(nx, ny, 0) = img(nx, ny, channel);
		return i;
	}

   /**
    @ingroup core
    @brief return a greyscale image of 1 component of an image.
    */
   template <class type, class mapper, class allocator>
	inline void decolor(Image<type, mapper, allocator>& img)
	{
		if (img.getNbComponents() > 1 && img.getBuf())
		{
			Image<type, mapper, allocator> tmp(img.sizex(), img.sizey(), 1, false, img.getAllocator());
			for (ui32 y = 0; y < img.sizey(); ++y)
				for (ui32 x = 0; x < img.sizex(); ++x)
				{
					f64 tt = 0;
					type* t1 = img.point(x, y);
					for (ui32 c = 0; c < img.getNbComponents(); ++c)
						tt += t1[c];
					tmp(x, y, 0) = tt / img.getNbComponents();
				}
			img = tmp;
		}
	}

   /**
    @ingroup core
    @brief extend to a specified number of component a 1-component image.

    Replicate the value of the pixel for each component.
    */
	template <class type, class mapper, class allocator>
	inline void extend(Image<type, mapper, allocator>& img, ui32 nbc)
	{
		ensure( img.getNbComponents() == 1, "it can only be done with images having only one component" ); // "error: only 1 comp handled"
	
		Image<type, mapper, allocator> tmp(img.sizex(), img.sizey(), nbc, false, img.getAllocator());
		for (ui32 y = 0; y < img.sizey(); ++y)
			for (ui32 x = 0; x < img.sizex(); ++x)
			{
				type v = *img.point(x, y);
				type* t2 = tmp.point(x, y);
				for (ui32 c = 0; c < nbc; ++c)
					t2[c] = v;
			}
		img = tmp;
	}


   /**
    @ingroup core
    @brief add a specified border (in pixels) of an image.
    */
   template <class type, class mapper, class allocator>
	inline void addBorder(Image<type, mapper, allocator>& image, ui32 borderx, ui32 bordery)
	{
		Image<type, mapper, allocator> tmp(image.sizex() + borderx * 2, image.sizey() + bordery * 2, image.getNbComponents(), true, image.getAllocator());
		for (ui32 y = 0; y < image.sizey(); ++y)
			for (ui32 x = 0; x < image.sizex(); ++x)
			{
				type* t1 = tmp.point(x + borderx, y + bordery);
				type* t2 = image.point(x, y);
				for (ui32 c = 0; c < image.getNbComponents(); ++c)
					t1[c] = t2[c];
			}
		image = tmp;
	}

	
   /**
    @ingroup core
    @brief remove a specified border (in pixels) of an image.
    */
	template <class type, class mapper, class allocator>
	inline void subBorder(Image<type, mapper, allocator>& image, ui32 borderx, ui32 bordery)
	{
		Image<type, mapper, allocator> tmp(image.sizex() - borderx * 2, image.sizey() - bordery * 2, image.getNbComponents(), false, image.getAllocator());
		for (ui32 y = 0; y < tmp.sizey(); ++y)
			for (ui32 x = 0; x < tmp.sizex(); ++x)
			{
				type* t1 = tmp.point(x, y);
				type* t2 = image.point(x + borderx, y + bordery);
				for (ui32 c = 0; c < tmp.getNbComponents(); ++c)
					t1[c] = t2[c];
			}
		image = tmp;
	}

   /**
    @ingroup core
    @brief invert the components of an image.
    */
	template <class type, class mapper, class allocator>
	inline void changeRGBtoBGR(Image<type, mapper, allocator>& image)
	{
		for (ui32 y = 0; y < image.sizey(); ++y)
			for (ui32 x = 0; x < image.sizex(); ++x)
			{
				type* t1 = image.point(x, y);
				for (ui32 c = 0; c < image.getNbComponents() / 2; ++c)
					std::swap(t1[c], t1[image.getNbComponents() - c - 1]);
			}
	}

   /**
    @ingroup core
    @brief y-inverse an image.
    */
	template <class type, class mapper, class allocator>
	inline void inversey(Image<type, mapper, allocator>& image)
	{
		for (ui32 y = 0; y < image.sizey() / 2; ++y)
			for (ui32 x = 0; x < image.sizex(); ++x)
			{
				type* t1 = image.point(x, y);
				type* t2 = image.point(x, image.sizey() - 1 - y);
				for (ui32 c = 0; c < image.getNbComponents(); ++c)
					std::swap(t1[c], t2[c]);
			}
	}

   /**
    @ingroup core
    @brief extract a subpart of an image.
    */
	template <class type, class mapper, class allocator>
	inline void extract(Image<type, mapper, allocator>& img, ui32 x1, ui32 y1, ui32 x2, ui32 y2)
	{
		assert(x1 <= x2 && x2 < img.sizex() &&
			    y1 <= y2 && y2 < img.sizey());
		Image<type, mapper, allocator> i(x2 - x1 + 1, y2 - y1 + 1, img.getNbComponents(), false, img.getAllocator());
		for (ui32 x = x1; x <= x2; ++x)
			for (ui32 y = y1; y <= y2; ++y)
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
					i(x - x1, y - y1, c) = img(x, y, c);
		img = i;
	}

   /**
    @ingroup core
    @brief vertically crop an image.

    x-crop the image : used for example on a noisy image, we can remove a specific proportion of the image.

    @param p1x export the first left cropped point
    @param p2x export the first right cropped point
    */
	template <class type, class mapper, class allocator>
	void cropVertical(Image<type, mapper, allocator>& img, f32 noiseRatio, f32 threshold = 15, ui32* p1x = 0, ui32* p2x = 0)
	{
		if (p1x)
			*p1x = 0;
		if (p2x)
			*p2x = 0;

		// compute the number of actual pixels
		ui32 nbPixel = 0;
		for (ui32 x = 0; x < img.sizex(); ++x)
			for (ui32 y = 0; y < img.sizey(); ++y)
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
            {
               const type t = img(x, y, c);
					if (t > threshold)
					{
						++nbPixel;
						break;
					}
            }
		f32 nbPixelSkipped = noiseRatio * nbPixel;

		// scan from the left
		i32 x1;
		ui32 n = 0;
		for (x1 = 0; x1 < static_cast<i32>(img.sizex()); ++x1)
		{
			for (ui32 y = 0; y < img.sizey(); ++y)
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
					if (img(x1, y, c) > threshold)
					{
						++n;
						break;
					}

			if (n > nbPixelSkipped)
				break;
		}

		// scan form the right
		i32 x2;
		n = 0;
		for (x2 = img.sizex() - 1; x2 >= 0; --x2)
		{
			for (ui32 y = 0; y < img.sizey(); ++y)
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
					if (img(x2, y, c) > threshold)
					{
						++n;
						break;
					}

			if (n > nbPixelSkipped)
				break;
		}

		if (x2 <= x1)
			return;
		if (p1x)
			*p1x = x1;
		if (p2x)
			*p2x = x2;

		// crop the image
		Image<type, mapper, allocator> result(x2 - x1 + 1, img.sizey(), img.getNbComponents(), false, img.getAllocator());
		for (i32 x = x1; x <= x2; ++x)
			for (ui32 y = 0; y < img.sizey(); ++y)
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
					result(x - x1, y, c) = img (x, y, c);

		img = result;
	}

      /**
    @ingroup core
    @brief horizontally crop an image.

    y-crop the image : used for example on a noisy image, we can remove a specific proportion of the image.

    @param p1y export the first left cropped point
    @param p2y export the first right cropped point
    */
	template <class type, class mapper, class allocator>
	void cropHorizontal(Image<type, mapper, allocator>& img, f32 noiseRatio, f32 threshold = 15, ui32* p1y = 0, ui32* p2y = 0)
	{
		if (p1y)
			*p1y = 0;
		if (p2y)
			*p2y = 0;

		// compute the number of actual pixels
		ui32 nbPixel = 0;
		for (ui32 x = 0; x < img.sizex(); ++x)
			for (ui32 y = 0; y < img.sizey(); ++y)
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
            {
               const type t = img(x, y, c);
					if (t > threshold)
					{
						++nbPixel;
						break;
					}
            }
		f32 nbPixelSkipped = noiseRatio * nbPixel;

		// scan from the left
		i32 y1;
		ui32 n = 0;
		for (y1 = 0; y1 < static_cast<i32>(img.sizey()); ++y1)
		{
			for (ui32 x = 0; x < img.sizex(); ++x)
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
					if (img(x, y1, c) > threshold)
					{
						++n;
						break;
					}

			if (n > nbPixelSkipped)
				break;
		}

		// scan form the right
		i32 y2;
		n = 0;
		for (y2 = img.sizey() - 1; y2 >= 0; --y2)
		{
			for (ui32 x = 0; x < img.sizex(); ++x)
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
					if (img(x, y2, c) > threshold)
					{
						++n;
						break;
					}

			if (n > nbPixelSkipped)
				break;
		}

		if (y2 <= y1)
			return;
		if (p1y)
			*p1y = y1;
		if (p2y)
			*p2y = y2;

		// crop the image
		Image<type, mapper, allocator> result(img.sizex(), y2 - y1 + 1, img.getNbComponents(), false, img.getAllocator());
		for (i32 y = y1; y <= y2; ++y)
			for (ui32 x = 0; x < img.sizex(); ++x)
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
					result(x, y - y1, c) = img (x, y, c);

		img = result;
	}

   /**
    @ingroup core
    @brief replace a specific color by another one.
    */
	template <class type, class mapper, class allocator>
	void replaceColor(Image<type, mapper, allocator>& img, const type* col, const type* newcol)
	{
		if (!img.getBuf() || !img.sizex() || !img.sizey())
			return;
		assert(col && newcol);
		for (ui32 x = 0; x <= img.sizex(); ++x)
			for (ui32 y = 0; y < img.sizey(); ++y)
			{
				type* p = img.point(x, y);
				if (isColorEqual(p, col, img.getNbComponents()))
					for (ui32 c = 0; c < img.getNbComponents(); ++c)
						p[c] = newcol[c];
			}
	}


   /**
    @ingroup core
    @brief crop an image and return the cropped area.
    */
	template <class type, class mapper, class allocator>
	void crop(Image<type, mapper, allocator>& img, const type* background, ui32& outx1, ui32& outy1, ui32& outx2, ui32& outy2)
	{
		if (!img.getBuf() || !img.sizex() || !img.sizey())
			return;
		ui32 minx = img.sizex() - 1;
		ui32 miny = img.sizey() - 1;
		ui32 maxx = 0;
		ui32 maxy = 0;

		for (ui32 y = 0; y < img.sizey(); ++y)
			for (ui32 x = 0; x < img.sizex(); ++x)
			{
            if ( isColorEqual( background, img.point(x, y), img.getNbComponents() ) )
               continue;
				if (x < minx)
					minx = x;
				if (x > maxx)
					maxx = x;
				if (y < miny)
					miny = y;
				if (y > maxy)
					maxy = y;
			}
		if (minx > maxx || miny > maxy)
			return;
		extract(img, minx, miny, maxx, maxy);
		outx1 = minx;
		outx2 = maxx;
		outy1 = miny;
		outy2 = maxy;
	}


   /**
    @ingroup core
    @brief resacle an image with no interpolation.
    */
	template <class type, class mapper, class allocator>
	void scale(Image<type, mapper, allocator>& img, f32 scalex, f32 scaley)
	{
		if (!(scalex > 0 && scaley > 0 && img.getBuf()))
			return;
		ui32 newx = (ui32)(scalex * img.sizex());
		ui32 newy = (ui32)(scaley * img.sizey());
		ui32 colorbyte = img.getNbComponents();

		Image<type, mapper, allocator> scaled(newx, newy, colorbyte, true, img.getAllocator());
		
		f32 dx = 1 / scalex;
		f32 dy = 1 / scaley;
		for (f32 y = 0; y < newy; y += 1)
			for (f32 x = 0; x < newx; x += 1)
			{
				type* psrc = img.point(static_cast<ui32>(x * dx), static_cast<ui32>(y * dy));
				type* pdst = scaled.point(static_cast<ui32>(x), static_cast<ui32>(y));
				for (ui8 c = 0; c < colorbyte; ++c)
					pdst[c] = psrc[c];
			}
		img = scaled;
	}

   /**
    @ingroup core
    @brief Computes the barycentre of the image. Only handle UI8 greyscale type
    */
   template <class Mapper, class Allocator>
   vector2d computeBarycentre( const Image<ui8, Mapper, Allocator>& i )
   {
      ensure( i.getNbComponents() == 1, "only grey level can be used" );

      double weight = 0;
      double xm = 0;
      double ym = 0;
      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            double w = static_cast<double>( i( x, y, 0 ) ) / 255;
            xm += x * w;
            ym += y * w;
            weight += w;
         }
      }

      if ( weight > 0 )
         return vector2d( xm / weight, ym / weight );
      return vector2d( i.sizex() / 2, i.sizey() / 2 );
   }

   /**
    @ingroup core
    @brief Center a greyscaly image
    */
   template <class Mapper, class Allocator>
   void centerImage( Image<ui8, Mapper, Allocator>& i )
   {
      ensure( i.getNbComponents() == 1, "only grey level can be used" );

      vector2d t = computeBarycentre( i );
      vector2ui tfm( - t[ 0 ] + i.sizex() / 2, - t[ 1 ] + i.sizey() / 2 );

      Image<ui8, Mapper, Allocator> ii( i.sizex(), i.sizey(), 1 );
      for ( ui32 x = 0; x < ii.sizex(); ++x )
         for ( ui32 y = 0; y < ii.sizey(); ++y )
         {
            ui32 posx = x - tfm[ 0 ];
            ui32 posy = y - tfm[ 1 ];
            if ( posx < i.sizex() && posy < i.sizey() )
            {
               ii( x, y, 0 ) = i( x - tfm[ 0 ], y - tfm[ 1 ], 0 );
            }
         }

      i = ii;
   }

   template <class Image, class Color>
   void bresham( Image& i, const vector2i a1, const vector2i a2, Color color )
   {
      vector2i p1 = a1;
      vector2i p2 = a2;
      const bool steep = abs( p2[ 1 ] - p1[ 1 ] ) > abs( p2[ 0 ] - p1[ 0 ] );
      if ( steep )
      {
         std::swap( p1[ 0 ], p1[ 1 ] );
         std::swap( p2[ 0 ], p2[ 1 ] );
      }
      if ( p1[ 0 ] > p2[ 0 ] )
      {
         std::swap( p1[ 0 ], p2[ 0 ] );
         std::swap( p1[ 1 ], p2[ 1 ] );
      }
      int deltax = p2[ 0 ] - p1[ 0 ];
      int deltay = abs( p2[ 1 ] - p1[ 1 ] );
      int error = deltax / 2;
      int ystep = ( p1[ 1 ] < p2[ 1 ] ) ? 1 : -1;
      int y = p1[ 1 ];

      if ( steep )
      {
         typename Image::DirectionalIterator it = i.getIterator( y, p1[ 0 ], 0 );
         for ( int x = p1[ 0 ]; x <= p2[ 0 ]; ++x )
         {
            for ( ui32 col = 0; col < i.getNbComponents(); ++col )
            {
               it.pickcol( col ) = color[ col ];
            }
            error -= deltay;
            if ( error < 0 )
            {
               it.addx( ystep );
               error = error + deltax;
            }
            it.addy();
         }
      } else {
         typename Image::DirectionalIterator it = i.getIterator( p1[ 0 ], y, 0 );
         for ( int x = p1[ 0 ]; x <= p2[ 0 ]; ++x )
         {
            for ( ui32 col = 0; col < i.getNbComponents(); ++col )
            {
               it.addcol();
               *it = color[ col ];
            }
            error -= deltay;
            if ( error < 0 )
            {
               it.addy( ystep );
               error = error + deltax;
            }
         }
      }
   }

}
}

# pragma warning( pop )

#endif
