#ifndef NLL_IMAGE_IO_H_
# define NLL_IMAGE_IO_H_

# include <iostream>
# include "image.h"

namespace nll
{
namespace core
{
   #define INV2(n)(((n & 0x0F) << 4) | ((n & 0xF0) >> 4))
   #define TO1(n)	(n & 0xFF)
   #define TO2(n)	((n & 0xFF00) >> 8)
   #define TO3(n)	((n & 0xFF0000) >> 16)
   #define TO4(n)	((n & 0xFF000000) >> 24)

   inline static void write4(i8* buf, ui32 v)
   {
	   *buf		= (i8)TO1(v);
	   *(buf + 1)	= (i8)TO2(v);
	   *(buf + 2)	= (i8)TO3(v);
	   *(buf + 3)	= (i8)TO4(v);
   }

   inline static void write2(i8* buf, ui32 v)
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
      ui32 size = i.sizex() * i.sizey() * i.getNbComponents() + 54;
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
		ui32 padding = (i.sizex()) % 4;
		i8 padding_tab[] = {0, 0, 0, 0};
		T* buf2 = new T[i.sizex() * i.getNbComponents()];
		for (ui32 y = 0; y < i.sizey(); ++y)
		{
			for (ui32 n = 0; n < i.sizex(); ++n)
				for (ui32 c = 0; c < i.getNbComponents(); ++c)
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
   bool readBmp( Image<T, Mapper>& out_i, std::istream& f, Alloc alloc = Alloc() )
   {
      assert( IsNativeType<T>::value );
      ui32 sx = 0;
      ui32 sy = 0;
      ui32 nbcomp = 0;
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

		ui32 padding = sx % 4;
		ui8 padding_tab[3];		
		T* buf = new T[sx * nbcomp];
		for (ui32 y = 0; y < sy; ++y)
		{
			f.read((i8*)buf, sx * nbcomp * sizeof (T));
			if (padding)
				f.read((i8*)padding_tab, padding);
			for (ui32 n = 0; n < sx; ++n)
				for (ui32 c = 0; c < nbcomp; ++c)
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
         assert(0); // can't open file
         return false;
      }
      return readBmp(out_i, f, alloc);
   }
}
}

#endif
