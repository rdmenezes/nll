#ifndef NLL_PREPROCESSING_BRIDGE_IMAGE_VECTOR_H_
# define NLL_PREPROCESSING_BRIDGE_IMAGE_VECTOR_H_

namespace nll
{
namespace preprocessing
{
   /**
    @ingroup preprocessing
    @brief Build a bridge to go from an Image to a Vector. Just do nothing. (optimized if value_type of vector and buffer are equal)
    */
   template <class Type, class Mapper, class VectorType, class VectorMapper = core::IndexMapperFlat1D>
   class PreprocessingBridgeImageVector : public Preprocessing<core::Image<Type, Mapper>, core::Buffer1D<VectorType, VectorMapper> >
   {
   public:
      typedef core::Image<Type, Mapper>                  PointI;
      typedef core::Buffer1D<VectorType, VectorMapper>   PointO;

      typedef Preprocessing<PointI, PointO>     Base;
      typedef typename Base::InputDatabase      InputDatabase;
      typedef typename Base::OutputDatabase     OutputDatabase;
      typedef typename Base::OClassifier        OClassifier;

   public:
      PreprocessingBridgeImageVector(){}

      virtual ~PreprocessingBridgeImageVector(){}

      virtual PointO process( const PointI& p ) const
      {
         PointO v( p.size(), false );
         for ( ui32 n = 0; n < p.size(); ++n )
            v[ n ] = static_cast<VectorType>( p[ n ] );
         return v;
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
   };

   template <class Type, class Mapper, class VectorMapper>
   class PreprocessingBridgeImageVector<Type, Mapper, Type, VectorMapper> : public Preprocessing<core::Image<Type, Mapper>, core::Buffer1D<Type, VectorMapper> >
   {
   public:
      typedef core::Image<Type, Mapper>            PointI;
      typedef core::Buffer1D<Type, VectorMapper>   PointO;

      typedef Preprocessing<PointI, PointO>     Base;
      typedef typename Base::InputDatabase      InputDatabase;
      typedef typename Base::OutputDatabase     OutputDatabase;
      typedef typename Base::OClassifier        OClassifier;

   public:
      PreprocessingBridgeImageVector(){}

      virtual ~PreprocessingBridgeImageVector(){}

      virtual PointO process( const PointI& p ) const
      {
         return p;
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
   };
}
}

#endif
