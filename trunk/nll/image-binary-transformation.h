#ifndef NLL_BINARY_TRANSFORMATION_H_
# define NLL_BINARY_TRANSFORMATION_H_

# include "utility.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Adds 2 points.
    */
   template <class T>
   struct BinaryAdd
   {
      inline void operator()(const T* c1, const T* c2, T* out, ui32 comp ) const
      {
         for ( ui32 n = 0; n < comp; ++n )
         {
            f32 sum = static_cast<f32>( c1[ n ] ) + static_cast<f32>( c2[ n ] );
            out[ n ] = (T)NLL_BOUND( sum, Bound<T>::min, Bound<T>::max );
         }
      }
   };

   /**
    @ingroup core
    @brief Substracts 2 points.
    */
   template <class T>
   struct BinarySub
   {
      inline void operator()(const T* c1, const T* c2, T* out, ui32 comp ) const
      {
         for ( ui32 n = 0; n < comp; ++n )
         {
            f32 sum = static_cast<f32>( c1[ n ] ) - static_cast<f32>( c2[ n ] );
            out[ n ] = (T)NLL_BOUND( sum, Bound<T>::min, Bound<T>::max );
         }
      }
   };

   /**
    @ingroup core
    @brief Selects the maximum of each component of the 2 points.
    */
   template <class T>
   struct BinaryMax
   {
      inline void operator()(const T* c1, const T* c2, T* out, ui32 comp ) const
      {
         for ( ui32 n = 0; n < comp; ++n )
         {
            f32 sum = static_cast<f32>( c1[ n ] ) + static_cast<f32>( c2[ n ] );
            out[ n ] = std::max( c1[ n ], c2[ n ] );
         }
      }
   };

   /**
    @ingroup core
    @brief Selects the minimum of each component of the 2 points.
    */
   template <class T>
   struct BinaryMin
   {
      inline void operator()(const T* c1, const T* c2, T* out, ui32 comp ) const
      {
         for ( ui32 n = 0; n < comp; ++n )
         {
            f32 sum = static_cast<f32>( c1[ n ] ) + static_cast<f32>( c2[ n ] );
            out[ n ] = std::min( c1[ n ], c2[ n ] );
         }
      }
   };

   /**
    @ingroup core
    @brief Transform the image with a defined operation
    @sa BinaryMin, BinaryMax, BinaryAdd, BinarySub.
    */
   template <class type, class mapper, class Transformation>
	Image<type, mapper> transform(const Image<type, mapper>& img1, const Image<type, mapper>& img2, const Transformation& transf)
	{
      assert( img1.sizex() == img2.sizex() );
      assert( img1.sizey() == img2.sizey() );
      assert( img1.getNbComponents() == img2.getNbComponents() );
		Image<type, mapper> tmp( img1.sizex(), img1.sizey(), img1.getNbComponents(), false );
		for ( ui32 y = 0; y < img1.sizey(); ++y )
			for ( ui32 x = 0; x < img1.sizex(); ++x )
			{
            const type* t1 = img1.point( x, y );
			   const type* t2 = img2.point( x, y );
            type* t3 = tmp.point( x, y );
				transf( t1, t2, t3, img1.getNbComponents() );
         }
		return tmp;
	}
}
}

#endif
