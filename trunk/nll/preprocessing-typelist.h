#ifndef NLL_PREPROCESSING_TYPELIST_H_
# define NLL_PREPROCESSING_TYPELIST_H_

# pragma warning( push )
# pragma warning( disable:4512 ) // assignment operator not generated
# pragma warning( disable:4251 ) // dll-interface
# pragma warning( disable:4127 ) // constant conditional expression

namespace nll
{
namespace preprocessing
{
   /**
    @ingroup preprocessing
    @brief Define the units to be optimized
    @sa Typelist
   */
   class NLL_API PreprocessingOptimizationScheme
   {
   public:
      typedef std::set<ui32>  Ids;

   public:
      /**
       @brief Init an empty optimization scheme. use add() to add Ids
       */
      PreprocessingOptimizationScheme(){}

      /**
       @brief Init the class using a list of IDs
       */
      PreprocessingOptimizationScheme( const Ids& idList ) : _ids( idList ){}

      /**
       @brief Add Preprocessing unit ID to be optimized
       */
      void add( ui32 id ){ _ids.insert( id ); }

      /**
       @brief Returns all the ids to be optimized
       */
      const Ids& getIds() const { return _ids; }

      /**
       @return True if the <code>id</code> is in the list of the preprocessing
               unit to be optimized
       */
      bool shouldBeOptimized( ui32 id ) const
      {
         Ids::const_iterator it = _ids.find( id );
         return it != _ids.end();
      }

   private:
      Ids      _ids;
   };

   /**
    @ingroup preprocessing
    @brief Define the end of a Typelist. Needs to reference a classifier.

    Define the end of the Typelist. NullTypelist needs to know a classifier,
    this classifier won't be released at the end of life of the Typelist.

    @sa Typelist
   */
   template <class TClassifier>
   struct NullTypelist
   {
   public:
      typedef typename TClassifier::Point InputType;
      typedef TClassifier                 Classifier;

   public:
      NullTypelist( Classifier* c ) : _classifier( c ){}

      Classifier* getClassifier() const { return _classifier; }

   private:
      Classifier*    _classifier;
   };

   //
   // forward declaration of elementary operations
   //
   template <class TypeList, class I, class O, int level>
   struct ProcessFeature;

   template <class TypeList>
   struct GetLastTypelistType;

   template <class TypeList>
   struct GetSize;

   template <class TypeList>
   class TypelistClassifier;

   /**
    @ingroup preprocessing
    @brief Define a preprocessing pipeline (all the operations needed from the input to the input of the classifier)
           defined at compile time.

    The Typelist define a preprocessing pipeline. It is composed of:
    - Preprocessing units.
    - NullTypelist to end the typelist.
    The Typelist is defined at compile type and strong type checking is ensured
    accross the Preprocessing units composing the pipeline and the classifier. All the preprocessing
    units are handled uniformly making possible to apply automatic optimization of the pipeline.

    @sa Preprocessing
   */
   template <class TT1, class TT2, class C>
   class Typelist
   {
   public:
      typedef typename C::Point              ClassifierInput;
      typedef TT1                            T1;
      typedef TT2                            T2;
      typedef C                              Classifier;
      typedef typename Classifier::Database  ClassifierDatabaseInput;
      typedef typename T1::InputDatabase     Database;

   public:
      /**
       @brief build recursively a typelist.
       @param val the actual Preprocessing unit
       @param next a new Typelist or a NullTypelist
      */
      Typelist( T1 val, T2 next ) : _val( val ), _next( next ){}

      /**
       @return a Preprocessing unit
       */
      const T1& getValue() const { return _val; }

      /**
       @return a Typelist or a NullTypelist
       */
      const T2& getNext() const { return _next; }

      /**
       @brief Process the input features following the whole preprocessing pipeline,
              ready to be used by the classifier.
       @param i the input.
       @return the preprocessed input. recursively returned by the preprocessing pipeline.
       */
      ClassifierInput process( const typename T1::InputType& i ) const
      {
         //ProcessFeature<int, int, int, 400>();
         //return ClassifierInput();

         return ProcessFeature<Typelist, typename T1::InputType, ClassifierInput, GetSize<Typelist>::value>::process(*this, i);
      }

