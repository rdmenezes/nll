#ifndef NLL_FEATURE_TRANSFORMATION_DISCRIMINANT_H_
# define NLL_FEATURE_TRANSFORMATION_DISCRIMINANT_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a feature transformation algorithm usin Quadratic discriminants. One feature (i.e. discriminant) will
           be created for each class
    @see QuadraticDiscriminantAnalysis
    */
   template <class Point>
   class FeatureTransformationDiscriminant : public FeatureTransformation<Point>
   {
   public:
      typedef FeatureTransformation<Point>          Base;

   public:
      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureTransformationDiscriminant()
      {
      }

      ~FeatureTransformationDiscriminant()
      {
      }

      /**
       @brief Compute the principal components of the database. Only the <code>LEARNING|VALIDATION</code> are
              used.
       @return true if successful
       */
      template <class Database>
      bool compute( const Database& dat )
      {
         _qda.compute( dat );
         return true;
      }

      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const
      {
         return _qda.project( p );
      }
   
      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         _qda.read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         _qda.write( o );
      }

   private:
      QuadraticDiscriminantAnalysis    _qda;

   };
}
}

#endif
