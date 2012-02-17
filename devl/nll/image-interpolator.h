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

#ifndef NLL_IMAGE_INTERPOLATOR_H_
# define NLL_IMAGE_INTERPOLATOR_H_

/// we define a bias so that all the pixels are shifted to the same direction. Due to rounding it is not necessary the case without this bias factor
# define NLL_IMAGE_BIAS    1e-5

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief 2D interpolator of an image

    For all Interoplators:
    - Ensure interpolated value is in the range of T
    - assume (x, y) : is exactly the corner of the pixel

    Virtual methods are used, but they do not generally imply performance loss
    for this particluar case: it is used statically, so a specific optimization
    will be used to avoid the virtual calls.
   */
   template <class T, class Mapper, class Alloc>
   class Interpolator2D
   {
   public:
      // if the volume is a floating point type, the interpolation is the same type
      // else a float
      typedef typename core::If<T, float, core::IsFloatingType<T>::value >::type value_type;

      typedef Image<T, Mapper, Alloc>   TImage;

      /**
       @brief wrap the image in an interpolator
       */
      Interpolator2D( const TImage& img ) : _img( img ){}

      /**
       @brief return the size the underlying image
       */
      ui32 sizex() const { return _img.sizex(); }

      /**
       @brief return the size the underlying image
       */
      ui32 sizey() const { return _img.sizey(); }

      /**
       @brief return the number of components of the underlying image
       */
      ui32 getNbComponents() const { return _img.getNbComponents(); }
      
      virtual ~Interpolator2D(){}

      /**
       @brief return the value of an interpolated point
       */
      virtual value_type interpolate( value_type x, value_type y, ui32 c ) const = 0;

   protected:
      Interpolator2D& operator=( const Interpolator2D& );

   protected:
      const TImage&    _img;
   };

   /**
    @ingroup core
    @brief 2D bilinear interpolation of an image. (0, 0) points to the center of the top left pixel
    */
   template <class T, class Mapper, class Alloc>
   class InterpolatorLinear2D : public Interpolator2D<T, Mapper, Alloc>
   {
   public:
      typedef Interpolator2D<T, Mapper, Alloc>   Base;
      InterpolatorLinear2D( const typename Base::TImage& i ) : Base( i ){}
      value_type interpolate( value_type x, value_type y, ui32 c ) const
      {
         value_type buf[ 4 ];

         const int xi = core::floor( x );
         const int yi = core::floor( y );

         const value_type dx = fabs( x - xi );
         const value_type dy = fabs( y - yi );

         // if we can't interpolate first & last line/column (we are missing one sample), just don't do any interpolation
         // and return background value
         if ( xi < 0 || ( xi + 1 ) >= static_cast<int>( this->_img.sizex() ) ||
              yi < 0 || ( yi + 1 ) >= static_cast<int>( this->_img.sizey() ) )
         {
            return 0;
         }
  
         typename Base::TImage::ConstDirectionalIterator iter = this->_img.getIterator( xi, yi, c );
         buf[ 0 ] = *iter;
         buf[ 1 ] = iter.pickx();
         buf[ 3 ] = iter.picky();
         iter.addx();
         buf[ 2 ] = iter.picky();

         // factorized form of:
         //double val = ( 1 - dx ) * ( 1 - dy ) * buf[ 0 ] +
         //             ( dx )     * ( 1 - dy ) * buf[ 1 ] +
         //             ( dx )     * ( dy )     * buf[ 2 ] +
         //             ( 1 - dx ) * ( dy )     * buf[ 3 ];
         value_type val = ( 1 - dy ) * ( ( 1 - dx ) * buf[ 0 ] + ( dx ) * buf[ 1 ] ) +
                          ( dy )     * ( ( dx )     * buf[ 2 ] + ( 1 - dx ) * buf[ 3 ] );

         // the first line|col is discarded as we can't really interpolate it correctly
         assert( val >= Bound<T>::min );
         assert( val <= ( Bound<T>::max + 0.999 ) );   // like 255.000000003, will be automatically truncated to 255
         return val;
      }

      /**
       @brief Helper method to do interpolation on multi valued values
       */
      template <class Iterator>
      void interpolateValues( value_type x, value_type y, Iterator output ) const
      {
         value_type buf[ 4 ];

         const int xi = static_cast<int>( std::floor( x ) );
         const int yi = static_cast<int>( std::floor( y ) );

         const value_type dx = fabs( x - xi );
         const value_type dy = fabs( y - yi );

         // if we can't interpolate first & last line/column (we are missing one sample), just don't do any interpolation
         // and return background value
         if ( xi < 0 || ( xi + 1 ) >= static_cast<int>( this->_img.sizex() ) ||
              yi < 0 || ( yi + 1 ) >= static_cast<int>( this->_img.sizey() ) )
         {
            for ( ui32 nbcomp = 0; nbcomp < this->_img.getNbComponents(); ++nbcomp )
               output[ nbcomp ] = 0;
            return;
         }
  
         // precompute coef
         const value_type a0 = ( 1 - dx ) * ( 1 - dy );
         const value_type a1 = ( dx )     * ( 1 - dy );
         const value_type a2 = ( dx )     * ( dy );
         const value_type a3 = ( 1 - dx ) * ( dy );

         typename Base::TImage::ConstDirectionalIterator iterOrig = this->_img.getIterator( xi, yi, 0 );
         for ( ui32 nbcomp = 0; nbcomp < this->_img.getNbComponents(); ++nbcomp, iterOrig.addcol() )
         {
            typename Base::TImage::ConstDirectionalIterator iter = iterOrig;
            buf[ 0 ] = *iter;
            buf[ 1 ] = iter.pickx();
            buf[ 3 ] = iter.picky();
            iter.addx();
            buf[ 2 ] = iter.picky();

            *output++ = a0 * buf[ 0 ] + a1 * buf[ 1 ] +
                        a2 * buf[ 2 ] + a3 * buf[ 3 ];
         }
      }
   };


   /**
    @ingroup core
    @brief 2D nearest neighbor interpolation of an image
    */
   template <class T, class Mapper, class Alloc>
   class InterpolatorNearestNeighbor2D : public Interpolator2D<T, Mapper, Alloc>
   {
   public:
      typedef Interpolator2D<T, Mapper, Alloc>   Base;
      InterpolatorNearestNeighbor2D( const typename Base::TImage& i ) : Base( i ){}
      value_type interpolate( value_type x, value_type y, ui32 c ) const
      {
         const value_type xf = x + 0.5f;
         const value_type yf = y + 0.5f;
         const int xi = static_cast<int>( xf );
         const int yi = static_cast<int>( yf );

         if ( xf < 0 || ( xi ) >= static_cast<int>( this->_img.sizex() ) ||
              yf < 0 || ( yi ) >= static_cast<int>( this->_img.sizey() ) )
         {
            return 0;
         }

         const value_type val = this->_img( xi, yi, c );
         return val;
      }

      /**
       @brief Helper method to do interpolation on multi valued values
       */
      template <class Iterator>
      void interpolateValues( value_type x, value_type y, Iterator output ) const
      {
         const value_type xf = x + 0.5f;
         const value_type yf = y + 0.5f;
         const int xi = static_cast<int>( xf );
         const int yi = static_cast<int>( yf );

         if ( xf < 0 || ( xi ) >= static_cast<int>( this->_img.sizex() ) ||
              yf < 0 || ( yi ) >= static_cast<int>( this->_img.sizey() ) )
         {
            for ( ui32 nbcomp = 0; nbcomp < this->_img.getNbComponents(); ++nbcomp )
               *output++ = 0;
            return;
         }

         typename Base::TImage::ConstDirectionalIterator iterOrig = this->_img.getIterator( xi, yi, 0 );
         for ( ui32 nbcomp = 0; nbcomp < this->_img.getNbComponents(); ++nbcomp )
            *output++ = iterOrig.pickcol( nbcomp );
      }
   };
}
}

#endif
