#ifndef MVV_PLATFORM_FONT_H_
# define MVV_PLATFORM_FONT_H_

# include "mvvPlatform.h"
# include "types.h"
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
         _colorBackground = nll::core::vector3uc( 0, 0, 0 );
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

      void setBackground( nll::core::vector3uc color )
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
      virtual void write( const std::string& str, const nll::core::vector2ui& position, Image& image, bool displayBackground = false ) = 0;

      /**
       @brief Writes the string to an image, only the text within (min, max) will actually be written
       */
      virtual void write( const std::string& str, const nll::core::vector2ui& position, Image& image, const nll::core::vector2ui& min, const nll::core::vector2ui& max, bool displayBackground = false ) = 0;

      /**
       @brief Returns the x position of the 'character' displaying 'str' at this position
       */
      virtual ui32 getSize( const std::string& str, const nll::core::vector2ui& position, ui32 character ) = 0;

   protected:
      nll::core::vector3uc    _color;
      nll::core::vector3uc    _colorBackground;
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
                   Image maskt ) : image( imaget ), mask( maskt )
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
            ui32 v1 = size + ( static_cast<ui32>( color[ 0 ] ) << 8 ) + ( static_cast<ui32>( color[ 1 ] ) << 16 ) + ( static_cast<ui32>( color[ 2 ] ) << 24 );
            ui32 v2 = k.size + ( static_cast<ui32>( k.color[ 0 ] ) << 8 ) + ( static_cast<ui32>( k.color[ 1 ] ) << 16 ) + ( static_cast<ui32>( k.color[ 2 ] ) << 24 );
            return v1 < v2;
         }

         bool operator==( const Key& k ) const
         {
            return size == k.size && color == k.color;
         }

         ui32                 size;
         nll::core::vector3uc color;
      };
      /*
      struct KeyCompareLess
      {
         bool operator()(const Key& s1, const Key& s2) const
         {
            return s1 < s2;
         }
      };*/

      typedef std::map<Key, FontSet> FontSets;


   public:
      /**
       @param fontResourcePath the path to  a .bmp font. If colors are needed, then 
       @param characterSize the size of each character, they must be the same for all characters
       @param layout the number of column and row of characters
       @param charactersMapping the mapped characters, one letter for each line by line from left to right concatenated
       @param cropAlignX if true, the font will be cropped on X and aligned on X only
       @param transparentColor the transparent color to use (the only color that won't be printed on the screen from the font image)
       @param margin an initial margin (top-left and top margin)
       @param an empty space between each character
       */
      FontBitmapMatrix( const std::string& fontResourcePath,
                        const nll::core::vector2ui& characterSize,
                        const nll::core::vector2ui& layout,
                        const std::vector<char>& charactersMapping,
                        bool cropAlignX = true,
                        nll::core::vector3uc transparentColor = nll::core::vector3uc( 0, 0, 0 ),
                        nll::core::vector2ui margin = nll::core::vector2ui( 0, 0),
                        nll::core::vector2ui space = nll::core::vector2ui( 0, 0),
                        bool throwOnMissingChar = true ) : _reference( 0 ), _sizeXRef( characterSize[ 0 ] ), _throwOnMissingChar( throwOnMissingChar )
      {
         _populateFont( fontResourcePath, characterSize, layout, charactersMapping, cropAlignX, transparentColor, margin, space );
      }

      virtual ui32 getSize( const std::string& str, const nll::core::vector2ui& position, ui32 character )
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
         nll::core::vector2ui p = position;
         for ( ui32 n = 0; n < character; ++n )
         {
            FontSet::const_iterator it = font.find( str[ n ] );
            if ( it == font.end() )
            {
               if ( _throwOnMissingChar )
                  throw std::runtime_error( "character not found in charset" );
               else
                  it = font.find( ' ' );
               if ( it == font.end() )
                  throw std::runtime_error( "character not found in charset" );
            }
            ui32 sizex = it->second.image.sizex();
            p[ 0 ] += sizex;
         }
         return p[ 0 ];
      }

      /**
       @brief Write a string using the current size & color

       @param maxpos Add a position constraint: letters 
       */
      virtual void write( const std::string& str, const nll::core::vector2ui& position, Image& image, const nll::core::vector2ui& minpos, const nll::core::vector2ui& maxpos, bool displayBackground = false )
      {
         ensure( image.getNbComponents() == 3, "error: it must be a RGB image!" );
         ensure( maxpos[ 0 ] <= image.sizex() && maxpos[ 1 ] <= image.sizey(), "min/max problem" );
         ensure( maxpos[ 0 ] >= minpos[ 0 ] && maxpos[ 1 ] >= minpos[ 1 ], "min/max problem" );

         // find the font
         Key key( _size, _color );
         FontSets::const_iterator it = _fontsets.find( key );
         if ( it == _fontsets.end() )
         {
            // we need to generate the font with the appropriate properties
            _generate( key );
         }

         FontSet& font = _fontsets[ key ];
         nll::core::vector2ui p = position;
         for ( ui32 n = 0; n < str.size(); ++n )
         {
            FontSet::const_iterator it = font.find( str[ n ] );
            if ( it == font.end() )
            {
               if ( _throwOnMissingChar )
                  throw std::runtime_error( "character not found in charset" );
               else
                  it = font.find( ' ' );
               if ( it == font.end() )
                  throw std::runtime_error( "character not found in charset" );
            }
            
            ui32 sizex = it->second.image.sizex();
            ui32 sizey = it->second.image.sizey();

            if ( p[ 0 ] >= minpos[ 0 ] && p[ 1 ] >= minpos[ 1 ] )
            {
               if ( p[ 0 ] + sizex > maxpos[ 0 ] ||
                    p[ 1 ] + sizey > maxpos[ 1 ] )
               {
                  // we are out of bound, just don't write the next letters...
                  break;
               }
               _writeLetter( it->second, p, image, displayBackground );
            }
            p[ 0 ] += sizex;
         }
      }

      /**
       @brief Write a string using the current size & color
       */
      virtual void write( const std::string& str, const nll::core::vector2ui& position, Image& image, bool displayBackground = false )
      {
         write( str, position, image, nll::core::vector2ui( 0, 0 ), nll::core::vector2ui( image.sizex(), image.sizey() ), displayBackground );
      }

   protected:
      void _writeLetter( const ImageSet& font, const nll::core::vector2ui& position, Image& image, bool displayBackground )
      {
         // we now it is a row-based storage so we can optimize a bit...
         Image::const_iterator itFont = font.image.begin();
         Image::const_iterator itMask = font.mask.begin();
         Image::DirectionalIterator itImage = image.getIterator( position[ 0 ], position[ 1 ], 0 );

         const ui32 sizex = font.image.sizex();
         const ui32 sizey = font.image.sizey();

         if ( displayBackground )
         {
            for ( ui32 y = 0; y < sizey; ++y )
            {
               Image::DirectionalIterator line = itImage;
               for ( ui32 x = 0; x < sizex; ++x )
               {
                  if ( *itMask > 32  )
                  {
                     line.pickcol( 0 ) = *itFont++;
                     line.pickcol( 1 ) = *itFont++;
                     line.pickcol( 2 ) = *itFont++;
                  } else {
                     line.pickcol( 0 ) = _colorBackground[ 0 ];
                     line.pickcol( 1 ) = _colorBackground[ 1 ];
                     line.pickcol( 2 ) = _colorBackground[ 2 ];
                     itFont += 3;
                  }

                  line.addx();
                  ++itMask;
               }
               itImage.addy();
            }
         } else {
            for ( ui32 y = 0; y < sizey; ++y )
            {
               Image::DirectionalIterator line = itImage;
               for ( ui32 x = 0; x < sizex; ++x )
               {
                  if ( *itMask > 32  )
                  {
                     line.pickcol( 0 ) = *itFont++;
                     line.pickcol( 1 ) = *itFont++;
                     line.pickcol( 2 ) = *itFont++;
                  } else {
                     itFont += 3;
                  }

                  line.addx();
                  ++itMask;
               }
               itImage.addy();
            }
         }
      }

      void _generate( Key key )
      {
         std::cout << "generating font..." << std::endl;
         ensure( _reference, "Critical error: font reference can't be null" );

         // get a character
         ImageSet& refI = _reference->begin()->second;

         // new color
         const float ratioR = static_cast<float>( key.color[ 0 ] ) / 255; 
         const float ratioG = static_cast<float>( key.color[ 1 ] ) / 255; 
         const float ratioB = static_cast<float>( key.color[ 2 ] ) / 255; 

         // compute it's new size
         float ratio = static_cast<float>( key.size ) / static_cast<float>( _sizeXRef );
         int sizey = nll::core::round( refI.image.sizey() * ratio );

         FontSet newFontSet;
         for ( FontSet::const_iterator it = _reference->begin(); it != _reference->end(); ++it )
         {
            ImageSet imageSet( it->second );
            ui32 sizex = static_cast<ui32>( it->second.image.sizex() * ratio );
            if ( it->first == ' ' )
            {
               nll::core::rescaleBilinear( imageSet.image, sizex / 2, sizey );
               nll::core::rescaleBilinear( imageSet.mask, sizex / 2, sizey );
            } else {
               nll::core::rescaleBilinear( imageSet.image, sizex, sizey );
               nll::core::rescaleBilinear( imageSet.mask, sizex, sizey );
            }
            for ( ui32 y = 0; y < imageSet.image.sizey(); ++y )
            {
               for ( ui32 x = 0; x < imageSet.image.sizex(); ++x )
               {
                  imageSet.image( x, y, 0 ) = static_cast<Image::value_type>( imageSet.image( x, y, 0 ) * ratioR );
                  imageSet.image( x, y, 1 ) = static_cast<Image::value_type>( imageSet.image( x, y, 1 ) * ratioG );
                  imageSet.image( x, y, 2 ) = static_cast<Image::value_type>( imageSet.image( x, y, 2 ) * ratioB );
                  if ( imageSet.mask( x, y, 0 ) <= 128 )
                  {
                     imageSet.mask( x, y, 0 ) = 0;
                  }
               }
            }
            newFontSet.insert( std::make_pair( it->first, imageSet ) );
         }
         _fontsets[ key ] = newFontSet;
      }

      /**
       @brief create the reference font, which is not resampled!
       */
      void _populateFont( const std::string& fontResourcePath,
                          const nll::core::vector2ui& characterSize,
                          const nll::core::vector2ui& layout,
                          const std::vector<char>& charactersMapping,
                          bool cropAlignX, 
                          nll::core::vector3uc transparentColor,
                          nll::core::vector2ui margin,
                          nll::core::vector2ui space )
      {
         std::cout << "loading font..." << std::endl;
         Image full;
         bool loaded = nll::core::readBmp( full, fontResourcePath );
         if ( !loaded )
         {
            throw std::runtime_error( "Font could not be loaded." );
         }
         if ( !full.sizex() || !full.sizey() || full.getNbComponents() != 3 )
         {
            throw std::runtime_error( "Font is not valid" );
         }

         ui32 minSizeX = margin[ 0 ] + ( layout[ 0 ] - 1 ) * space[ 0 ] + layout[ 0 ] * characterSize[ 0 ];
         ui32 minSizeY = margin[ 1 ] + ( layout[ 1 ] - 1 ) * space[ 1 ] + layout[ 1 ] * characterSize[ 1 ];
         if ( minSizeX > full.sizex() ||
              minSizeY > full.sizey() ||
              charactersMapping.size() != layout[ 0 ] * layout[ 1 ] )
         {
            throw std::runtime_error( "font size is incorrect: computed minium size is bigger than the bitmap, or all characters of the font are not mapped." );
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
                        mask( dx, dy, 0 ) = 255;
                     }
                  }
               }

               if ( cropAlignX )
               {
                  ensure( transparentColor == nll::core::vector3uc( 0, 0, 0 ), "only handled for transparentColor=(0, 0, 0)" );
                  ui32 x1, x2;
                  nll::core::cropVertical( c, 0, 0, &x1, &x2 );
                  nll::core::addBorder( c, 1, 0 );
                  nll::core::extract( mask, x1, 0, x2, c.sizey() - 1 );
                  nll::core::addBorder( mask, 1, 0 );
               }

               reference[ charactersMapping[ x + y * layout[ 0 ] ] ] = ImageSet( c, mask );
            }
         }
         Key key( characterSize[ 0 ], nll::core::vector3uc( 255, 255, 255 ) );
         _fontsets[ key ] = reference;
         _reference = &_fontsets[ key ];
         std::cout << "font loaded!" << std::endl;
      }

   protected:
      FontSet*             _reference; /// this is the inital font as loaded (so we know what font to use for optimal resampling)
      FontSets             _fontsets;
      ui32                 _sizeXRef; /// the initial size X which is used to compute resampling coefficients
      bool                 _throwOnMissingChar;
   };
}
}

#endif