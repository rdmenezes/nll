#ifndef NLL_ALGORITHM_HAAR_FEATURES_H_
# define NLL_ALGORITHM_HAAR_FEATURES_H_

# pragma warning( push )
# pragma warning( disable:4127 ) // conditional expression is constant

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Represent an "Integral image".
    @see http://research.microsoft.com/en-us/um/people/viola/pubs/detect/violajones_cvpr2001.pdf for full detail
         Rapid Object Detection using a Boosted Cascade of Simple Features, Paul Viola, Michael Jones
    
    It is defined such as i(x, y) = sum( image( x', y'))_{x' <= x && y' <= y}
    */
   class IntegralImage
   {
   public:
      typedef double                   value_type;

   private:
      typedef core::Image<value_type>  Storage;

   public:
      /**
       @brief Construct the integral image
       */
      template <class T>
      void process( const core::Image<T>& image )
      {
         //
         // computes the integral image ii, with s the sum of the cumulated rows, s(x, -1) = 0, ii( -1, y ) = 0
         // s(x, y) = s(x, y - 1) + i(x, y)
         // ii(x, y) = i( x - 1, y) + s(x, y)
         //
         ensure( image.sizex() > 1 && image.sizey() > 1 && image.getNbComponents() == 1, "only 1 component image is handled" );
         _img = Storage( image.sizex(), image.sizey(), 1, false );

         std::vector<value_type> s( image.sizex() );
         std::vector<value_type> ii( image.sizex() );

         // first compute the first row & column
         Storage::DirectionalIterator it = _img.getIterator( 0, 0, 0 );
         core::Image<T>::DirectionalIterator itSrc = image.getIterator( 0, 0, 0 );

         // init first step
         s[ 0 ] = *itSrc;
         *it = *itSrc;
         value_type previous_ii = s[ 0 ];
         it.addx();
         itSrc.addx();
         for ( ui32 x = 1; x < image.sizex(); ++x )
         {
            s[ x ] = *itSrc;
            *it = previous_ii + s[ x ];

            previous_ii = *it;
            it.addx();
            itSrc.addx();
         }


         // computes the other rows
         for ( ui32 y = 1; y < image.sizey(); ++y )
         {
            // init
            it = _img.getIterator( 0, y, 0 );
            itSrc = image.getIterator( 0, y, 0 );

            value_type tmp = *itSrc;
            s[ 0 ] = s[ 0 ] + static_cast<value_type>( tmp );

            // first step
            *it = s[ 0 ];
            previous_ii = s[ 0 ];

            it.addx();
            itSrc.addx();

            // main loop
            for ( ui32 x = 1; x < image.sizex(); ++x )
            {
               s[ x ] = s[ x ] + static_cast<value_type>( *itSrc );
               *it = previous_ii + s[ x ];
               previous_ii = *it;

               it.addx();
               itSrc.addx();
            }
         }
      }

      /**
       @brief returns the sum of the pixels contained in a rectangle
       */
      value_type getSum( const core::vector2ui& bottomLeft,
                         const core::vector2ui& topRight ) const
      {
         value_type val1;
         value_type val2;
         value_type val3;
         value_type val4;

         core::vector2ui bl( bottomLeft[ 0 ] - 1, bottomLeft[ 1 ] - 1 );
         if ( bl[ 0 ] >= sizex() || bl[ 1 ] >= sizex() )
         {
            // specific case for a bottom left index == 0
            val1 = operator()( bl[ 0 ], bl[ 1 ] );
            val2 = operator()( bl[ 0 ] + topRight[ 0 ] - bl[ 0 ], bl[ 1 ] );
            val3 = operator()( bl[ 0 ], bl[ 1 ] + topRight[ 1 ] - bl[ 1 ] );
            val4 = operator()( topRight[ 0 ], topRight[ 1 ] );
         } else {
            Storage::DirectionalIterator it = _img.getIterator( bl[ 0 ], bl[ 1 ], 0 );
            val1 = *it;
            Storage::DirectionalIterator itr = it;
            itr.addx( topRight[ 0 ] - bl[ 0 ] );
            val2 = *itr;
            it.addy( topRight[ 1 ] - bl[ 1 ] );
            val3 = *it;
            it.addx( topRight[ 0 ] - bl[ 0 ] );
            val4 = *it;
         }

         return val4 + val1 - ( val2 + val3 );
      }

      /**
       @brief returns the value of the integral image at (x, y) = sum( image( x', y'))_{x' < x && y' < y}
       */
      const value_type operator()( ui32 x, ui32 y ) const
      {
         if ( x >= sizex() || y >= sizey() )
            return 0;
         return _img( x, y, 0 );
      }

      /**
       @brief returns the size in x of the image
       */
      ui32 sizex() const
      {
         return _img.sizex();
      }

      /**
       @brief returns the size in y of the image
       */
      ui32 sizey() const
      {
         return _img.sizey();
      }

   private:
      Storage     _img;
   };


   /**
    @ingroup algorithm
    @brief Computes 2D Haar features on an image
    */
   class Haar2dFeatures
   {
   public:
      /**
       @brief encode a feature.
       @param bf the bottom left coordinate, it must be in [0..1][0..1] intervals. (i.e. for scaling)
       @param tr the top right coordinate, it must be in [0..1][0..1] intervals. (i.e. for scaling)
       */
      struct Feature
      {
         enum Direction
         {
            VERTICAL,
            HORIZONTAL
         };

         /**
          @brief Construct the feature
          */
         Feature( Direction dir, const core::vector2d& bf, const core::vector2d& tr ) : direction( dir ), bottomLeft( bf ), topRight( tr )
         {
            assert( bf[ 0 ] >= 0 && bf[ 0 ] <= 1 &&
                    bf[ 1 ] >= 0 && bf[ 1 ] <= 1 &&
                    tr[ 0 ] >= 0 && tr[ 0 ] <= 1 &&
                    tr[ 1 ] >= 0 && tr[ 1 ] <= 1 ); // must be in [0..1]
            assert( bf[ 0 ] <= tr[ 0 ] && bf[ 1 ] <= tr[ 1 ] ); // check precedence
         }

         Direction         direction;
         core::vector2d    bottomLeft;
         core::vector2d    topRight;
      };
      typedef std::vector<Feature>           Features;

   public:
      typedef double                         internal_type;
      typedef core::Buffer1D<internal_type>  Buffer;

   public:
      /**
       @brief computes the Haar features on an integral image
       @param scale the scale applied to the features
       */
      static Buffer process( const Features& features, const IntegralImage& i, double scale = 1.0 )
      {
         Buffer buffer( static_cast<ui32>( features.size() ), false );
         for ( ui32 n = 0; n < buffer.size(); ++n )
         {
            const Feature& f = features[ n ];
            ui32 x1 = static_cast<ui32>( scale * f.bottomLeft[ 0 ] * i.sizex() );
            ui32 y1 = static_cast<ui32>( scale * f.bottomLeft[ 1 ] * i.sizey() );
            ui32 x2 = static_cast<ui32>( scale * f.topRight[ 0 ]   * i.sizex() );
            ui32 y2 = static_cast<ui32>( scale * f.topRight[ 1 ]   * i.sizey() );

            x1 = std::min( x1, i.sizex() );
            x2 = std::min( x2, i.sizex() );

            y1 = std::min( y1, i.sizey() );
            y2 = std::min( y2, i.sizey() );

            IntegralImage::value_type sump;
            IntegralImage::value_type sumd;
            ui32 mid;
            switch ( f.direction )
            {
            case Feature::HORIZONTAL:
               mid = ( x1 + x2 ) / 2;
               sump = i.getSum( core::vector2ui( x1, y1 ), core::vector2ui( mid, y2 ) );
               sumd = i.getSum( core::vector2ui( mid, y1 ), core::vector2ui( x2, y2 ) );
               buffer[ n ] = static_cast<internal_type>( sump - sumd );
               break;
            case Feature::VERTICAL:
               mid = ( y1 + y2 ) / 2;
               sump = i.getSum( core::vector2ui( x1, y1 ), core::vector2ui( x2, mid ) );
               sumd = i.getSum( core::vector2ui( x1, mid ), core::vector2ui( x2, y2 ) );
               buffer[ n ] = static_cast<internal_type>( sump - sumd );
               break;
            default:
               ensure( 0, "not handled type" );
            }
         }
         return buffer;
      }

      template <class T>
      static Buffer process( const Features& features, const core::Image<T>& i, double scale = 1.0 )
      {
         IntegralImage integral;
         integral.process( i );
         return process( features, integral, scale );
      }
   };
}
}

# pragma warning( pop )

#endif
