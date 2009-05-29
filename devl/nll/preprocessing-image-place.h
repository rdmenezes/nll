#ifndef NLL_PREPROCESSING_IMAGE_H_
# define NLL_PREPROCESSING_IMAGE_H_

namespace nll
{
namespace preprocessing
{
   /**
    @ingroup preprocessing
    @brief create a new image that has at least a minimum size
   */
   template <class Type, class Mapper>
   class PreprocessingImagePlace : public Preprocessing<core::Image<Type, Mapper>, core::Image<Type, Mapper> >
   {
   public:
      typedef core::Image<Type, Mapper>         Point;
      typedef Preprocessing<Point, Point>       Base;
      typedef typename Base::InputDatabase      InputDatabase;
      typedef typename Base::OutputDatabase     OutputDatabase;
      typedef typename Base::OClassifier        OClassifier;

   public:
      /**
       @brief Specifies the minimal size and the background value.
       
       @param sizex the minimal width of the image.
       @param sizey the minimal heigh of the image.
       @param background the background of the image. ensure background is alive until the end of the algorithm
       */

      PreprocessingImagePlace( ui32 sizex, ui32 sizey, const Type* background ) : _sizex( sizex ), _sizey( sizey ), _background( background ){}

      virtual ~PreprocessingImagePlace(){}

      virtual Point process( const Point& p ) const
      {
         if ( p.sizex() >= _sizex && p.sizey() >= _sizey )
            return p;
         Point pp( _sizex, _sizey, p.getNbComponents(), false );
         for ( ui32 ny = 0; ny < pp.sizey(); ++ny )
            for ( ui32 nx = 0; nx < pp.sizex(); ++nx )
               pp.setPixel( nx, ny, _background );
         for ( ui32 c = 0; c < p.getNbComponents(); ++c )
            for ( ui32 ny = 0; ny < p.sizey(); ++ny )
               for ( ui32 nx = 0; nx < p.sizex(); ++nx )
                  pp( nx, ny, c ) = p( nx, ny, c );
         return pp;
      }

      /**
       @todo implement an optimizer (gridsearch).
       */
      virtual OutputDatabase optimize( const InputDatabase& dat, const OClassifier* , const typename OClassifier::ClassifierParameters& )
      {
         OutputDatabase ndat = processDatabase( dat );
         return ndat;
      }

      virtual void write( std::ostream& ) const
      {
         // nothing to do
      }

      virtual void read( std::istream& )
      {
         // nothing to do
      }

   private:
      ui32  _sizex;
      ui32  _sizey;
      const Type* _background;
   };
}
}

#endif
