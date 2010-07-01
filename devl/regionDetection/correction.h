#ifndef CORRECTION_H_
# define CORRECTION_H_

# include "globals.h"
# include "read-result.h"
# include <map>

# ifndef UNDEFINED_NB
#  define UNDEFINED_NB    99999.0f
# endif

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

         // the maximum error possible for the distance ratio ROI template/test
         _thresholdInterDistance = Matrix( NB_CLASS, NB_CLASS );
         _thresholdInterDistance( 2, 1 ) = 0.2f;
         _thresholdInterDistance( 3, 1 ) = 0.2f;
         _thresholdInterDistance( 4, 1 ) = 0.2f;
         _thresholdInterDistance( 5, 1 ) = 0.2f;

         _thresholdInterDistance( 3, 2 ) = 0.2f;
         _thresholdInterDistance( 4, 2 ) = 0.2f;
         _thresholdInterDistance( 5, 2 ) = 0.2f;

         _thresholdInterDistance( 4, 3 ) = 0.2f;
         _thresholdInterDistance( 5, 3 ) = 0.2f;

         _thresholdInterDistance( 5, 4 ) = 0.1f;
      }

      static float distance( float x )
      {
         return 1 / ( 1 + exp( -10 + fabs( 0.5f * x )  ) );
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
       -find a matching template
       -vote for suitable/unsuitable ROI
       -detect a pivot, used as base to correct the other ROI
       -correct the ROI based on the shape distance ratio
       -add missing ROI if necessary
       */
      void correct( core::Buffer1D<float>& labels, ui32 maxIter = 10 )
      {
         core::Buffer1D<float> origLabels;
         origLabels.clone( labels );

         struct Pivot
         {
            Pivot( ui32 p1, ui32 p2 ) : a( p1 ), b( p2 )
            {}

            ui32  a;
            ui32  b;
         };

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
            std::cout << "-- abort case: similarity is too low for the best matching template --" << std::endl;
            return;
         }

         // check the proportions are ok
         //std::set<ui32> labelToUpdate;
         Matrix ref = getFullDistances( _templates[ bestId ].distances );
         typedef std::multimap<float, Pivot> PivotContainer;

         ref.print( std::cout );
         test.print( std::cout );

         PivotContainer pivots;
         core::Buffer1D<ui32> correctClass( NB_CLASS );
         core::Buffer1D<ui32> dontcorrectClass( NB_CLASS );
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
                  float dt = fabs( test( u, n ) );
                  float dr = fabs( ref( u, n ) );

                  dtest += dt;
                  dref += dr;

                  const float err = fabs( dr - dt ) / dr;
                  std::cout << "error:" << u << "/" << n << "=" << err << std::endl;

                  if ( u == 4 )
                     // we don't want to use the skull as the main pivot!
                     pivots.insert( std::make_pair( err, Pivot( n, u ) ) );
                  else
                     pivots.insert( std::make_pair( err, Pivot( u, n ) ) );

                  if ( err > _thresholdInterDistance( u, n ) )
                  {
                     ++correctClass[ n ];
                     ++correctClass[ u ];
                     std::cout << "correct:" << u << " " << n << std::endl;
                  } else {
                     ++dontcorrectClass[ n ];
                     ++dontcorrectClass[ u ];
                     std::cout << "don't correct:" << u << " " << n << std::endl;
                  }

                 // std::cout << "1drf=" << fabs( ref( u, n ) ) << std::endl;
                 // std::cout << "1drft=" << fabs( test( u, n ) ) << std::endl;
               }
            }


            // horizontal
            for ( ui32 u = 1; u < n; ++u )
            {
               if ( ref( n, u ) < UNDEFINED_NB && test( n, u ) < UNDEFINED_NB )
               {
                  float dt = fabs( test( n, u ) );
                  float dr = fabs( ref( n, u ) );

                  dtest += dt;
                  dref += dr;

                  const float err = fabs( dr - dt ) / dr;
                  if ( u == 4 )
                     // we don't want to use the skull as the main pivot!
                     pivots.insert( std::make_pair( err, Pivot( n, u ) ) );
                  else
                  pivots.insert( std::make_pair( err, Pivot( u, n ) ) );

                  std::cout << "error:" << u << "/" << n << "=" << err << std::endl;

                  if ( err > _thresholdInterDistance( n, u ) )
                  {
                     ++correctClass[ n ];
                     ++correctClass[ u ];
                     //std::cout << "correct:" << u << " " << n << std::endl;
                  }  else {
                     ++dontcorrectClass[ n ];
                     ++dontcorrectClass[ u ];
                     //std::cout << "don't correct:" << u << " " << n << std::endl;
                  }
               }
            }

            // select the best pivot

            /*
            if ( fabs( dref ) > 0 )
            {
               float rate = fabs( dtest - dref ) / dref;
               std::cout << "label" << n << " error rate=" << rate << std::endl;
               pivots.insert( std::make_pair( rate, n ) );
            }
            */
         }

         correctClass.print( std::cout );
         dontcorrectClass.print( std::cout );

         std::cout << "----------------------------------" << std::endl;
         std::cout << "label to update=";
         for ( ui32 n = 1; n < NB_CLASS; ++n )
            if ( correctClass[ n ] > dontcorrectClass[ n ] )
               std::cout << n << " ";
         std::cout << std::endl;
         std::cout << "1pivot used=" << pivots.begin()->second.a <<  " err=" << pivots.begin()->first << std::endl;
         std::cout << "1pivot used=" << pivots.begin()->second.b <<  " err=" << pivots.begin()->first << std::endl;
         //for ( std::set<ui32>::iterator it = labelToUpdate.begin(); it != labelToUpdate.end(); ++it )
           // std::cout << *it << " " << std::endl;

         // finally correct the labels that need to!
         ensure( pivots.size() >= 1, "must have at least 2 ref points!" );
         //Pivot& pivot = pivots.begin()->second;
         if ( pivots.begin()->first > CORRECTION_MAX_PIVOT_TEMPLATE_ERROR )
         {
            std::cout << "-- abort pivot error is too big -- pivotError=" << pivots.begin()->first << std::endl;
            return;
         }

         //bool corrected = false;
         labels.print(std::cout);
         for ( ui32 n = 1; n < NB_CLASS; ++n )
         {
            if ( correctClass[ n ] > dontcorrectClass[ n ] )
            {
               // correct the label
               ui32 pivotb = pivots.begin()->second.b;
               ui32 pivota = pivots.begin()->second.a;
               if ( pivota == n )
               {
                  std::swap( pivota, pivotb );
               }

               Template& temp = _templates[ bestId ];
               const float dtest = origLabels[ pivotb ] - origLabels[ pivota ];
               const float tRatio = ( temp.distances[ n ] - temp.distances[ pivota ] ) / ( temp.distances[ pivotb ] - temp.distances[ pivota ] );
               const float newPosition = origLabels[ pivota ] + tRatio * dtest;

               // for the skull, because the template is not annotated correctly (not always at the top of the skull)
               // so it needs to be corrected! In case we find a position lower than the actual, this is very likely
               // to be wrong->we choose a template at the base of the skull
               if ( n == 4 && newPosition < origLabels[ n ] )
               {
                  // erf do nothing...
                  labels[ n ] = origLabels[ n ];
               } else {
                  labels[ n ] = newPosition;
                  //corrected = true;
               }
            } else {
               // check the position coherency
            }
               // add the missing labels
               // TODO
         }
         labels.print(std::cout);

         /*
         // doesn't seem to help...
         if ( corrected && maxIter > 0 )
         {
            // recursively recheck the resutls
            correct( labels, maxIter - 1 );
         }*/
      }

   private:
      Templates    _templates;
      Matrix       _thresholdInterDistance;
   };
}
}

#endif