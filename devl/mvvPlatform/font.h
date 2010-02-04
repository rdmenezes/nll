#ifndef MVV_PLATFORM_FONT_H_
# define MVV_PLATFORM_FONT_H_

# include "mvvPlatform.h"
# include <nll/nll.h>

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
      virtual void write( const std::string& str, const nll::core::vector2ui& position, Image& image ) = 0;

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
   class MVVPLATFORM_API FontBitmapMatrix : public Font
   {
      struct ImageSet
      {
         Image    image;
         Image    mask;

         ImageSet( Image imaget,
                   Image maskt ) : image( imaget ), mask( mask )
         {}

         ImageSet()
         {}
      };
      typedef std::map<char, ImageSet> FontSet;
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
       @param fontResourcePath the path to  a .bmp font. If colors are needed, then 
       @param characterSize the size of each character, they must be the same for all characters
       @param layout the number of column and row of characters
       @param charactersMapping the mapped characters, one letter for each line by line from left to right concatenated
       @param transparentColor the transparent color to use (the only color that won't be printed on the screen from the font image)
       @param margin an initial margin (top-left and top margin)
       @param an empty space between each character
       */
      FontBitmapMatrix( const std::string& fontResourcePath,
                        const nll::core::vector2ui& characterSize,
                        const nll::core::vector2ui& layout,
                        const std::vector<char>& charactersMapping,
                        nll::core::vector3uc transparentColor = nll::core::vector3uc( 0, 0, 0 ),
                        nll::core::vector2ui margin = nll::core::vector2ui( 0, 0),
                        nll::core::vector2ui space = nll::core::vector2ui( 0, 0) ) : _reference( 0 )
      {
         _populateFont( fontResourcePath, characterSize, layout, charactersMapping, transparentColor, margin, space );
      }

      virtual void write( const std::string& str, const nll::core::vector2ui& position, Image& image )
      {
         // find the font
         Key key( _size, _color );
         FontSets::const_iterator it = _fontsets.find( key );
         if ( it == _fontsets.end() )
         {
            // we need to generate the font with the appropriate properties
            _generate( key );
         }

         FontSet& font = _fontsets[ key ];
      }

   protected:
      void _generate( Key key )
      {
         ensure( _reference, "Critical error: font reference can't be null" );

         // get a character
         ImageSet& refI = _reference->begin()->second;

         // compute it's new size
         float ratio = static_cast<float>( key.size ) / static_cast<float>( refI.image.sizex() );
         int size = nll::core::round( refI.image.sizey() * ratio );

         for ( FontSet::const_iterator it = _reference->begin(); it != _reference->end(); ++it )
         {
         }
      }

      /**
       @brief create the reference font, which is not resampled!
       */
      void _populateFont( const std::string& fontResourcePath,
                          const nll::core::vector2ui& characterSize,
                          const nll::core::vector2ui& layout,
                          const std::vector<char>& charactersMapping,
                          nll::core::vector3uc transparentColor,
                          nll::core::vector2ui margin,
                          nll::core::vector2ui space )
      {
         Image full;
         bool loaded = nll::core::readBmp( full, fontResourcePath );
         if ( !loaded )
         {
            throw std::exception( "Font could not be loaded." );
         }
         if ( !full.sizex() || !full.sizey() || full.getNbComponents() != 3 )
         {
            throw std::exception( "Font is not valid" );
         }

         ui32 minSizeX = margin[ 0 ] + ( layout[ 0 ] - 1 ) * space[ 0 ] + layout[ 0 ] * characterSize[ 0 ];
         ui32 minSizeY = margin[ 1 ] + ( layout[ 1 ] - 1 ) * space[ 1 ] + layout[ 1 ] * characterSize[ 1 ];
         if ( minSizeX >= full.sizex() ||
              minSizeY >= full.sizey() ||
              charactersMapping.size() != layout[ 0 ] * layout[ 1 ] )
         {
            throw std::exception( "font size is incorrect: computed minium size is bigger than the bitmap, or all characters of the font are not mapped." );
         }

         FontSet reference;
         for ( ui32 y = 0; y < layout[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < layout[ 0 ]; ++x )
            {
               ui32 px = margin[ 0 ] + x * characterSize[ 0 ] + ( x - 1 ) * space[ 0 ];
               ui32 py = margin[ 1 ] + y * characterSize[ 1 ] + ( y - 1 ) * space[ 1 ];
               Image c( characterSize[ 0 ], characterSize[ 1 ], 3, false );
               Image mask( characterSize[ 0 ], characterSize[ 1 ], 1, false );
               for ( ui32 dy = 0; dy < characterSize[ 1 ]; ++dy )
               {
                  for ( ui32 dx = 0; dx < characterSize[ 0 ]; ++dx )
                  {
                     // update the bitmap
                     const ui8 r = full( px + dx, py + dy, 0 );
                     const ui8 g = full( px + dx, py + dy, 1 );
                     const ui8 b = full( px + dx, py + dy, 2 );
                     c( dx, dy, 0 ) = r;
                     c( dx, dy, 1 ) = g;
                     c( dx, dy, 2 ) = b;

                     // update if mask
                     if ( r == transparentColor[ 0 ] &&
                          g == transparentColor[ 1 ] &&
                          b == transparentColor[ 2 ] )
                     {
                        mask( dx, dy, 0 ) = 0;
                     } else {
                        mask( dx, dy, 0 ) = 1;
                     }
                  }
               }
               reference[ charactersMapping[ x + y * layout[ 0 ] ] ] = ImageSet( c, mask );
            }
         }
         Key key( characterSize[ 0 ], nll::core::vector3uc( 255, 255, 255 ) );
         _fontsets[ key ] = reference;
         _reference = &_fontsets[ key ];
      }

   protected:
      FontSet*             _reference; // this is the inital font as loaded (so we know what font to use for optimal resampling)
      FontSets             _fontsets;
   };
}
}

#endif