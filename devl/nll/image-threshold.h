#ifndef NLL_IMAGE_THRESHOLD
# define NLL_IMAGE_THRESHOLD

# include "image.h"

# pragma warning( push )
# pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Define a threshold operator (greater).
    */
   template <class T>
	class ThresholdGreater
	{
	public:
      /**
       @brief sets the threshold
       */
		ThresholdGreater(T threshold) : _threshold(threshold){}

      /**
       @return true if above the threshold
       */
		bool operator()(T test) const {return test > _threshold;}
	private:
		T			_threshold;
	};

   /**
    @ingroup core
    @brief Define a threshold operator (lower).
    */
	template <class T>
	class ThresholdLower
	{
	public:
      /**
       @brief sets the threshold
       */
		ThresholdLower(T threshold) : _threshold(threshold){}

      /**
       @return true if below the threshold
       */
		bool operator()(T test) const {return test < _threshold;}
	private:
		T			_threshold;
	};

   /**
    @ingroup core
    @brief Define a threshold operator (between).
    */
	template <class T>
	class ThresholdBetween
	{
	public:
      /**
       @brief sets the 2 thresholds
       */
		ThresholdBetween(T min, T max) : _min(min), _max(max){}

      /**
       @return true if it is inbetween
       */
		bool operator()(T test) const {return test >= _min && test <= _max;}
	private:
		T			_min;
		T			_max;
	};

   /**
    @ingroup core
    @brief Define a sum agregator.
    
    Tt computes the sum of  the pixel components. It is used to parametrized the threshold
    @sa threshold_generic
    */
	template <class type>
	struct AgregatorSum
	{
		static typename BiggerType<type>::type op(type* buf, ui32 nbComp)
		{
			typename BiggerType<type>::type tmp = 0;
			for (ui32 n = 0; n < nbComp; ++n)
				tmp += buf[n];
			return static_cast<typename BiggerType<type>::type> (tmp / nbComp);
		}
	};

   /**
    @ingroup core
    @brief Thresold an image.
    
    If ( Thresold(x, y) ) then we keep the pixel, else it is replaced by the background value
    @example threshold<ui8, Mapper, ThresholdLower<ui8>, AgregatorSum<ui8> >(image, mat::ThresholdLower<ui8>(127));
    */
	template <class type, class mapper, class Threshold, class Agregator>
	void threshold_generic(Image<type, mapper>& image, const Threshold& thr, const type* background = Image<type, mapper>::black())
	{
		Image<type, mapper> tmp(image.sizex(), image.sizey(), image.getNbComponents(), false);
		for (ui32 y = 0; y < image.sizey(); ++y)
			for (ui32 x = 0; x < image.sizex(); ++x)
			{
				type* t1 = image.point(x, y);
				type* t2 = tmp.point(x, y);
				typename BiggerType<type>::type tt = Agregator::op(t1, image.getNbComponents());
				if (thr(tt))
					for (ui32 c = 0; c < image.getNbComponents(); ++c)
						t2[c] = t1[c];
				else
					for (ui32 c = 0; c < image.getNbComponents(); ++c)
						t2[c] = background[c];
			}
		image = tmp;
	}

   /**
    @ingroup core
    @brief Thresold an image using a sum agregator.
    @sa AgregatorSum
    */
	template <class type, class mapper, class Threshold>
	void threshold(Image<type, mapper>& image, const Threshold& thr, const type* background = Image<type, mapper>::black())
	{
		threshold_generic<type, mapper, Threshold, AgregatorSum<type> >(image, thr, background);
	}


   /**
    @ingroup core
    @brief binarize an image according to a thresold.

    If threshold(x, y ) then the pixel (x, y ) is set to foreground else background
    */
	template <class type, class mapper, class Threshold, class Agregator>
	void binarize_generic(Image<type, mapper>& image, const Threshold& thr, const type* background = Image<type, mapper>::black(), const type* foreground = Image<type, mapper>::white())
	{
		Image<type, mapper> tmp(image.sizex(), image.sizey(), image.getNbComponents(), false);
		for (ui32 y = 0; y < image.sizey(); ++y)
			for (ui32 x = 0; x < image.sizex(); ++x)
			{
				type* t1 = image.point(x, y);
				type* t2 = tmp.point(x, y);
				typename BiggerType<type>::type tt = Agregator::op(t1, image.getNbComponents());
				if (thr(tt))
					for (ui32 c = 0; c < image.getNbComponents(); ++c)
						t2[c] = foreground[c];
				else
					for (ui32 c = 0; c < image.getNbComponents(); ++c)
						t2[c] = background[c];
			}
		image = tmp;
	}

   /**
    @ingroup core
    @brief binarize an image according to a thresold using a sum agregator.

    If threshold(x, y ) then the pixel (x, y ) is set to foreground else background
    */
	template <class type, class mapper, class Threshold>
	void binarize(Image<type, mapper>& image, const Threshold& thr, const type* background = Image<type, mapper>::black(), const type* foreground = Image<type, mapper>::white())
	{
		binarize_generic<type, mapper, Threshold, AgregatorSum<type> >(image, thr, background, foreground);
	}
}
}

#pragma warning( pop )

#endif
