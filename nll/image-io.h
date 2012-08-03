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

#ifndef NLL_IMAGE_IO_H_
# define NLL_IMAGE_IO_H_

# include <iostream>
# include "image.h"

# pragma warning( push )
# pragma warning( disable:4127 ) // conditional expression is constant

namespace nll
{
namespace core
{
   #define INV2(n)(((n & 0x0F) << 4) | ((n & 0xF0) >> 4))
   #define TO1(n)	(n & 0xFF)
   #define TO2(n)	((n & 0xFF00) >> 8)
   #define TO3(n)	((n & 0xFF0000) >> 16)
   #define TO4(n)	((n & 0xFF000000) >> 24)

   inline static void write4(i8* buf, size_t v)
   {
	   *buf		= (i8)TO1(v);
	   *(buf + 1)	= (i8)TO2(v);
	   *(buf + 2)	= (i8)TO3(v);
	   *(buf + 3)	= (i8)TO4(v);
   }

   inline static void write2(i8* buf, size_t v)
   {
	   *buf		= (ui8)TO1(v);
	   *(buf + 1)	= (ui8)TO2(v);
   }

   /**
    @ingroup core
    @brief write a BMP, image internal format is BGR
    */
   template <class T, class Mapper, class Alloc>
   bool writeBmp( const Image<T, Mapper, Alloc>& i, std::ostream& f )
   {
      assert( IsNativeType<T>::value );
      size_t size = i.sizex() * i.sizey() * i.getNbComponents() + 54;
		i8 buf[54];
		memset(buf, 0, 54);
		write2(buf, 19778);
		write4(buf + 2, size);
		write4(buf + 6, 0);
		write4(buf + 10, 54);
		write4(buf + 14, 40);
		write4(buf + 18, i.sizex());
		write4(buf + 22, i.sizey());
		write2(buf + 26, 1);
		write2(buf + 28, i.getNbComponents() * 8);
		write4(buf + 30, 0);
		write4(buf + 34, i.sizex() * i.sizey() * i.getNbComponents());
		write4(buf + 38, 0);
		write4(buf + 42, 0);
		write4(buf + 46, 0);
		write4(buf + 50, 0);
		f.write(buf, 54);
		
		// write BMP DATA
		size_t padding = (i.sizex()) % 4;
		i8 padding_tab[] = {0, 0, 0, 0};
		T* buf2 = new T[i.sizex() * i.getNbComponents()];
		for (size_t y = 0; y < i.sizey(); ++y)
		{
			for (size_t n = 0; n < i.sizex(); ++n)
				for (size_t c = 0; c < i.getNbComponents(); ++c)
					buf2[n * i.getNbComponents() + c] = i(n, y, c);			// FIXME conversion min/max
			f.write((i8*)buf2, i.sizex() * i.getNbComponents() * sizeof (T));
			if (padding)
				f.write(padding_tab, padding);
		}
		delete [] buf2;
		return true;
   }


   /**
    @ingroup core
    @brief write a BMP, image internal format is BGR to a file.
    */
   template <class T, class Mapper, class Alloc>
   bool writeBmp( const Image<T, Mapper, Alloc>& i, const std::string& file )
   {
      std::ofstream f(file.c_str(), std::ios_base::binary);
      if (!f.is_open())
      {
         assert(0); // can't open file
         return false;
      }
      return writeBmp(i, f);
   }

   /**
    @ingroup core
    @brief read a BMP, image internal format is BGR
    */
   template <class T, class Mapper, class Alloc>
   bool readBmp( Image<T, Mapper, Alloc>& out_i, std::istream& f, Alloc alloc = Alloc() )
   {
      assert( IsNativeType<T>::value );
      size_t sx = 0;
      size_t sy = 0;
      size_t nbcomp = 0;
      f.seekg(18L, std::ios::beg);
		f.read((i8*)&sx, 4);
		f.read((i8*)&sy, 4);
		f.seekg(28L, std::ios::beg);
		f.read((i8*)&nbcomp, 2);
		nbcomp /= 8;
		if (!nbcomp || !sx || !sy)
			return false;
		f.seekg(54L, std::ios::beg);
      Image<T, Mapper, Alloc> image(sx, sy, nbcomp, false, alloc);

		size_t padding = sx % 4;
		ui8 padding_tab[3];		
		T* buf = new T[sx * nbcomp];
		for (size_t y = 0; y < sy; ++y)
		{
			f.read((i8*)buf, sx * nbcomp * sizeof (T));
			if (padding)
				f.read((i8*)padding_tab, padding);
			for (size_t n = 0; n < sx; ++n)
				for (size_t c = 0; c < nbcomp; ++c)
					image(n, y, c) = buf[n * nbcomp + c];		// FIXME conversion min/max
		}
		delete [] buf;

      out_i = image;
		return true;
   }

   /**
    @ingroup core
    @brief read a BMP, image internal format is BGR from a file.
    */
   template <class T, class Mapper, class Alloc>
   bool readBmp( Image<T, Mapper, Alloc>& out_i, const std::string& file, Alloc alloc = Alloc() )
   {
      std::ifstream f(file.c_str(), std::ios_base::binary);
      if (!f.is_open())
      {
         ensure( 0, "can't read image:" + file ); // can't open file
         return false;
      }
      return readBmp(out_i, f, alloc);
   }
}
}

# pragma warning( pop )

#endif
