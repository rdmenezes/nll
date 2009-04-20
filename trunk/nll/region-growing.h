#ifndef NLL_REGION_GROWING_H_
# define NLL_REGION_GROWING_H_

# include <deque>

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Specify if 2 pixels are different (exact match).
    */
   template <class T>
   class RegionPixelDifferent
   {
   public:
      RegionPixelDifferent( ui32 nbColors ) : _nbColors( nbColors )
      {}

      // return true if same, false otherwise
      bool operator()( const T* c1, const T* c2 ) const
      {
         for ( ui32 n = 0; n < _nbColors; ++n )
            if ( c1[ n ] != c2[ n ] )
               return false;
         return true;
      }
   private:
      ui32     _nbColors;
   };

   /**
    @ingroup algorithm
    @brief Specify if 2 pixels are different (thresolded distance).
    */
   template <class T>
   class RegionPixelSimilar
   {
   public:
      RegionPixelSimilar( ui32 nbColors, f64 minDist ) : _nbColors( nbColors ), _dist( minDist )
      {}

      bool operator()( const T* c1, const T* c2 ) const
      {
         return core::generic_norm2<T*, f64>( (T*)c1, (T*)c2, _nbColors ) < _dist;
      }
   private:
      ui32     _nbColors;
      f64      _dist;
   };


   /**
    @ingroup algorithm
    @brief Region growing from a seed point. the pixels on the border of the image are all discarded.
    */
   template <  class T,
               class Mapper = core::IndexMapperRowMajorFlat2DColorRGBn,
               class Different = RegionPixelDifferent<T> >
   class RegionGrowing
   {
   public:
      typedef Different                   DifferentPixel;
      typedef T                           value_type;

   public:
      RegionGrowing( const Different& diff ) : _diff( diff ){}

      /**
       @brief run the algorithm in a specific bounding box
       @note boundingbox is inclusive
       @param outRegions returns and allocated image with regions
       @param seed seed point from where the region is growing
       @param seedId must be >0, the growing region will have this ID in the mask
       */
      void grow( const core::Image<T, Mapper>& img, const core::vector2i& seed, const core::vector2i& minBoundingBox, const core::vector2i& maxBoundingBox, const ui32 seedId, core::ImageMask& outRegions )
      {
         typedef std::deque<core::vector2i>  Container;
         assert( img.sizex() && img.sizey() );
         assert( seed[ 0 ] < (i32)img.sizex() && seed[ 1 ] < (i32)img.sizey() );
         assert( maxBoundingBox[ 0 ] < (i32)img.sizex() );
         assert( maxBoundingBox[ 1 ] < (i32)img.sizey() );
         assert( minBoundingBox[ 0 ] <= maxBoundingBox[ 0 ] );
         assert( minBoundingBox[ 1 ] <= maxBoundingBox[ 1 ] );
         assert( seedId != 0 );

         if ( outRegions.sizex() != img.sizex() ||
              outRegions.sizey() != img.sizey() ||
              outRegions.getNbComponents() != 1 )
            outRegions = core::ImageMask( img.sizex(), img.sizey(), 1 );
         Container pixels;
         pixels.push_back( seed );
         const T* seedValue = img.point( seed[ 0 ], seed[ 1 ] );
         while ( pixels.size() )
         {
            const core::vector2i pixel = pixels.front(); // TODO test
            ui32 index = outRegions.index( pixel[ 0 ], pixel[ 1 ], 0 );
            if ( pixel[ 0 ] <= maxBoundingBox[ 0 ] &&
                 pixel[ 0 ] >= minBoundingBox[ 0 ] &&
                 pixel[ 1 ] <= maxBoundingBox[ 1 ] &&
                 pixel[ 1 ] >= minBoundingBox[ 1 ] &&
                 *(outRegions.getBuf() + index) == 0 &&
                 _diff( seedValue, img.point( pixel[ 0 ], pixel[ 1 ] ) ) )
            {
               pixels.push_back( core::vector2i( pixel[ 0 ] + 1, pixel[ 1 ] + 0 ) );
               pixels.push_back( core::vector2i( pixel[ 0 ] - 1, pixel[ 1 ] + 0 ) );
               pixels.push_back( core::vector2i( pixel[ 0 ] + 0, pixel[ 1 ] + 1 ) );
               pixels.push_back( core::vector2i( pixel[ 0 ] - 0, pixel[ 1 ] - 1 ) );
               *(outRegions.getBuf() + index) = seedId;
            }
            pixels.pop_front();
         }
      }

      /**
       @brief run the algorithm
       @note boundingbox is inclusive
       @param outRegions returns and allocated image with regions
       @param seed seed point from where the region is growing
       @param seedId must be >0, the growing region will have this ID in the mask
       */
      inline void grow( const core::Image<T, Mapper>& img, const core::vector2i& seed, const ui32 seedId, core::ImageMask& outRegions )
      {
         return grow( img, seed, core::vector2i( 0, 0 ), core::vector2i( img.sizex() - 1, img.sizey() - 1 ), seedId, outRegions );
      }
   public:
      Different   _diff;
   };

}
}

#endif
