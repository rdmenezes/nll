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
      virtual double interpolate( double x, double y, ui32 c ) const = 0;

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
      double interpolate( double x, double y, ui32 c ) const
      {
         double buf[ 4 ];

         x -= 0.5;
         y -= 0.5;
         const int xi = core::floor( x );
         const int yi = core::floor( y );

         const double dx = fabs( x - xi );
         const double dy = fabs( y - yi );

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
         double val = ( 1 - dy ) * ( ( 1 - dx ) * buf[ 0 ] + ( dx ) * buf[ 1 ] ) +
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
      void interpolateValues( float x, float y, Iterator output ) const
      {
         float buf[ 4 ];

         const int xi = static_cast<int>( std::floor( x ) );
         const int yi = static_cast<int>( std::floor( y ) );

         const float dx = fabs( x - xi );
         const float dy = fabs( y - yi );

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
         const float a0 = ( 1 - dx ) * ( 1 - dy );
         const float a1 = ( dx )     * ( 1 - dy );
         const float a2 = ( dx )     * ( dy );
         const float a3 = ( 1 - dx ) * ( dy );

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
      inline double interpolate( double x, double y, ui32 c ) const
      {
         // we need to add a bias factor due to rounding error. In this case the pixel
         // location could be both ways. In this case, we force to choose the right one
         // (it is also right to choose the left one, but we have to choose one way...)
         const double val = this->_img( (ui32)NLL_BOUND( ( x + NLL_IMAGE_BIAS ), 0, this->_img.sizex() - 1 ), (ui32)NLL_BOUND( ( y + NLL_IMAGE_BIAS ), 0, this->_img.sizey() - 1 ), c );
         assert( val >= Bound<T>::min && val <= (Bound<T>::max + 0.999) );
         return val;
      }

      /**
       @brief Helper method to do interpolation on multi valued values
       */
      template <class Iterator>
      void interpolateValues( float x, float y, Iterator output ) const
      {
         int xi = (int)( x + 0.5f );
         int yi = (int)( y + 0.5f );

         if ( xi < 0 || ( xi + 1 ) >= static_cast<int>( this->_img.sizex() ) ||
              yi < 0 || ( yi + 1 ) >= static_cast<int>( this->_img.sizey() ) )
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
