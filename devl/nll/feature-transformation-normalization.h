#ifndef FEATURE_TRANSFORMATION_NORMALIZATION_H_
# define FEATURE_TRANSFORMATION_NORMALIZATION_H_

#pragma warning( push )
#pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief This preprocessing class will normalize a database so that the mean( input ) = 0
    and variance( input ) = 1

    For each feature, apply f_x = ( f_x - mean( f_x ) ) / var( f_x )
    */
   template <class Point>
   class FeatureTransformationNormalization : public FeatureTransformation<Point>
   {
      typedef Normalize<Point>               Impl;
   public:
      typedef core::Buffer1D<double>         Vector;
      typedef FeatureTransformation<Point>   Base;
      typedef typename Base::Database        Database;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      /**
       @brief Computes the mean and variance independantly for each feature. Only the
              LEARNING|VALIDATION data are used to compute the parameters.
       */
      bool compute( const Database& points )
      {
         Database learningDat = core::filterDatabase( points,
                                                      core::make_vector<nll::ui32>( Database::Sample::LEARNING,
                                                                                    Database::Sample::VALIDATION ),
                                                      Database::Sample::LEARNING );
         core::DatabaseInputAdapterRead<Database> databaseAdapter( learningDat );
         return _impl.compute( databaseAdapter );
      }

      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const
      {
         return _impl.process( p );
      }

      /**
       @return the mean for each feature
       */
      const Vector& getMean() const
      {
         return _impl.getMean();
      }

      /**
       @return the variance
       */
      const Vector& getVariance() const
      {
         return _impl.getVariance();
      }

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         _impl.read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         _impl.write( o );
      }

   protected:
      Impl     _impl;
   };
}
}

#pragma warning( pop )

#endif
