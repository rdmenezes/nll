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

   #define UNDEFINED_NB    99999.0f

   class CorrectPosition
   {
   public:
      typedef RegionResult::Measure                Measure;
      typedef std::vector<RegionResult::Measure>   Measures;
      typedef core::Matrix<float>                  Matrix;

      struct Template
      {
         Template( ui32 id,
                   float heightNeck,
                   float heightHeart,
                   float heightLung,
                   float heightSkull,
                   float heightHips )
         {
            distances = core::Buffer1D<float>( NB_CLASS );
            distances[ 1 ] = heightNeck;
            distances[ 2 ] = heightHeart;
            distances[ 3 ] = heightLung;
            distances[ 4 ] = heightSkull;
            distances[ 5 ] = heightHips;

            caseId = id;
         }

         ui32                    caseId;
         core::Buffer1D<float>   distances;
      };
      typedef std::vector<Template>    Templates;

   public:
      /*
      // the learning data file to use to detect and correct outliers
      CorrectPosition( const std::string& measures )
      {
         Measures m = RegionResult::readMeasures( measures );
         ensure( m.size(), "can't be empty: couldn't load the file" );

         for ( ui32 n = 0; n < m.size(); ++n )
         {
            _templates.push_back( Template( m[ n ].id,  m[ n ].heightNeck, m[ n ].heightHeart, m[ n ].heightLung, m[ n ].heightSkull, m[ n ].heightHips ) );
         }
      }*/

      CorrectPosition( const Measures& m )
      {
         for ( ui32 n = 0; n < m.size(); ++n )
         {
            _templates.push_back( Template( m[ n ].id,  m[ n ].heightNeck, m[ n ].heightHeart, m[ n ].heightLung, m[ n ].heightSkull, m[ n ].heightHips ) );
         }
      }

      static float distance( float x )
      {
         return 1 / ( 1 + exp( -15 + fabs( x ) / 4 ) );
      }

      /**
       @brief Measure the similarity between 2 cases
       */
      static float similarity( const Matrix& ref,
                               const Matrix& test )
      {
         float sum = 0;
         ensure( ref.sizex() == test.sizex() && ref.sizey() == ref.sizey(), "must be the same" );

         //ref.print( std::cout );
         //test.print( std::cout );

         for ( ui32 n = 1; n < NB_CLASS; ++n )  // start a 1, we don't care about the 0 label
         {
            for ( ui32 m = 1; m < n; ++m )
            {
               if ( ref( n, m ) < UNDEFINED_NB && test( n, m ) < UNDEFINED_NB )
               {
                  sum += distance( ref( n, m ) - test( n, m ) );
                  //std::cout << "sum(" << ref( n, m ) << "," << test( n, m ) << "=" << sum << std::endl;
               }
            }
         }
         return sum;
      }

      // return a matrix will all interdistances between he labels
      // matrix( x, y ) = signed distance y->x
      // matrix( x, y ) = -1 if the distance is not valid
      static Matrix getFullDistances( const core::Buffer1D<float>& labels )
      {
         //labels.print( std::cout );
         Matrix ref( NB_CLASS, NB_CLASS );
         for ( ui32 n = 0; n < ref.size(); ++n )
            ref[ n ] = UNDEFINED_NB;

         for ( ui32 n = 2; n < NB_CLASS; ++n )  // start a 1, we don't care about the 0 label
         {
            for ( ui32 m = 1; m < n; ++m )
            {
               if ( labels[ n ] > 0 && labels[ m ] > 0 )
               {
                  ref( n, m ) = labels[ m ] - labels[ n ];
               }
            }
         }
         //ref.print( std::cout );
         return ref;
      }

      /**
       @brief correct the different heights
       */
      void correct( const core::Buffer1D<float>& labels )
      {
         ensure( labels.size() == NB_CLASS, "must be the same" );

         // select the closest template
         int bestId = -1;
         float bestDist = (float)-1;

         Matrix test = getFullDistances( labels );
         for ( int n = 0; n < (int)_templates.size(); ++n )
         {
            Matrix t = getFullDistances( _templates[ n ].distances );
            float d = similarity( t, test );
            //std::cout << "similarity=" << d << std::endl;
            if ( d > bestDist )
            {
               bestDist = d;
               bestId = n;
            }
         }

         ensure( bestId >= 0, "error: can't find template" );

         std::cout << "best similarity=" << bestDist << " caseid=" << _templates[ bestId ].caseId << std::endl;
         if ( bestDist < CORRECTION_MIN_SIMILARITY )
         {
            std::cout << "abort case" << std::endl;
            return;
         }

         // check the proportions are ok
         std::vector<ui32> labelToUpdate;
         Matrix ref = getFullDistances( _templates[ bestId ].distances );
         int labelRef = -1;   // the label used as a reference
         float labelRefDist = (float)INT_MAX;

         ref.print( std::cout );
         test.print( std::cout );
         bool abort = false;
         for ( ui32 n = 1; n < NB_CLASS; ++n )
         {
            // sum all the distances for a label in ref & test
            float dtest = 0;
            float dref = 0;

            // vertical
            for ( ui32 u = n; u < NB_CLASS; ++u )
            {
               if ( ref( u, n ) < UNDEFINED_NB && test( u, n ) < UNDEFINED_NB )
               {
                  dtest += fabs( test( u, n ) );
                  dref += fabs( ref( u, n ) );

                  std::cout << "1drf=" << fabs( ref( u, n ) ) << std::endl;
                  std::cout << "1drft=" << fabs( test( u, n ) ) << std::endl;
               }
            }

            // horizontal
            for ( ui32 u = 1; u < n; ++u )
            {
               if ( ref( n, u ) < UNDEFINED_NB && test( n, u ) < UNDEFINED_NB )
               {
                  dtest += fabs( test( n, u ) );
                  dref += fabs( ref( n, u ) );

                  std::cout << "2drf=" << fabs( ref( n, u ) ) << std::endl;
                  std::cout << "2drft=" << fabs( test( n, u ) ) << std::endl;
               }
            }

            if ( fabs( dref ) > 0 )
            {
               float rate = fabs( dtest - dref ) / dref;
               std::cout << "label" << n << " error rate=" << rate << std::endl;
               
               if ( rate > CORRECTION_DETECTION_RATE )
               {
                  // we need to update this label
                  labelToUpdate.push_back( n );
               } else {
                  if ( fabs( rate - 1 ) < labelRefDist )
                  {
                     labelRefDist = fabs( rate - 1 );
                     labelRef = n;
                  }
               }
            }
         }

         std::cout << "label to update=";
         for ( ui32 n = 0; n < labelToUpdate.size(); ++n )
            std::cout << labelToUpdate[ n ] << " " << std::endl;

      }

   private:
      Templates    _templates;
   };
}
}

#endif