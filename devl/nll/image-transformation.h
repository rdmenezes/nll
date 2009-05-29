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
       */
		TransformationRotation(f32 angle, vector2f vector) : _dx(-vector[0]), _dy(-vector[1]), _cosa(cos(-angle)), _sina(sin(-angle)){}
		vector2f operator()(vector2f point) const 
		{
			return vector2f
				   (
						(point[0] * _cosa - point[1] * _sina + _dx),
						(point[0] * _sina + point[1] * _cosa + _dy)
				   );
		}
	private:
		f32		_dx;
		f32		_dy;
		f32		_cosa;
		f32		_sina;
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
	template <class type, class mapper, class Transformation>
	void transformUnaryFast(Image<type, mapper>& img, const Transformation& transf, const type* background = Image<type, mapper>::black())
	{
		Image<type, mapper> tmp(img.sizex(), img.sizey(), img.getNbComponents(), false);
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
   template <class type, class mapper, class Transformation, class Interpolator>
	void transformUnary(Image<type, mapper>& img, const Transformation& transf, const type* background = Image<type, mapper>::black())
	{
		Image<type, mapper> tmp(img.sizex(), img.sizey(), img.getNbComponents(), false);
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
   template <class type, class mapper, class Transformation>
	inline void transformUnaryBilinear(Image<type, mapper>& img, const Transformation& transf, const type* background = Image<type, mapper>::black())
   {
      transformUnary<type, mapper, Transformation, InterpolatorLinear2D<type, mapper> >( img, transf, background );
   }

   /**
    @ingroup core
    @brief Define the transformation operation using a nearest neighbor interpolation.
    */
   template <class type, class mapper, class Transformation>
	inline void transformUnaryNearestNeighbor(Image<type, mapper>& img, const Transformation& transf, const type* background = Image<type, mapper>::black())
   {
      transformUnary<type, mapper, Transformation, InterpolatorNearestNeighbor2D<type, mapper> >( img, transf, background );
   }
   
}
}

#endif
