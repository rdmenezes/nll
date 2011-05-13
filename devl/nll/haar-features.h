/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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
         typename core::Image<T>::DirectionalIterator itSrc = image.getIterator( 0, 0, 0 );

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
   class HaarFeatures2d
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
            HORIZONTAL,
            VERTICAL_TRIPLE,
            HORIZONTAL_TRIPLE,
            CHECKER,
            NONE
         };

         Feature() : _direction( NONE )
         {
         }

         /**
          @brief Construct the feature
          */
         Feature( Direction dir, const core::vector2d& bf, const core::vector2d& tr ) : _direction( dir )
         {
            assert( bf[ 0 ] >= 0 && bf[ 0 ] <= 1 &&
                    bf[ 1 ] >= 0 && bf[ 1 ] <= 1 &&
                    tr[ 0 ] >= 0 && tr[ 0 ] <= 1 &&
                    tr[ 1 ] >= 0 && tr[ 1 ] <= 1 ); // must be in [0..1]
            assert( bf[ 0 ] <= tr[ 0 ] && bf[ 1 ] <= tr[ 1 ] ); // check precedence

            double dx = static_cast<double>( tr[ 0 ] - bf[ 0 ] );
            double dy = static_cast<double>( tr[ 1 ] - bf[ 1 ] );

            _size = core::vector2d( dx, dy );
            _centre = core::vector2d( ( tr[ 0 ] + bf[ 0 ] ) / ( 2 ),
                                      ( tr[ 1 ] + bf[ 1 ] ) / ( 2 ) );
         }

         Feature( Direction dir, const IntegralImage& i, const core::vector2ui& bf, const core::vector2ui& tr ) : _direction( dir )
         {
            ensure( bf[ 0 ] < tr[ 0 ] && bf[ 1 ] < tr[ 1 ], "bad corner" );
            ensure( tr[ 0 ] < i.sizex() && tr[ 1 ] < i.sizey(), "outside image" );

            double dx = static_cast<double>( tr[ 0 ] - bf[ 0 ] );
            double dy = static_cast<double>( tr[ 1 ] - bf[ 1 ] );

            _size = core::vector2d( dx, dy );
            _centre = core::vector2d( ( tr[ 0 ] + bf[ 0 ] ) / ( 2 * i.sizex() ),
                                      ( tr[ 1 ] + bf[ 1 ] ) / ( 2 * i.sizey() ) );
         }

         Direction getDirection() const
         {
            return _direction;
         }

         const core::vector2d& getCentre() const
         {
            return _centre;
         }

         const core::vector2d& getSize() const
         {
            return _size;
         }

         /**
          @brief Given an image and a feature (self), returns the actual bottom left & top right coordinates
          */
         void getPosition( const IntegralImage& i, double scale, core::vector2ui& bl, core::vector2ui& tr ) const
         {
            i32 sx = core::round( i.sizex() * scale * _size[ 0 ] );
            i32 sy = core::round( i.sizey() * scale * _size[ 1 ] );

            // we must use features that are centred
            if ( sx % 2 == 0 )
               ++sx;
            if ( sy % 2 == 0 )
               ++sy;

            const i32 halfx = sx / 2;
            const i32 halfy = sy / 2;

            const i32 cx = static_cast<i32>( i.sizex() * _centre[ 0 ] * scale );
            const i32 cy = static_cast<i32>( i.sizey() * _centre[ 1 ] * scale );

            bl[ 0 ] = std::max<int>( 0, cx - halfx );
            bl[ 1 ] = std::max<int>( 0, cy - halfy );

            tr[ 0 ] = std::min<int>( (int)i.sizex() - 1, cx + halfx );
            tr[ 1 ] = std::min<int>( (int)i.sizey() - 1, cy + halfy );
         }

         double getValue( const IntegralImage& i, const core::vector2ui& bl, const core::vector2ui& tr ) const
         {
            const int x1 = bl[ 0 ];
            const int x2 = tr[ 0 ];

            const int y1 = bl[ 1 ];
            const int y2 = tr[ 1 ];

            double sump;
            double sumd;
            ui32 mid;
            ui32 mid1, mid2, d;
            int border;

            switch ( _direction )
            {
            case HORIZONTAL:
               mid = ( x1 + x2 ) / 2;
               sump = i.getSum( core::vector2ui( x1, y1 ), core::vector2ui( mid, y2 ) );
               sumd = i.getSum( core::vector2ui( mid, y1 ), core::vector2ui( x2, y2 ) );
               return static_cast<double>( sump - sumd );

            case VERTICAL:
               mid = ( y1 + y2 ) / 2;
               sump = i.getSum( core::vector2ui( x1, y1 ), core::vector2ui( x2, mid ) );
               sumd = i.getSum( core::vector2ui( x1, mid ), core::vector2ui( x2, y2 ) );
               return static_cast<double>( sump - sumd );

            case VERTICAL_TRIPLE:
               ensure( y2 - y1 + 1 == x2 - x1 + 1, "the filter must be square" );
               //
               // note that we remove the border so that a filter of size 9x9
               // is a discrete approximation of a gaussian w = 1.2
               // compute: b border weight = 0
               //          + weight = 1
               //          - weight = 2
               // b+++b
               // b---b
               // b+++b
               //
               d = ( y2 - y1 + 1 ) / 3;
               mid1 = y1 + 1 * d;
               mid2 = y1 + 2 * d;
               border = ( ( x2 - x1 + 1 ) - ( 2 * d - 1 ) ) / 2; // actualFilterSize = 2 * d - 1, d = sizey / 3

               ensure( mid1 > 0 && mid2 > 0, "problem in feature position" );
               sump = i.getSum( core::vector2ui( x1 + border, y1 ),   core::vector2ui( x2 - border, y2 ) );
               sumd = i.getSum( core::vector2ui( x1 + border, mid1 ), core::vector2ui( x2 - border, mid2 - 1 ) );
               return static_cast<double>( sump - ( 2 + 1 ) * sumd ); // optim: 2 area computation only + weighting

            case HORIZONTAL_TRIPLE:
               ensure( y2 - y1 + 1 == x2 - x1 + 1, "the filter must be square" );
               //
               // note that we remove the border so that a filter of size 9x9
               // is a discrete approximation of a gaussian w = 1.2
               // compute: b border weight = 0
               //          + weight = 1
               //          - weight = 2
               // bbb
               // +-+
               // +-+
               // +-+
               // bbb
               d = ( x2 - x1 + 1 ) / 3;
               mid1 = x1 + 1 * d;
               mid2 = x1 + 2 * d;
               border = ( ( y2 - y1 + 1 ) - ( 2 * d - 1 ) ) / 2; // actualFilterSize = 2 * d - 1, d = sizey / 3

               ensure( mid1 > 0 && mid2 > 0, "problem in feature position" );
               sump = i.getSum( core::vector2ui( x1,   y1 + border ), core::vector2ui( x2,       y2 - border ) );
               sumd = i.getSum( core::vector2ui( mid1, y1 + border ), core::vector2ui( mid2 - 1, y2 - border ) );
               return static_cast<double>( sump - ( 2 + 1 ) * sumd ); // optim: 2 area computation only + weighting

            case CHECKER:
               ensure( y2 - y1 + 1 == x2 - x1 + 1, "the filter must be square" );
               //
               // note that we remove the border so that a filter of size 9x9
               // is a discrete approximation of a gaussian w = 1.2
               // compute: b border weight = 0
               //          + weight = 1
               //          - weight = 1
               // b b b b b
               // b + b - b
               // b - b + b
               // b b b b b
               //
               d = ( x2 - x1 + 1 ) / 3;
               mid1 = ( x2 + x1 ) / 2;
               mid2 = ( y2 + y1 ) / 2;
               border = ( ( x2 - x1 + 1 ) - ( 2 * d + 1 ) ) / 2;

               sump = i.getSum( core::vector2ui( x1 + border,   y1 + border ), core::vector2ui( mid1 - 1, mid2 - 1 ) ) +
                      i.getSum( core::vector2ui( mid1 + 1, mid2 + 1 ),         core::vector2ui( x2 - border, y2 - border ) );
               sumd = i.getSum( core::vector2ui( mid1 + 1,   y1 + border ),    core::vector2ui( x2 - border, mid2 - 1 ) ) +
                      i.getSum( core::vector2ui( x1 + border, mid2 + 1 ),      core::vector2ui( mid1 - 1, y2 - border ) );
               return static_cast<double>( sump - sumd );


            case NONE:
            default:
               ensure( 0, "not handled type" );
            }
         }

         IntegralImage::value_type getValue( const IntegralImage& i, double scale ) const
         {
            core::vector2ui bl;
            core::vector2ui tr;
            getPosition( i, scale, bl, tr );
            return getValue( i, bl, tr );
         }

         void write( std::ostream& f ) const
         {
            ensure( f.good(), "file not open correctly" );
            core::write<ui32>( static_cast<ui32>( _direction ), f );
            core::write<f64> ( _centre[ 0 ], f );
            core::write<f64> ( _centre[ 1 ], f );
            core::write<f64> ( _size[ 0 ], f );
            core::write<f64> ( _size[ 1 ], f );
         }

         void read( std::istream& f )
         {
            ui32 direction;

            core::read<ui32>( direction, f );
            core::read<f64> ( _centre[ 0 ], f );
            core::read<f64> ( _centre[ 1 ], f );
            core::read<f64> ( _size[ 0 ], f );
            core::read<f64> ( _size[ 1 ], f );
            _direction = static_cast<Direction>( direction );
         }

      private:
         // we need to store the centre/size and not bl,tr so that the scale is easily handled
         Direction         _direction;
         core::vector2d    _centre;
         core::vector2d    _size;
      };
      typedef std::vector<Feature>           Features;

   public:
      typedef double                         internal_type;
      typedef core::Buffer1D<internal_type>  Buffer;

   public:
      void clear()
      {
         _features.clear();
      }

      void add( const Feature& f )
      {
         _features.push_back( f );
      }

      void write( const std::string& f ) const
      {
         std::ofstream file( f.c_str(), std::ios::binary );
         ensure( file.good(), "can't open the file" );
         write( file );
      }

      void write( std::ostream& f ) const
      {
         ensure( f.good(), "file not open correctly" );
         core::write<ui32>( static_cast<ui32>( _features.size() ), f );
         for ( ui32 n = 0; n < static_cast<ui32>( _features.size() ); ++n )
         {
            _features[ n ].write( f );
         }
      }

      void read( const std::string& f )
      {
         std::ifstream file( f.c_str(), std::ios::binary );
         ensure( file.good(), "can't open the file" );
         read( file );
      }

      void read( std::istream& f )
      {
         ensure( f.good(), "file not open correctly" );

         ui32 size = 0;
         core::read<ui32>( size, f );
         _features = Features( size );
         for ( ui32 n = 0; n < size; ++n )
         {
            _features[ n ].read( f );
         }
      }

      /**
       @brief computes the Haar features on an integral image
       @param scale the scale applied to the features
       */
      Buffer process( const IntegralImage& i, double scale = 1.0 ) const
      {
         Buffer buffer( static_cast<ui32>( _features.size() ), false );
         for ( ui32 n = 0; n < buffer.size(); ++n )
         {
            buffer[ n ] = _features[ n ].getValue( i, scale );   
         }
         return buffer;
      }

      template <class T>
      Buffer process( const core::Image<T>& i, double scale = 1.0 ) const
      {
         IntegralImage integral;
         integral.process( i );
         return process( integral, scale );
      }

   private:
      Features    _features;
   };
}
}

# pragma warning( pop )

#endif
