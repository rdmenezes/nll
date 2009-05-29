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
   template <class type, class mapper>
	inline Image<type, mapper> extractChannel(const Image<type, mapper>& img, ui32 channel)
	{
		assert(channel < img.getNbComponents());
		assert(img.getBuf());
		if (!img.getBuf())
			return Image<type, mapper>();
		Image<type, mapper> i(img.sizex(), img.sizey(), 1);
		for (ui32 ny = 0; ny < img.sizey(); ++ny)
			for (ui32 nx = 0; nx < img.sizex(); ++nx)
				i(nx, ny, 0) = img(nx, ny, channel);
		return i;
	}

   /**
    @ingroup core
    @brief return a greyscale image of 1 component of an image.
    */
   template <class type, class mapper>
	inline void decolor(Image<type, mapper>& img)
	{
		if (img.getNbComponents() > 1 && img.getBuf())
		{
			Image<type, mapper> tmp(img.sizex(), img.sizey(), 1, false);
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
	template <class type, class mapper>
	inline void extend(Image<type, mapper>& img, ui32 nbc)
	{
		assert(img.getNbComponents() == 1); // "error: only 1 comp handled"
	
		Image<type, mapper> tmp(img.sizex(), img.sizey(), nbc, false);
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
   template <class type, class mapper>
	inline void addBorder(Image<type, mapper>& image, ui32 borderx, ui32 bordery)
	{
		Image<type, mapper> tmp(image.sizex() + borderx * 2, image.sizey() + bordery * 2, image.getNbComponents());
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
	template <class type, class mapper>
	inline void subBorder(Image<type, mapper>& image, ui32 borderx, ui32 bordery)
	{
		Image<type, mapper> tmp(image.sizex() - borderx * 2, image.sizey() - bordery * 2, image.getNbComponents(), false);
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
	template <class type, class mapper>
	inline void changeRGBtoBGR(Image<type, mapper>& image)
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
	template <class type, class mapper>
	inline void inversey(Image<type, mapper>& image)
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
	template <class type, class mapper>
	inline void extract(Image<type, mapper>& img, ui32 x1, ui32 y1, ui32 x2, ui32 y2)
	{
		assert(x1 <= x2 && x2 < img.sizex() &&
			    y1 <= y2 && y2 < img.sizey());
		Image<type, mapper> i(x2 - x1 + 1, y2 - y1 + 1, img.getNbComponents(), false);
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
	template <class type, class mapper>
	void cropVertical(Image<type, mapper>& img, f32 noiseRatio, f32 threshold = 15, ui32* p1x = 0, ui32* p2x = 0)
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
					if (img(x, y, c) > threshold)
					{
						++nbPixel;
						break;
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
		Image<type, mapper> result(x2 - x1 + 1, img.sizey(), img.getNbComponents(), false);
		for (i32 x = x1; x <= x2; ++x)
			for (ui32 y = 0; y < img.sizey(); ++y)
				for (ui32 c = 0; c < img.getNbComponents(); ++c)
					result(x - x1, y, c) = img (x, y, c);

		img = result;
	}

   /**
    @ingroup core
    @brief replace a specific color by another one.
    */
	template <class type, class mapper>
	void replaceColor(Image<type, mapper>& img, const type* col, const type* newcol)
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
	template <class type, class mapper>
	void crop(Image<type, mapper>& img, const type* background, ui32& outx1, ui32& outy1, ui32& outx2, ui32& outy2)
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
	template <class type, class mapper>
	void scale(Image<type, mapper>& img, f32 scalex, f32 scaley)
	{
		if (!(scalex > 0 && scaley > 0 && img.getBuf()))
			return;
		ui32 newx = (ui32)(scalex * img.sizex());
		ui32 newy = (ui32)(scaley * img.sizey());
		ui32 colorbyte = img.getNbComponents();

		Image<type, mapper> scaled(newx, newy, colorbyte);
		
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
}
}

# pragma warning( pop )

#endif
