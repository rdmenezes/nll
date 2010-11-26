#ifndef CLASSIFIER_NAIVE_BAYES_H_
# define CLASSIFIER_NAIVE_BAYES_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief quadratic discriminant analysis for classification
    */
   template <class Point>
   class ClassifierNaiveBayes : public Classifier<Point>
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
      ClassifierNaiveBayes() : Base( buildParameters() )
      {}

      virtual typename Base::Classifier* deepCopy() const
      {
         ClassifierNaiveBayes* c = new ClassifierNaiveBayes();
         c->_nb = _nb;
         c->_crossValidationBin = this->_crossValidationBin;
         return c;
      }

      virtual void read( std::istream& i )
      {
         _nb.read( i );
      }

      virtual void write( std::ostream& o ) const
      {
         _nb.write( o );
      }

      virtual Class test( const Point& p ) const
      {
         return _nb.test( p );
      }

      virtual Class test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         return _nb.test( p, &probability );
      }

      /**
       param parameters:
             - no parameters
       */
      virtual void learn( const Database& dat, const core::Buffer1D<f64>& )
      {
         _nb.compute( dat );
      }

   private:
      NaiveBayes _nb;
   };
}
}

#endif
