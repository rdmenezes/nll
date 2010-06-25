#ifndef CORRECTION_H_
# define CORRECTION_H_

# include "globals.h"
# include "read-result.h"

namespace nll
{
namespace detect
{
   /**
    @brief Detect and correct the position 
    */
   class CorrectPosition
   {
   public:
      typedef RegionResult::Measure                Measure;
      typedef std::vector<RegionResult::Measure>   Measures;
      typedef core::Matrix<float>                  Matrix;

   public:
      // the learning data file to use to detect and correct outliers
      CorrectPosition( const std::string& measures )
      {
         _measures = RegionResult::readMeasures( measures );
         ensure( _measures.size(), "can't be empty: couldn't load the file" );
      }

      /**
       @brief Measure the similarity between 2 cases
       */
      float distance( const Measure& m,
                      float heightNeck,
                      float heightHeart,
                      float heightLung,
                      float heightSkull,
                      float heightHips )
      {
         float sum = 0;
         bool null = true;

         if ( heightNeck > 0 && m.heightNeck > 0 )
         {
            sum += fabs( heightNeck - m.heightNeck );
            null = false;
         }

         if ( heightHeart > 0 && m.heightHeart > 0 )
         {
            sum += fabs( heightHeart - m.heightHeart );
            null = false;
         }

         if ( heightLung > 0 && m.heightLung > 0 )
         {
            sum += fabs( heightLung - m.heightLung );
            null = false;
         }

         if ( heightSkull > 0 && m.heightSkull > 0 )
         {
            sum += fabs( heightSkull - m.heightSkull );
            null = false;
         }

         if ( heightHips > 0 && m.heightHips > 0 )
         {
            sum += fabs( heightHips - m.heightHips );
            null = false;
         }

         if ( null )
            return (float)INT_MAX;
         return sum;
      }

      /**
       @brief correct the different heights
       */
      void correct( float& heightNeck,
                    float& heightHeart,
                    float& heightLung,
                    float& heightSkull,
                    float& heightHips )
      {
         // select the closest template
         int bestId = -1;
         float bestDist = (float)INT_MAX;
         for ( int n = 0; n < _measures.size(); ++n )
         {
            float d = distance( _measures[ n ], heightNeck, heightHeart, heightLung, heightSkull, heightHips );
            if ( d < bestDist )
            {
               bestDist = d;
               bestId = n;
            }
         }
         ensure( bestId > 0, "error: can't find template" );

         // check the proportions are ok
         Matrix ref( NB_CLASS, NB_CLASS );
         float dref[ NB_CLASS ] =
         {
            0,
            _measures[ bestId ].heightNeck,
            _measures[ bestId ].heightHeart,
            _measures[ bestId ].heightLung,
            _measures[ bestId ].heightSkull,
            _measures[ bestId ].heightHips,
         };

         Matrix test( NB_CLASS, NB_CLASS );
         float dtest[ NB_CLASS ] =
         {
            0,
            heightNeck,
            heightHeart,
            heightLung,
            heightSkull,
            heightHips,
         };

         for ( ui32 n = 1; n < NB_CLASS; ++n )
         {
            for ( ui32 m = 1; n < m; ++n )
            {
               if ( test[ n ] > 0 && test[ m ] > 0 )
               {
                  test( n, m ) = 0;
               } else {
                  test( n, m ) = -1;
               }
            }
         }

      }

   private:
      Measures    _measures;
   };
}
}

#endif