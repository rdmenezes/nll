#ifndef NLL_PREPROCESSING_IMAGE_RESAMPLE_H_
# define NLL_PREPROCESSING_IMAGE_RESAMPLE_H_

namespace nll
{
namespace preprocessing
{
   /**
    @ingroup preprocessing
    @brief Resample a 2D Image to a fixed size.

    Resample the image using a bilinear filtering method.
    */
   template <class Type, class Mapper>
   class PreprocessingImageResample : public Preprocessing<core::Image<Type, Mapper>, core::Image<Type, Mapper> >
   {
   public:
      typedef core::Image<Type, Mapper>         Point;
      typedef Preprocessing<Point, Point>       Base;
      typedef typename Base::InputDatabase      InputDatabase;
      typedef typename Base::OutputDatabase     OutputDatabase;
      typedef typename Base::OClassifier        OClassifier;

   public:
      /**
       @brief Specifies the initial fixed size.
       */
      PreprocessingImageResample( ui32 newSizex, ui32 newSizey ) : _sizex( newSizex ), _sizey( newSizey ){ assert( _sizex && _sizey ); }

      virtual ~PreprocessingImageResample(){}

      /**
       @brief Process the input using bilinear filtering.
       @note use rescaleFast() as interpolator. Seems to be more effective for very small resampling
       */
      virtual Point process( const Point& p ) const
      {
         Point pp( p );
         core::rescaleFast( pp, _sizex, _sizey );
         //core::rescaleBilinear( pp, _sizex, _sizey );
         return pp;
      }

      /**
       @brief Optimize the preprocessing unit.
       @todo needs to implement using a gridsearch algorithm
       */
      virtual OutputDatabase optimize( const InputDatabase& dat, const OClassifier* , const typename OClassifier::ClassifierParameters& )
      {
         OutputDatabase ndat = processDatabase( dat );
         // FIXME : optimize the best resampling with a gridsearch (n, n)
         return ndat;
      }

      virtual void write( std::ostream& o ) const
      {
         core::write<ui32>( _sizex, o );
         core::write<ui32>( _sizey, o );
      }

      virtual void read( std::istream& i )
      {
         core::read<ui32>( _sizex, i );
         core::read<ui32>( _sizey, i );
      }

   private:
      ui32     _sizex;
      ui32     _sizey;
   };
}
}

#endif
