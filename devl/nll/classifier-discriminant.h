#ifndef CLASSIFIER_DISCRIMINANT_H_
# define CLASSIFIER_DISCRIMINANT_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief quadratic discriminant analysis for classification
    */
   template <class Point>
   class ClassifierDiscriminant : public Classifier<Point>
   {
      typedef Classifier<Point>  Base;

   public:
      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

      // for gcc...
      typedef typename Base::Result                   Result;
      typedef typename Base::Database                 Database;
      typedef typename Base::Class                    Class;

   public:
      /**
        Create the parameter specification
             - no parameters
       */
      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         return parameters;
      }

   public:
      /**
       @param defines the weight decay to be used
       */
      ClassifierDiscriminant() : Base( buildParameters() )
      {}

      virtual typename Base::Classifier* deepCopy() const
      {
         ClassifierDiscriminant* c = new ClassifierDiscriminant();
         c->_qda = _qda;
         c->_crossValidationBin = this->_crossValidationBin;
         return c;
      }

      virtual void read( std::istream& i )
      {
         _qda.read( i );
      }

      virtual void write( std::ostream& o ) const
      {
         _qda.write( o );
      }

      virtual Class test( const Point& p ) const
      {
         return _qda.test( p );
      }

      virtual Class test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         return _qda.test( p, &probability );
      }

      /**
       param parameters:
             - no parameters
       */
      virtual void learn( const Database& dat, const core::Buffer1D<f64>& )
      {
         _qda.compute( dat );
      }

   private:
      QuadraticDiscriminantAnalysis _qda;
   };
}
}

#endif
