#ifndef NLL_PREPROCESSING_UNIT_H_
# define NLL_PREPROCESSING_UNIT_H_

namespace nll
{
namespace preprocessing
{
   /**
    @ingroup preprocessing
    @brief Preprocessing unit base class. All class used by the preprocessing pipeline
           has to inherit from this class.
   */
   template <class I, class O>
   class Preprocessing
   {
   public:
      typedef I                                 InputType;
      typedef O                                 OutputType;
      typedef algorithm::Classifier<I>          IClassifier;
      typedef algorithm::Classifier<O>          OClassifier;
      typedef typename IClassifier::Database    InputDatabase;
      typedef typename OClassifier::Database    OutputDatabase;

   public:
      /**
       @brief Process the input.
       @return the processed input.
      */
      virtual O process( const I& i ) const = 0;

      /**
       @brief Write the processing unit to a stream.
      */
      virtual void write( std::ostream& o ) const = 0;

      /**
       @brief Read the processing unit from a stream.
      */
      virtual void read( std::istream& i ) = 0;

      /**
       @brief Optimize the processing unit.

       @param i the input database used to optimize the unit.
       @param virtualClassifier the remaining of the Typelist is virtually viewed as a classifier
              we don't depend on the remaining of the Typelist.
       @param parameters the parameters of the actual classifier used for evaluation.
      */
      virtual OutputDatabase optimize( const InputDatabase& i, const OClassifier* virtualClassifier, const typename OClassifier::ClassifierParameters& parameters ) = 0;

      virtual ~Preprocessing()
      {}

   public:
      Preprocessing() : _id( core::IdMaker::instance().generateId() ) {}
      
      OutputDatabase processDatabase( const InputDatabase& i ) const
      {
         OutputDatabase odat;
         for ( ui32 n = 0; n < i.size(); ++n )
         {
            const typename InputDatabase::Sample& ref = i[ n ];
            typename OutputDatabase::Sample s;
            s.input = process( ref.input );
            s.output = ref.output;
            s.type = (typename OutputDatabase::Sample::Type)ref.type;
            s.debug = ref.debug;

            odat.add( s );
         }
         return odat;
      }

      ui32 getId() const { return _id; }

   private:
      ui32     _id;
   };
}
}

/*
   //
   // processing unit base template
   //
   template <class Type, class Mapper>
   class PreprocessingImageCenter : public Preprocessing<core::Image<Type, Mapper>, core::Image<Type, Mapper> >
   {
   public:
      typedef core::Image<Type, Mapper>   Point;

   public:
      PreprocessingImageCenter(){}

      virtual ~PreprocessingRawSelectBestFirst(){}

      virtual Point process( const Point& p ) const
      {
      }

      virtual OutputDatabase optimize( const InputDatabase& dat, const OClassifier* _classifier, const typename OClassifier::ClassifierParameters& params )
      {
         OutputDatabase ndat = processDatabase( dat );
         return ndat;
      }

      virtual void write( std::ostream& o ) const
      {
      }

      virtual void read( std::istream& i )
      {
      }
   };
*/
#endif
