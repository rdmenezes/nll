#ifndef MVV_PLATFORM_FONT_H_
# define MVV_PLATFORM_FONT_H_

# include "mvvPlatform.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Interface of a font writer: write a string to an image
    */
   class MVVPLATFORM_API Font
   {
   public:
      Font()
      {
         _color = nll::core::vector3uc( 255, 255, 255 );
         _size = 12;
      }

      void setSize( ui32 size )
      {
         assert( size > 6 );
         _size = size;
      }

      void setColor( nll::core::vector3uc color )
      {
         _color = color;
      }

      ui32 getSize() const
      {
         return _size;
      }

      const nll::core::vector3uc& getColor() const
      {
         return _color;
      }

      virtual ~Font()
      {}

      /**
       @brief Writes a string to an image. If the string doesn't fit in the image, then it should be properly cropped
              to the visible area
       */
      virtual void write( const std::string& str, const nll::core::vector2u& position, Image& image ) const = 0;

   protected:
      nll::core::vector3uc    _color;
      ui32                    _size;
   };

   /**
    @brief A default implementation of the <code>Font</code> base class

    This class loads a .bmp file containing all characters. They must be in a rectangular
    matrix regularly spaced.

    @note this class will load an inital BMP-font image. Then each time a color/font size is requested,
          it will be rsampled from the original font, and cached for futur use.
    */
   class MVVPLATFORM_API FontDefault
   {
      typedef std::map<char, Image> FontSet;
      struct Key
      {
         Key( ui32 sizet, const nll::core::vector3uc& colort ) : size( sizet ), color( colort )
         {}

         bool operator<( const Key& k ) const
         {
            return size < k.size;
         }

         ui32                 size;
         nll::core::vector3uc color;
      };
      typedef std::map<Key, FontSet> FontSets;

   public:
      /**
       @param fontResourcePath the path to  a .bmp font.
       @param characterSize the size of each character, they must be the same for all characters
       @param layout the number of column and row of characters
       @param transparentColor the transparent color to use (the only color that won't be printed on the screen from the font image)
       @param margin an initial margin
       @param an empty space between each character
       */
      FontDefault( const std::string& fontResourcePath,
                   const nll::core::vector2ui& characterSize,
                   const nll::core::vector2ui& layout,
                   nll::core::vector3u transparentColor = nll::core::vector3u( 0, 0, 0 ),
                   nll::core::vector2ui margin = nll::core::vector2ui( 0, 0),
                   nll::core::vector2ui space = nll::core::vector2ui( 0, 0) )
      {
      }

   protected:
      /**
       @brief create the reference font
       */
      void _populateFont( const std::string& fontResourcePath,
                          const nll::core::vector2ui& characterSize,
                          const nll::core::vector2ui& layout,
                          nll::core::vector3u transparentColor,
                          nll::core::vector2ui margin,
                          nll::core::vector2ui space )
      {
      }

   protected:
      FontSet*    _reference; // this is the inital font as loaded (so we know what font to use for optimal resampling)
      FontSets    _fontsets;
   };
}
}

#endif