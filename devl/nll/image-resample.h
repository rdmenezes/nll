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

#ifndef NLL_IMAGE_RESAMPLE_H_
# define NLL_IMAGE_RESAMPLE_H_

//
// TODO THIS FILE IS TO BE REWRITED USING TRANSFORMATION MAPPER
//
//
namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief resample an image using a specific interpolator and 2D affine transformation

    The source image and tfm3x3 is defining a source space : the source image is transformed using this transformation
    The target image is defined in the same space as the source image

    So the resampling basically a source image using this tfm and directly output it in the target image (as they are defined in the same space)

    Here we are simplifying the problem compared to the 3D case where the target is not defined in the same space.

    @param Interpolator interpolator used for resampling
    */
   template <class T, class IMapper, class Allocator, class Color, class Interpolator>
   void resample( const Image<T, IMapper, Allocator>& source, Image<T, IMapper, Allocator>& target, const Matrix<double>& tfm3x3, Color background )
   {
      ensure( tfm3x3.sizex() == 3 && tfm3x3.sizey() == 3, "the transformation matrix must be a 3x3" );
      ensure( source.getNbComponents() == target.getNbComponents(), "same NB of components only" );

      // we want to find the transformation from target->source
      Matrix<double> invTfm;
      invTfm.clone( tfm3x3 );
      inverse3x3( invTfm );

      // here we consider origin = (0,0) for both images
      Interpolator interpolator( source );
      vector2d dy( invTfm( 0, 2 ), invTfm( 1, 2 ) );  // start at the translation
      for ( ui32 y = 0; y < target.sizey(); ++y )
      {
         vector2d dx = dy;
         for ( ui32 x = 0; x < target.sizex(); ++x )
         {
            for ( ui32 c = 0; c < target.getNbComponents(); ++c )
            {
               if ( dx[ 0 ] < 0 || dx[ 0 ] >= source.sizex() ||
                    dx[ 1 ] < 0 || dx[ 1 ] >= source.sizey() )
               {
                  target( x, y, c ) = background[ c ];
               } else {
                  target( x, y, c ) = interpolator.interpolate( dx[ 0 ] - 0.5, dx[ 1 ] - 0.5, c );
               }
            }
            dx[ 0 ] += invTfm( 0, 0 );
            dx[ 1 ] += invTfm( 1, 0 );
         }
         dy[ 0 ] += invTfm( 0, 1 );
         dy[ 1 ] += invTfm( 1, 1 );
      }
   }

   /**
    @ingroup core
    @brief resample an image using a Bilinear filtering and 2D affine transformation
    */
   template <class T, class IMapper, class Allocator, class Color>
   void resampleBilinear( const Image<T, IMapper, Allocator>& source, Image<T, IMapper, Allocator>& target, const Matrix<double>& tfm3x3, Color background )
   {
      typedef InterpolatorLinear2D<T, IMapper, Allocator> Interpolator;
      resample<T, IMapper, Allocator, Color, Interpolator>( source, target, tfm3x3, background );
   }

   /**
    @ingroup core
    @brief resample an image using a Nearest Neighbour filter and 2D affine transformation
    */
   template <class T, class IMapper, class Allocator, class Color>
   void resampleNearestNeighbour( const Image<T, IMapper, Allocator>& source, Image<T, IMapper, Allocator>& target, const Matrix<double>& tfm3x3, Color background )
   {
      typedef InterpolatorNearestNeighbor2D<T, IMapper, Allocator> Interpolator;
      resample<T, IMapper, Allocator, Color, Interpolator>( source, target, tfm3x3, background );
   }

   /**
    @ingroup core
    @brief resample an image using a specific interpolator

    @param Interpolator interpolator used for resampling
    */
   template <class T, class IMapper, class Interpolator, class Allocator>
   void rescale( Image<T, IMapper, Allocator>& img, ui32 newSizeX, ui32 newSizeY, Allocator alloc = Allocator() )
   {
      typedef typename Interpolator::value_type CoordType;
      CoordType dxsize = static_cast<CoordType> ( img.sizex() - 0 ) / newSizeX;
		CoordType dysize = static_cast<CoordType> ( img.sizey() - 0 ) / newSizeY;
      Image<T, IMapper, Allocator> i( newSizeX, newSizeY, img.getNbComponents(), false, alloc );

      Interpolator interpolator( img );
	   for ( ui32 y = 0; y < newSizeY; ++y )
         for ( ui32 x = 0; x < newSizeX; ++x )
            for ( ui32 c = 0; c < img.getNbComponents(); ++c )
            {
               i( x, y, c ) = static_cast<T> ( interpolator.interpolate( x * dxsize - (CoordType)0.5, y * dysize - (CoordType)0.5, c ) );
            }
      img = i;
   }

   /**
    @ingroup core
    @brief resample an image using a bilinear interpolation.
    */
   template <class T, class IMapper, class Allocator>
   inline void rescaleBilinear( Image<T, IMapper, Allocator>& img, ui32 newSizeX, ui32 newSizeY, Allocator alloc = Allocator() )
   {
      rescale<T, IMapper, InterpolatorLinear2D<T, IMapper, Allocator>, Allocator>( img, newSizeX, newSizeY, alloc );
   }

   /**
    @ingroup core
    @brief resample an image using a nearest neighbor interpolation.
    */
   template <class T, class IMapper, class Allocator>
   inline void rescaleNearestNeighbor( Image<T, IMapper, Allocator>& img, ui32 newSizeX, ui32 newSizeY, Allocator alloc = Allocator() )
   {
      rescale<T, IMapper, InterpolatorNearestNeighbor2D<T, IMapper, Allocator>, Allocator>( img, newSizeX, newSizeY, alloc );
   }

   /**
    @ingroup core
    @brief resample an image a weighted grid.
    @note there will be 0.5 pixel shift as the pixel center is the lop left corner.
    
    The resampled image is the mean of all pixels in a specific cell of this grid.
    */
   template <class T, class IMapper, class Allocator>
   void rescaleFast( Image<T, IMapper, Allocator>& img, ui32 newSizeX, ui32 newSizeY, Allocator alloc = Allocator() )
   {
      Image<T, IMapper, Allocator> i( newSizeX, newSizeY, img.getNbComponents(), false, alloc );
		f64 dxsize = static_cast<f64> ( img.sizex() ) / newSizeX;
		f64 dysize = static_cast<f64> ( img.sizey() ) / newSizeY;

		assert( dxsize * dysize ); // "error: image too small"
      f64 divsize = ( ( ( dxsize < 1 ) ? 1 : (int)( dxsize + 1 ) ) *
                      ( ( dysize < 1 ) ? 1 : (int)( dysize + 1 ) ) );
      for ( ui32 c = 0; c < img.getNbComponents(); ++c )
		   for ( ui32 y = 0; y < newSizeY; ++y )
            for ( ui32 x = 0; x < newSizeX; ++x )
			   {
				   f64 val = 0;
				   for ( ui32 dx = 0; dx < dxsize; ++dx )
					   for ( ui32 dy = 0; dy < dysize; ++dy )
						   val += img(static_cast<ui32>( x * dxsize )  + dx, static_cast<ui32>( y * dysize ) + dy, c );
               val /= divsize;
               i( x, y, c ) = static_cast<T>( NLL_BOUND( val, ( T )Bound<T>::min, ( T )Bound<T>::max ) );
			   }
		img = i;
   }
}
}
#endif
