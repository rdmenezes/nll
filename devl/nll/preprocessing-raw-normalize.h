#ifndef NLL_PREPROCESSING_RAW_NORMALIZE_H_
# define NLL_PREPROCESSING_RAW_NORMALIZE_H_

namespace nll
{
namespace preprocessing
{
   /**
    @ingroup preprocessing
    @brief normalize an input vector.

    The input vector is normalized so that for training database, the feature n will have zero mean and 1 variance.
    */
   template <class Point>
   class PreprocessingRawNormalize : public Preprocessing<Point, Point>
   {
   public:
      typedef core::Buffer1D<f64>   Vector;
      typedef Preprocessing<Point, Point>       Base;
      typedef typename Base::InputDatabase      InputDatabase;
      typedef typename Base::OutputDatabase     OutputDatabase;
      typedef typename Base::OClassifier        OClassifier;

   public:
      virtual ~PreprocessingRawNormalize(){}
      
      /**
       @brief Define a default PreprocessingRawNormalize with no mean/variance.
       
       Use optimize() before using process()
       */
      PreprocessingRawNormalize(){}

      /**
       @brief contruct the normalization unit with a deafult mean and variance for each feature.
       */
      PreprocessingRawNormalize( const Vector& mean, const Vector& variance ) : _mean( mean ), _variance( variance )
      {
         assert( mean.size() == variance.size() );
      }

      virtual Point process( const Point& p ) const
      {
         assert( _mean.size() && _variance.size() );
         Point p2( p.size() );
         for ( ui32 n = 0; n < p.size(); ++n )
            p2[ n ] = static_cast<typename Point::value_type>( ( p[ n ] - _mean[ n ] ) / _variance[ n ] );
         return p2;
      }

      /**
       @brief Optimize the unit using a database so the feature n has zero mean and 1 variance.
       */
      virtual OutputDatabase optimize( const InputDatabase& dat, const OClassifier* , const typename OClassifier::ClassifierParameters& )
      {
         if ( !dat.size() )
            return dat;
         ui32 size = dat[ 0 ].input.size();
         _mean = Vector( size );
         _variance = Vector( size );
         for ( ui32 n = 0; n < dat.size(); ++n )
            core::generic_add<Vector, Point>( _mean, dat[ n ].input, size );
         core::generic_div_cte<Vector>( _mean, dat.size(), size );

         for ( ui32 n = 0; n < size; ++n )
         {
            f64 tmp = 0;
            for ( ui32 nn = 0; nn < dat.size(); ++nn )
               tmp += core::sqr( dat[ nn ].input[ n ] - _mean[ n ] );
            _variance[ n ] = sqrt( tmp / ( dat.size() - 1 ) );
         }

         // reprocess the database with the new parameters
         return processDatabase( dat );
      }

      virtual void write( std::ostream& o ) const
      {
         _mean.write( o );
         _variance.write( o );
      }

      virtual void read( std::istream& i )
      {
         _mean.read( i );
         _variance.read( i );
      }
   protected:
      Vector      _mean;
      Vector      _variance;
   };
}
}

#endif
