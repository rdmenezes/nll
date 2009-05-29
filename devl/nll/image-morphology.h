#ifndef NLL_IMAGE_MORPHOLOGY_H_
# define NLL_IMAGE_MORPHOLOGY_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief binary morphology operation.
    */
   inline void dilate( ImageMask& mask )
   {
      ImageMask newMask( mask.sizex(), mask.sizey(), 1, true );
      for ( ui32 ny = 1; ny + 1 < mask.sizey(); ++ny )
         for ( ui32 nx = 1; nx + 1 < mask.sizex(); ++nx )
         {
            if ( mask( nx + 0, ny + 0, 0 ) ||
                 mask( nx + 1, ny + 0, 0 ) ||
                 mask( nx - 1, ny + 0, 0 ) ||
                 mask( nx + 0, ny + 1, 0 ) ||
                 mask( nx + 0, ny - 1, 0 ) )
              newMask( nx, ny, 0 ) = 255;
         }
      mask = newMask;
   }

   /**
    @ingroup core
    @brief binary morphology operation.
    */
   inline void erode( ImageMask& mask )
   {
      ImageMask newMask( mask.sizex(), mask.sizey(), 1, true );
      for ( ui32 ny = 1; ny + 1 < mask.sizey(); ++ny )
         for ( ui32 nx = 1; nx + 1 < mask.sizex(); ++nx )
         {
            if ( mask( nx + 0, ny + 0, 0 ) &&
                 mask( nx + 1, ny + 0, 0 ) &&
                 mask( nx - 1, ny + 0, 0 ) &&
                 mask( nx + 0, ny + 1, 0 ) &&
                 mask( nx + 0, ny - 1, 0 ) )
              newMask( nx, ny, 0 ) = 255;
         }
      mask = newMask;
   }

   /**
    @ingroup core
    @brief greyscale morphology operation.
    */
   template <class T, class Mapper>
   void dilate( Image<T, Mapper>& i )
   {
      Image<T, Mapper> ni( i.sizex(), i.sizey(), i.getNbComponents(), true );
      for ( ui32 c = 0; c < i.getNbComponents(); ++c )
         for ( ui32 ny = 1; ny + 1 < i.sizey(); ++ny )
            for ( ui32 nx = 1; nx + 1 < i.sizex(); ++nx )
            {
               T max = i( nx + 0, ny + 0, c );
               if ( i( nx + 1, ny + 0, 0 ) >= max )
                  max = i( nx + 1, ny + 0, c );
               if ( i( nx + 0, ny + 1, 0 ) >= max )
                  max = i( nx + 0, ny + 1, c );
               if ( i( nx - 1, ny + 0, 0 ) >= max )
                  max = i( nx - 1, ny + 0, c );
               if ( i( nx + 0, ny - 1, 0 ) >= max )
                  max = i( nx + 0, ny - 1, c );
               ni( nx, ny, c ) = max;
            }
      i = ni;
   }

   /**
    @ingroup core
    @brief greyscale morphology operation.
    */
   template <class T, class Mapper>
   void erode( Image<T, Mapper>& i )
   {
      Image<T, Mapper> ni( i.sizex(), i.sizey(), i.getNbComponents(), true );
      for ( ui32 c = 0; c < i.getNbComponents(); ++c )
         for ( ui32 ny = 1; ny + 1 < i.sizey(); ++ny )
            for ( ui32 nx = 1; nx + 1 < i.sizex(); ++nx )
            {
               T min = i( nx + 0, ny + 0, c );
               if ( i( nx + 1, ny + 0, 0 ) <= min )
                  min = i( nx + 1, ny + 0, c );
               if ( i( nx + 0, ny + 1, 0 ) <= min )
                  min = i( nx + 0, ny + 1, c );
               if ( i( nx - 1, ny + 0, 0 ) <= min )
                  min = i( nx - 1, ny + 0, c );
               if ( i( nx + 0, ny - 1, 0 ) <= min )
                  min = i( nx + 0, ny - 1, c );
               ni( nx, ny, c ) = min;
            }
      i = ni;
   }

   /**
    @ingroup core
    @brief morphology operation (greyscale or binary)
    */
   template <class T, class Mapper>
   inline void opening( Image<T, Mapper>& m, ui32 size )
   {
      for ( ui32 n = 0; n < size; ++n )
         erode( m );
      for ( ui32 n = 0; n < size; ++n )
         dilate( m );
   }

   /**
    @ingroup core
    @brief morphology operation (greyscale or binary)
    */
   template <class T, class Mapper>
   inline void closing( Image<T, Mapper>& m, ui32 size )
   {
      for ( ui32 n = 0; n < size; ++n )
         dilate( m );
      for ( ui32 n = 0; n < size; ++n )
         erode( m );
   }
}
}

#endif
