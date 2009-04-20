#ifndef NLL_PREPROCESSING_CLASSIFIER_H_
# define NLL_PREPROCESSING_CLASSIFIER_H_

# pragma warning( push )
# pragma warning( disable:4512 ) // assignment operator not generated

namespace nll
{
namespace preprocessing
{
   /**
    @ingroup preprocessing
    @brief build a virtual classifier based on an actual one and a TypeList hierarchy

    The remaining of the Typelist is viewed as a classifier. This is only used internally.
    @sa Typelist#optimize()
    */
   template <class TypeList>
   class TypelistClassifier : public algorithm::Classifier<typename TypeList::T1::InputType>
   {
      typedef algorithm::Classifier<typename TypeList::ClassifierInput> ClassifierImpl;
      typedef typename ClassifierImpl::Point    ActualPoint;
      typedef typename ClassifierImpl::Database ActualDatabase;

      typedef algorithm::Classifier<typename TypeList::T1::InputType>   Classifier;
      typedef typename Classifier::Database                             Database;
      typedef typename Classifier::Class                                Class;
      typedef typename Classifier::Point                                Point;

   public:
      static algorithm::ParameterOptimizers buildParameters()
      {
         algorithm::ParameterOptimizers parameters;
         return parameters;
      }

   public:
      TypelistClassifier( TypeList& typelist, ClassifierImpl* actualClassifier ) : Classifier( buildParameters() ), _typelist( typelist ), _actualClassifier( actualClassifier )
      {
         assert( actualClassifier );
      }
      virtual Classifier* deepCopy() const
      {
         unreachable("must not be reached");
         return 0;
      }

      virtual void read( std::istream& )
      {
         unreachable("must not be reached");
      }

      virtual void write( std::ostream& ) const
      {
         unreachable("must not be reached");
      }

      virtual Class test( const Point& p ) const
      {
         ActualPoint ap = _typelist.process( p );
         return _actualClassifier->test( ap );
      }

      virtual void learn( const Database& dat, const core::Buffer1D<f64>& parameters )
      {
         ActualDatabase adat = _typelist.process( dat );
         _actualClassifier->learn( adat, parameters );
      }
      virtual double evaluate( const nll::core::Buffer1D<nll::f64>& params, const Database& dat ) const
      {
         ActualDatabase adat = _typelist.process( dat );
         return _actualClassifier->evaluate( params, adat );
      }

   private:
      ClassifierImpl*   _actualClassifier;
      const TypeList&   _typelist;
   };
}
}

# pragma warning( pop )

#endif
