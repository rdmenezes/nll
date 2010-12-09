#ifndef NLL_ALGORITM_NORMALIZATION_H_
# define NLL_ALGORITM_NORMALIZATION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Normalize features of a sequence of points. Each feature is independantly normalized
           for (mean,variance)= (0,1)

           A point must define operator[](ui32) and ui32 size() and constructible(size)
    */
   template <class Point>
   class Normalize
   {
   public:
      typedef core::Buffer1D<double>   Vector;

   public:
      virtual ~Normalize()
      {}

      Normalize()
      {}

      Normalize( const Vector& mean, const Vector& var )
      {
         _mean.clone( mean );
         _var.clone( var );
      }

      template <class PPoint>
      Normalize( const Normalize<PPoint>& normalize )
      {
         _mean.clone( normalize.getMean() );
         _var.clone( normalize.getVariance() );
      }

      /**
       Compute the mean and variance of every features.

       Points must define const Point& operator[](ui32) and ui32 size() const
       */
      template <class Points>
      bool compute( const Points& points )
      {
         if ( !points.size() )
            return false;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "start compute normalization..." );

         const ui32 nbFeatures = (ui32)points[ 0 ].size();
         _mean = Vector( nbFeatures );
         _var = Vector( nbFeatures );
         ui32 nbSamples = static_cast<ui32>( points.size() );
         for ( ui32 n = 0; n < points.size(); ++n )
         {
            for ( ui32 nn = 0; nn < nbFeatures; ++nn )
               _mean[ nn ] += points[ n ][ nn ];
         }

         for ( ui32 nn = 0; nn < nbFeatures; ++nn )
            _mean[ nn ] /= nbSamples;

         for ( ui32 n = 0; n < points.size(); ++n )
         {
            for ( ui32 nn = 0; nn < nbFeatures; ++nn )
            {
               double val = points[ n ][ nn ] - _mean[ nn ];
               _var[ nn ] += val * val;
            }
         }

         for ( ui32 nn = 0; nn < nbFeatures; ++nn )
         {
            if ( fabs( _var[ nn ] ) < 1e-10 )
            {
               nllWarning( "null variance, this attribut should be discarded instead" );
               _var[ nn ] = 1;
            } else {
               _var[ nn ] /= nbSamples;
            }
         }

         std::stringstream ss1;
         ss1 << " mean vector=";
         std::stringstream ss2;
         ss2 << " variance vector=";

         for ( ui32 nn = 0; nn < nbFeatures; ++nn )
         {
            ss1 << _mean[ nn ] << " ";
            ss2 << _var[ nn ] << " ";
         }

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss1.str() );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss2.str() );
         return true;
      }

      /**
       @brief write the state of the class to a stream
       */
      bool write( std::ostream& o ) const
      {
         _mean.write( o );
         _var.write( o );
         return true;
      }

      /**
       @brief restore the state of the class from a stream
       */
      bool read( std::istream& i )
      {
         _mean.read( i );
         _var.read( i );
         return true;
      }

      /**
       @brief normalize a point
       */
      Point process( const Point& p ) const
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

   protected:
      Vector   _mean;
      Vector   _var;
   };
}
}

#endif
