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
      template <class T, class Mapper, class Alloc>
      void process( const core::Image<T, Mapper, Alloc>& image )
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
       @note the coordinate are inclusive
       */
      value_type getSum( const core::vector2i& bottomLeft,
                         const core::vector2i& topRight ) const
      {
         value_type val1;
         value_type val2;
         value_type val3;
         value_type val4;

         core::vector2i bl( bottomLeft[ 0 ] - 1, bottomLeft[ 1 ] - 1 ); // shift by (-1, -1)
         if ( bl[ 0 ] < 0 || bl[ 1 ] < 0 )
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
    @brief Box filters approximating gaussian derivatives

    VERTICAL, HORIZONTAL for the first derivatives
    VERTICAL_TRIPLE, HORIZONTAL_TRIPLE, CHECKER for the second derivatives
    */
   class HaarFeatures2d
   {
   public:
      enum Direction
      {
         VERTICAL,
         HORIZONTAL,
         VERTICAL_TRIPLE,
         HORIZONTAL_TRIPLE,
         CHECKER,
         NONE
      };

      static double getValue( const Direction direction, const IntegralImage& i, const core::vector2i& position, const int lobeSize )
      {
         //
         // Note: all drawings are inverted in Y as the (0, 0) of our images is bottom left!
         //
         if ( direction == VERTICAL_TRIPLE )
         {
            // computes:
            // X 0 0 0 0 0 0 0 0     X = (0, 0)
            // 0 0 0 0 0 0 0 0 0
            // p p p n n n p p p 
            // p p p n n n p p p
            // p p p n n n p p p
            // p p p n n n p p p
            // p p p n n n p p p
            // 0 0 0 0 0 0 0 0 0
            // 0 0 0 0 0 0 0 0 0
            //       <=+=> lobeSize

            #ifdef NLL_SECURE
            ensure( ( lobeSize % 2 ) == 1, "must be a odd" );
            #endif

            const int halfLobe = lobeSize / 2;
            const int halfx = lobeSize + halfLobe;
            const int halfy = ( 2 * lobeSize - 1 ) / 2;

            const core::vector2i min( position[ 0 ] - halfx, position[ 1 ] - halfy );
            const core::vector2i max( position[ 0 ] + halfx, position[ 1 ] + halfy );

            const core::vector2i subMin( position[ 0 ] - halfLobe, position[ 1 ] - halfy );
            const core::vector2i subMax( position[ 0 ] + halfLobe, position[ 1 ] + halfy );

            const double sump = i.getSum( min, max );
            const double sumd = i.getSum( subMin, subMax );
            return static_cast<double>( sump - 3 * sumd ); // optim: 2 area computation only + weighting
         }

         if ( direction == HORIZONTAL_TRIPLE )
         {
            // computes:
            // X 0 p p p p p 0 0     X = (0, 0)
            // 0 0 p p p p p 0 0
            // 0 0 p p p p p 0 0
            // 0 0 n n n n n 0 0  -
            // 0 0 n n n n n 0 0  |  lobeSize
            // 0 0 n n n n n 0 0  -
            // 0 0 p p p p p 0 0
            // 0 0 p p p p p 0 0
            // 0 0 p p p p p 0 0

            #ifdef NLL_SECURE
            ensure( ( lobeSize % 2 ) == 1, "must be a odd" );
            #endif

            const int halfLobe = lobeSize / 2;
            const int halfy = lobeSize + halfLobe;
            const int halfx = ( 2 * lobeSize - 1 ) / 2;

            const core::vector2i min( position[ 0 ] - halfx, position[ 1 ] - halfy );
            const core::vector2i max( position[ 0 ] + halfx, position[ 1 ] + halfy );

            const core::vector2i subMin( position[ 0 ] - halfx, position[ 1 ] - halfLobe );
            const core::vector2i subMax( position[ 0 ] + halfx, position[ 1 ] + halfLobe );

            const double sump = i.getSum( min, max );
            const double sumd = i.getSum( subMin, subMax );
            return static_cast<double>( sump - 3 * sumd ); // optim: 2 area computation only + weighting
         }

         if ( direction == VERTICAL )
         {
            // computes:
            // X n n n 0 p p p p     X = (0, 0)
            // n n n n 0 p p p p
            // n n n n 0 p p p p
            // n n n n 0 p p p p
            // n n n n 0 p p p p
            // n n n n 0 p p p p
            // n n n n 0 p p p p
            // n n n n 0 p p p p
            // n n n n 0 p p p p
            // <===============> lobeSize

            #ifdef NLL_SECURE
            ensure( lobeSize % 2 == 1, "must be a odd" );
            #endif

            const int half = lobeSize / 2;

            const core::vector2i min( position[ 0 ] - half, position[ 1 ] - half );
            const core::vector2i max( position[ 0 ] - 1,    position[ 1 ] + half );

            const core::vector2i subMin( position[ 0 ] + 1,    position[ 1 ] - half );
            const core::vector2i subMax( position[ 0 ] + half, position[ 1 ] + half );

            const double sumd = i.getSum( min, max );
            const double sump = i.getSum( subMin, subMax );
            return static_cast<double>( sump - sumd );
         }

         if ( direction == HORIZONTAL )
         {
            // computes:
            // X n n n n n n n n     X = (0, 0)
            // n n n n n n n n n 
            // n n n n n n n n n
            // n n n n n n n n n
            // 0 0 0 0 0 0 0 0 0
            // p p p p p p p p p
            // p p p p p p p p p
            // p p p p p p p p p
            // p p p p p p p p p
            // <===============> lobeSize

            #ifdef NLL_SECURE
            ensure( lobeSize % 2 == 1, "must be a odd" );
            #endif

            const int half = lobeSize / 2;


            const core::vector2i min( position[ 0 ] - half, position[ 1 ] - half );
            const core::vector2i max( position[ 0 ] + half, position[ 1 ] - 1 );

            const core::vector2i subMin( position[ 0 ] - half, position[ 1 ] + 1 );
            const core::vector2i subMax( position[ 0 ] + half, position[ 1 ] + half );

            const double sumd = i.getSum( min, max );
            const double sump = i.getSum( subMin, subMax );
            return static_cast<double>( sump - sumd );
         }

         if ( direction == CHECKER )
         {
            // computes:
            // X 0 0 0 0 0 0 0 0    X = (0, 0)
            // 0 n n n 0 p p p 0
            // 0 n n n 0 p p p 0
            // 0 n n n 0 p p p 0
            // 0 0 0 0 0 0 0 0 0
            // 0 p p p 0 n n n 0
            // 0 p p p 0 n n n 0
            // 0 p p p 0 n n n 0
            // 0 0 0 0 0 0 0 0 0
            //           <=+=> lobeSize

            #ifdef NLL_SECURE
            ensure( ( lobeSize % 2 ) == 1, "must be a odd" );
            #endif

            const core::vector2i min1( position[ 0 ] - lobeSize, position[ 1 ] - lobeSize );
            const core::vector2i max1( position[ 0 ] - 1,        position[ 1 ] - 1 );

            const core::vector2i min2( position[ 0 ] + 1,        position[ 1 ] - lobeSize );
            const core::vector2i max2( position[ 0 ] + lobeSize, position[ 1 ] - 1 );

            const core::vector2i min3( position[ 0 ] - lobeSize, position[ 1 ] + 1 );
            const core::vector2i max3( position[ 0 ] - 1,        position[ 1 ] + lobeSize );

            const core::vector2i min4( position[ 0 ] + 1,        position[ 1 ] + 1 );
            const core::vector2i max4( position[ 0 ] + lobeSize, position[ 1 ] + lobeSize );

            const double sum1 = i.getSum( min1, max1 );
            const double sum2 = i.getSum( min2, max2 );
            const double sum3 = i.getSum( min3, max3 );
            const double sum4 = i.getSum( min4, max4 );
            return static_cast<double>( sum2 + sum3 - sum1 - sum4 ); // optim: 2 area computation only + weighting
         }

         ensure( 0, "not handled haar feature" );
      }
   };
}
}

# pragma warning( pop )

#endif
