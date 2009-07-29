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
      typedef core::Buffer1D<double>   Vector;

   public:
      typedef  FeatureTransformation<Point>  Base;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      /**
       @brief Computes the mean and variance independantly for each feature
       */
      bool compute( const Database& points )
      {
         if ( !points.size() )
            return false;

         const ui32 nbFeatures = (ui32)points[ 0 ].input.size();
         _mean = Vector( nbFeatures );
         _var = Vector( nbFeatures );
         ui32 nbSamples = 0;
         for ( ui32 n = 0; n < points.size(); ++n )
            if ( points[ n ].type == Database::Sample::LEARNING ||
                 points[ n ].type == Database::Sample::VALIDATION )
            {
               for ( ui32 nn = 0; nn < nbFeatures; ++nn )
                  _mean[ nn ] += points[ n ].input[ nn ];
               ++nbSamples;
            }
         if ( !nbSamples )
            return false;

         for ( ui32 nn = 0; nn < nbFeatures; ++nn )
            _mean[ nn ] /= nbSamples;

         for ( ui32 n = 0; n < points.size(); ++n )
            if ( points[ n ].type == Database::Sample::LEARNING ||
                 points[ n ].type == Database::Sample::VALIDATION )
            {
               for ( ui32 nn = 0; nn < nbFeatures; ++nn )
               {
                  double val = points[ n ].input[ nn ] - _mean[ nn ];
                  _var[ nn ] += val * val;
               }
            }

         for ( ui32 nn = 0; nn < nbFeatures; ++nn )
         {
            ensure( fabs( _var[ nn ] ) > 1e-10, "error: null variance, this attribut should be discarded instead" );
            _var[ nn ] /= nbSamples;
         }
         return true;
      }

      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const
      {
         ensure( p.size() == _var.size(), "error size" );

         Point res( p.size() );
         for ( ui32 nn = 0; nn < p.size(); ++nn )
            res[ nn ] = ( p[ nn ] - _mean[ nn ] ) / _var[ nn ];
         return res;
      }

      /**
       @return the mean for each feature
       */
      const Vector& getMean() const
      {
         return _mean;
      }

      /**
       @return the variance
       */
      const Vector& getVariance() const
      {
         return _var;
      }

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         _mean.read( i );
         _var.read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         _mean.write( o );
         _var.write( o );
      }

   protected:
      Vector   _mean;
      Vector   _var;
   };
}
}

#pragma warning( pop )

#endif
