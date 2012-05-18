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

#ifndef NLL_IMAGE_TRANSFORMATION_H_
# define NLL_IMAGE_TRANSFORMATION_H_

# include "image.h"
# include "static-vector.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Define a tranlation tranformation
    */
   class TransformationTranslation
	{
	public:
		TransformationTranslation(vector2f vector) : _dx(-vector[0]), _dy(-vector[1]){}
		vector2f operator()(vector2f point) const {return vector2f(point[0] + _dx, point[1] + _dy);}
	private:
		f32 _dx;
		f32 _dy;
	};

   /**
    @ingroup core
    @brief Define a rotation
    */
	class TransformationRotation
	{
	public:
      /**
       @param angle angle in radian
       @param translation the translation applied after the rotation
       @param rotationCenter the center of the rotation
       */
		TransformationRotation(f32 angle, vector2f translation, vector2f rotationCenter = vector2f()) : _dx(-translation[0]), _dy(-translation[1]), _cosa(cos(-angle)), _sina(sin(-angle)), _rotationCenter( rotationCenter ){}
		vector2f operator()(vector2f point) const 
		{
         const float x = point[ 0 ] - _rotationCenter[ 0 ];
         const float y = point[ 1 ] - _rotationCenter[ 1 ];

			return vector2f( (x * _cosa - y * _sina + _dx),
						        (x * _sina + y * _cosa + _dy) );
		}
	private:
		f32		_dx;
		f32		_dy;
		f32		_cosa;
		f32		_sina;

      core::vector2f _rotationCenter;
	};

   /**
    @ingroup core
    @brief Define a scaling tranformation
    */
	class TransformationScale
	{
	public:
		TransformationScale(f32 scale, vector2f vector = vector2f(0, 0)) : _scale( 1 / scale), _dx(-vector[0]), _dy(-vector[1]){}
		vector2f operator()(vector2f point) const {return vector2f((point[0] * _scale + _dx), (point[1] * _scale + _dy));}
	private:
		f32		_scale;
		f32		_dx;
		f32		_dy;
	};

   /**
    @ingroup core
    @brief Define a transformation that inverse the x-axis
    */
	class TransformationInverseX
	{
	public:
		TransformationInverseX(ui32 imageSizeX) : _size(static_cast<f32>( imageSizeX - 1 ) ){}
		vector2f operator()(vector2f point) const {return vector2f(_size - point[0], point[1]);}
	private:
		f32		_size;
	};

   /**
    @ingroup core
    @brief Define a transformation that inverse the y-axis
    */
	class TransformationInverseY
	{
	public:
		TransformationInverseY(ui32 imageSizeY) : _size(static_cast<f32>( imageSizeY - 1 ) ){}
		vector2f operator()(vector2f point) const {return vector2f(point[0], _size - point[1]);}
	private:
		f32		_size;
	};

   /**
    @ingroup core
    @brief Define the transformation operation. No interpolation is done.
    */
	template <class type, class mapper, class Transformation, class allocator>
	void transformUnaryFast(Image<type, mapper, allocator>& img, const Transformation& transf, const type* background = Image<type, mapper, allocator>::black(), allocator alloc = allocator() )
	{
		Image<type, mapper, allocator> tmp(img.sizex(), img.sizey(), img.getNbComponents(), false, alloc);
		for (ui32 y = 0; y < img.sizey(); ++y)
			for (ui32 x = 0; x < img.sizex(); ++x)
			{
				vector2f vec = transf(vector2f((f32)x, (f32)y));
				if (vec[0] < 0 || vec[0] >= static_cast<int>(img.sizex()) || vec[1] < 0 || vec[1] >= static_cast<int>(img.sizey()))
            {
               type* t2 = tmp.point(x, y);
					for (ui32 c = 0; c < img.getNbComponents(); ++c)
						t2[c] = background[c];
            }
				else
            {
               type* t1 = img.point(static_cast<ui32>( vec[0] ), static_cast<ui32>( vec[1] ) );
				   type* t2 = tmp.point(x, y);
					for (ui32 c = 0; c < img.getNbComponents(); ++c)
						t2[c] = t1[c];
            }
			}
		img = tmp;
	}


   /**
    @ingroup core
    @brief Define the transformation operation. Interpolation is done.
    */
   template <class type, class mapper, class Transformation, class Interpolator, class allocator>
	void transformUnary(Image<type, mapper, allocator>& img, const Transformation& transf, const type* background = Image<type, mapper, allocator>::black(), allocator alloc = allocator())
	{
		Image<type, mapper, allocator> tmp(img.sizex(), img.sizey(), img.getNbComponents(), false, alloc);
      Interpolator interpolator( img );
		for (ui32 y = 0; y < img.sizey(); ++y)
			for (ui32 x = 0; x < img.sizex(); ++x)
			{
				vector2f vec = transf(vector2f(static_cast<f32>( x ), static_cast<f32>( y ) ));
				if (vec[0] < 0 || vec[0] >= static_cast<int>(img.sizex()) || vec[1] < 0 || vec[1] >= static_cast<int>(img.sizey()))
            {
               type* t2 = tmp.point(x, y);
					for (ui32 c = 0; c < img.getNbComponents(); ++c)
						t2[c] = background[c];
            }
				else
            {
				   type* t2 = tmp.point(x, y);
					for (ui32 c = 0; c < img.getNbComponents(); ++c)
                  t2[c] = static_cast<type> ( interpolator.interpolate(vec[0], vec[1], c) );
            }
			}
		img = tmp;
	}

   /**
    @ingroup core
    @brief Define the transformation operation using a bilinear interpolation
    */
   template <class type, class mapper, class Transformation, class allocator>
	inline void transformUnaryBilinear(Image<type, mapper, allocator>& img, const Transformation& transf, const type* background = Image<type, mapper, allocator>::black(), allocator alloc = allocator())
   {
      transformUnary<type, mapper, Transformation, InterpolatorLinear2D<type, mapper, allocator> >( img, transf, background, alloc );
   }

   /**
    @ingroup core
    @brief Define the transformation operation using a nearest neighbor interpolation.
    */
   template <class type, class mapper, class Transformation, class allocator>
	inline void transformUnaryNearestNeighbor(Image<type, mapper, allocator>& img, const Transformation& transf, const type* background = Image<type, mapper, allocator>::black(), allocator alloc = allocator())
   {
      transformUnary<type, mapper, Transformation, InterpolatorNearestNeighbor2D<type, mapper, allocator> >( img, transf, background, alloc );
   }
   
}
}

#endif
