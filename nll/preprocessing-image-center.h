#ifndef NLL_PREPROCESSING_CENTER_H_
# define NLL_PREPROCESSING_CENTER_H_

namespace nll
{
namespace preprocessing
{
   /**
    @ingroup preprocessing
    @brief specifies if a pixel belongs to the background or not.

    If the pixel is far enough from the background color using euclidian distance.
    */
   template <class Type>
   class IsBackground
   {
   public:
      /**
       @param background caller must ensure background is alive when running
       @param nbComponents the number of components of the image
       */
      IsBackground( const Type* background, ui32 nbComponents ) : _background( background ), _nbComponents( nbComponents )
      {}

      double operator()( const Type* val ) const
      {
         assert( val && _background );
         return core::generic_norm2<Type*, f64>( (Type*)val, (Type*)_background, _nbComponents );
      }

   private:
      const Type* _background;
      ui32        _nbComponents;
   };

   /**
    @ingroup preprocessing
    @brief center the image using a weighting method

    The weighting method used: the euclidian distance of the pixel from the background
    */
   template <class Type, class Mapper, class IsBackground = IsBackground<Type> >
   class PreprocessingImageCenter : public Preprocessing<core::Image<Type, Mapper>, core::Image<Type, Mapper> >
   {
   public:
      typedef core::Image<Type, Mapper>         Point;
      typedef Preprocessing<Point, Point>       Base;
      typedef typename Base::InputDatabase      InputDatabase;
      typedef typename Base::OutputDatabase     OutputDatabase;
      typedef typename Base::OClassifier        OClassifier;

   public:
      PreprocessingImageCenter( const IsBackground& isBackground, const Type* background ) : _isBackground( isBackground ),  _background( background )
      {}

      virtual ~PreprocessingImageCenter(){}

      virtual Point process( const Point& p ) const
      {
         f64 dsum = 0;
         f64 dx = 0;
         f64 dy = 0;
         for ( ui32 ny = 0; ny < p.sizey(); ++ny )
            for ( ui32 nx = 0; nx < p.sizex(); ++nx )
            {
               f64 dist = _isBackground( p.point( nx, ny ) );
               dsum += dist;
               dx += nx * dist;
               dy += ny * dist;
            }
         if ( nll::core::equal<f64>( dsum, 0 ) )
            return p;
         dx /= dsum;
         dy /= dsum;

         Point p2( p );
         core::TransformationTranslation translation( core::vector2f( static_cast<f32>( -dx ) + p.sizex() / 2, static_cast<f32>( -dy ) + p.sizey() / 2 ) );
         core::transformUnaryNearestNeighbor( p2, translation, _background );
         return p2;
      }

      virtual OutputDatabase optimize( const InputDatabase& dat, const OClassifier* , const typename OClassifier::ClassifierParameters& )
      {
         // nothing to do
         return processDatabase( dat );
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
      IsBackground      _isBackground;
      const Type*       _background;
   };
}
}

#endif