      /**
       @brief Process a whole database from the input type down to the classifier input type
       */
      ClassifierDatabaseInput process( const Database& dat ) const
      {
         ClassifierDatabaseInput pdat;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            const typename Database::Sample& ref = dat[ n ];
            typename ClassifierDatabaseInput::Sample s;
            s.input = process( ref.input );
            s.output = ref.output;
            s.type = (typename ClassifierDatabaseInput::Sample::Type)ref.type;
            s.debug = ref.debug;

            pdat.add( s );
         }
         return pdat;
      }

      /**
       @return the NullTypelist of the preprocessing pipeline
       */
      NullTypelist<Classifier>& getNullTypelist()
      {
         return _getNullTypelist( core::Val2Type<core::Equal<T2, NullTypelist<Classifier> >::value>() );
      }

      /**
         @brief dispatch the optimization to the preprocessing units
         create a virtual view of the data by creating a classifier that will preprocess the database

         @param dat the input database of the pipeline
         @param params the parameter of the classifier
         @param scheme the units of the pipeline to be optimized
       */
      void optimize( const Database& dat, const typename Classifier::ClassifierParameters& params, const PreprocessingOptimizationScheme& scheme )
      {
         _optimize( dat, params, scheme, core::Val2Type<core::Equal<T2, NullTypelist<Classifier> >::value>() );
      }

      /**
       @brief return the list of Ids of the units belonging to the pipeline
       */
      PreprocessingOptimizationScheme::Ids getUnitsId() const
      {
         PreprocessingOptimizationScheme::Ids ids;
         return _getUnitsId( ids, core::Val2Type<core::Equal<T2, NullTypelist<Classifier> >::value>() );
      }

   //private:
      /**
       @brief Don't use. Use getUnitsId() instead.
       */
      PreprocessingOptimizationScheme::Ids _getUnitsId( PreprocessingOptimizationScheme::Ids& ids, core::Val2Type<false> /*IsEndOfTail*/ ) const
      {
         ids.insert( _val.getId() );
         return _next._getUnitsId<core::Equal<typename T2::T2, NullTypelist<Classifier> >::value>( ids );
      }

      /**
       @brief Don't use. Use getUnitsId() instead.
       */
      PreprocessingOptimizationScheme::Ids _getUnitsId( PreprocessingOptimizationScheme::Ids& ids, core::Val2Type<true> /*IsEndOfTail*/ ) const
      {
         ids.insert( _val.getId() );
         return ids;
      }

   private:
      // specific implementation if Typelist == NullTypelist
      NullTypelist<Classifier>& _getNullTypelist( core::Val2Type<true> )
      {
         return _next;
      }

      // specific implementation if Typelist != NullTypelist
      NullTypelist<Classifier>& _getNullTypelist( core::Val2Type<false> )
      {
         return _next.getNullTypelist();
      }

      // specific implementation if Typelist == NullTypelist
      void _optimize( const Database& dat, const typename Classifier::ClassifierParameters& params, const PreprocessingOptimizationScheme& scheme, core::Val2Type<true> /* isNullTypelist */ )
      {
         typename T1::OutputDatabase odat;
         if ( scheme.shouldBeOptimized( _val.getId() ) )
         {
            Classifier* actualClassifier = _next.getClassifier();
            odat = _val.optimize( dat, actualClassifier, params );
         }
      }

      // specific implementation if Typelist != NullTypelist
      void _optimize( const Database& dat, const typename Classifier::ClassifierParameters& params, const PreprocessingOptimizationScheme& scheme, core::Val2Type<false> /* isNullTypelist */ )
      {
         typename T1::OutputDatabase odat;
         if ( scheme.shouldBeOptimized( _val.getId() ) )
         {
            Classifier* actualClassifier = getNullTypelist().getClassifier();
            TypelistClassifier<T2> virtualClassifier( _next, actualClassifier );
            odat = _val.optimize( dat, &virtualClassifier, params );
         }
         else
            odat = _val.processDatabase( dat );
         _next.optimize( odat, params, scheme );
      }

   private:
      T1    _val;
      T2    _next;
   };
}
}

# pragma warning( pop )

#endif
