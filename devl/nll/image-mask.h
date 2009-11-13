#ifndef NLL_IMAGE_MASK_H_
# define NLL_IMAGE_MASK_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief define a mask as an image of 32bits with 1 dimension.
    */
   typedef Image<ui32, IndexMapperRowMajorFlat2DColorRGBnMask>  ImageMask;

   /**
    @ingroup core
    @brief create a mask from an image.
    
    If the pixel (x, y) is different from(0, 0, ...) then mask(x, y)=true
    */
   template <class T, class Mapper, class Allocator>
   ImageMask createMask( const Image<T, Mapper, Allocator>& i )
   {
      ImageMask mask( i.sizex(), i.sizey(), 1, false );
      for ( ui32 ny = 0; ny < i.sizey(); ++ny )
         for ( ui32 nx = 0; nx < i.sizex(); ++nx )
         {
            bool flagVal = false;
            for ( ui32 nc = 0; nc < i.getNbComponents(); ++nc )
               if ( i( nx, ny, nc ) )
               {
                  flagVal = true;
                  break;
               }
            mask( nx, ny, 0 ) = flagVal ? 255 : 0;
         }
      return mask;
   }

   /**
    @ingroup core
    @brief extract a subimage from the image, using a mask.
    
    If mask(x, y) then the pixel (x, y) has the same value than the original, else replaced by backrgound
    */
   template <class T, class Mapper, class Allocator>
   Image<T, Mapper, Allocator> extract( const Image<T, Mapper, Allocator>& img, const ImageMask& mask, const T* background = Image<T, Mapper>::black(), Allocator alloc = Allocator() )
   {
      assert( mask.getNbComponents() == 1 && img.sizex() == mask.sizex() && img.sizey() == mask.sizey() );
      Image<T, Mapper, Allocator> img2( img.sizex(), img.sizey(), img.getNbComponents(), true, alloc );
      for ( ui32 ny = 0; ny < img.sizey(); ++ny )
         for ( ui32 nx = 0; nx < img.sizex(); ++nx )
         {
            ui32 index = img2.index( nx, ny, 0 );
            T* dst = img2.getBuf() + index;
            if ( mask( nx, ny, 0 ) )
            {
               const T* src = img.getBuf() + index;
               for ( ui32 c = 0; c < img.getNbComponents(); ++c )
                  dst[ c ] = src[ c ];
            } else {
               for ( ui32 c = 0; c < img.getNbComponents(); ++c )
                  dst[ c ] = background[ c ];
            }
         }
      return img2;
   }

   /**
    @ingroup core
    @brief extract a subimage from the image, using a specific ID in the mask.
    
    If mask(x, y) == id then the pixel (x, y) has the same value than the original, else replaced by backrgound
    */
   template <class T, class Mapper, class Allocator>
   Image<T, Mapper, Allocator> extract( const Image<T, Mapper, Allocator>& img, const ImageMask& mask, ui32 id, const T* background = Image<T, Mapper>::black(), Allocator alloc = Allocator() )
   {
      assert( mask.getNbComponents() == 1 && img.sizex() == mask.sizex() && img.sizey() == mask.sizey() );
      Image<T, Mapper, Allocator> img2( img.sizex(), img.sizey(), img.getNbComponents(), alloc );
      for ( ui32 ny = 0; ny < img.sizey(); ++ny )
         for ( ui32 nx = 0; nx < img.sizex(); ++nx )
         {
            ui32 index = img2.index( nx, ny, 0 );
            T* dst = img2.getBuf() + index;
            if ( mask( nx, ny, 0 ) == id )
            {
               const T* src = img.getBuf() + index;
               for ( ui32 c = 0; c < img.getNbComponents(); ++c )
                  dst[ c ] = src[ c ];
            } else {
               for ( ui32 c = 0; c < img.getNbComponents(); ++c )
                  dst[ c ] = background[ c ];
            }
         }
      return img2;
   }
}
}

#endif
