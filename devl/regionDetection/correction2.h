#ifndef CORRECTION2_H_
# define CORRECTION2_H_

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
   class CorrectPosition2
   {
   public:
      typedef RegionResult::Measure                Measure;
      typedef std::vector<RegionResult::Measure>   Measures;
      typedef core::Matrix<float>                  Matrix;
      typedef core::Buffer1D<float>                Vector;

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

         ui32                    caseId;        // case from which it was constructed
         Vector                  distances;     // the raw distances
         Vector                  ratios;        // the interdistance ratios
      };
      typedef std::vector<Template>    Templates;

   public:
      CorrectPosition2( const Measures& m )
      {
         for ( ui32 n = 0; n < m.size(); ++n )
         {
            _templates.push_back( Template( m[ n ].id,  m[ n ].heightNeck, m[ n ].heightHeart, m[ n ].heightLung, m[ n ].heightSkull, m[ n ].heightHips ) );
            Template& t = _templates[ _templates.size() - 1 ];
            t.ratios = _getFullRatios( _getFullDistances( t.distances ) );
         }
         ensure( _templates.size(), "can't have 0 templates" );
         _constructStatistics();
         _errorCorrection = core::make_buffer1D<float>( 0, 0.2f, 1.0f, 1.0f, 1.0f, 0.2f );
      }

      void correct( Vector& distances, ui32 maxIter = 10 )
      {
         try
         {
            Vector origLabels;
            origLabels.clone( distances );

            Vector testRatios = _getFullRatios( _getFullDistances( distances ) );
            const Template& ref = getMatchingTemplate( distances );

            core::Buffer1D<int>  error( NB_CLASS );
            core::Buffer1D<int>  good( NB_CLASS );
            for ( ui32 n = 0; n < ref.ratios.size(); ++n )
            {
               if ( ( testRatios[ n ] < ( _means[ n ] - 2 * _stddev[ n ] ) ) ||
                    ( testRatios[ n ] > ( _means[ n ] + 2 * _stddev[ n ] ) ) )
               {
                  for ( ui32 nn = 0; nn < 4; ++nn )
                     if ( distances[ _links[ n ][ nn ] ] > 0 && ref.distances[ _links[ n ][ nn ] ] > 0 )
                        ++error[ _links[ n ][ nn ] ];
               } else {
                  for ( ui32 nn = 0; nn < 4; ++nn )
                     if ( distances[ _links[ n ][ nn ] ] > 0 && ref.distances[ _links[ n ][ nn ] ] > 0 )
                        ++good[ _links[ n ][ nn ] ];
               }
            }

            std::map<ui32, ui32> labelToRecompute;
            error.print( std::cout );
            good.print( std::cout );
            std::multimap<ui32, ui32> pivots;     // <nb good, label>
            for ( ui32 n = 1; n < NB_CLASS; ++n )
            {
               if ( distances[ n ] > 0 && error[ n ] * _errorCorrection[ n ] > good[ n ] )
               {
                  std::cout << "recompute label=" << n << std::endl;
                  labelToRecompute.insert( std::make_pair( error[ n ], n ) );
               }
               if ( distances[ n ] > 0 && ref.distances[ n ] > 0 )
               {
                  if ( n != 4 )
                  {
                     //std::cout << "refd=" << ref.distances[ n ] << std::endl;
                     pivots.insert( std::make_pair( good[ n ], n ) );
                  }
               }
            }

            // select 2 pivots
            if ( pivots.size() < 3 )
            {
               std::cout << "-------- cancel: not enough pivots ----------" << std::endl;
               return;  // we need at least 3 points for safety!
            }

            ui32 pivota = pivots.rbegin()->second;
            ui32 pivotb = (++pivots.rbegin())->second;
            if ( pivota == 4 )
               std::swap( pivota, pivotb );

            std::cout << "pivot=" << pivota << " " << pivotb << std::endl;

            for ( std::map<ui32, ui32>::reverse_iterator it = labelToRecompute.rbegin(); it != labelToRecompute.rend(); ++it )
            {
               const float dtest = origLabels[ pivotb ] - origLabels[ pivota ];
               const float a = ( ref.distances[ it->second ] - ref.distances[ pivota ] );
               const float b = ( ref.distances[ pivotb ] - ref.distances[ pivota ] );
               const float tRatio = a / b;
               const float newPosition = origLabels[ pivota ] + tRatio * dtest;

               // for the skull, because the template is not annotated correctly (not always at the top of the skull)
               // so it needs to be corrected! In case we find a position lower than the actual, this is very likely
               // to be wrong->we choose a template at the base of the skull
               if ( it->second == 4 && newPosition < origLabels[ it->second ] )
               {
                  // erf do nothing...
                  distances[ it->second ] = origLabels[ it->second ];
               } else {
                  std::cout << "label updated=" << it->second << std::endl;
                  distances[ it->second ] = newPosition;

                  if ( maxIter > 0 )
                  {
                     // recursively recorrect
                     correct( distances, maxIter - 1 );
                     return;
                  }
               }
            }
         }
         catch (...)
         {
            std::cout << "-*--- abort: cant find template" << std::endl;
            return;
         }
      }

      const Template& getMatchingTemplate( const Vector& distances )
      {
         Matrix ref = _getFullDistances( distances );
         Vector refRatios = _getFullRatios( ref );

         int index = -1;
         f32 minDist = (f32)INT_MAX;
         for ( ui32 n = 0; n < _templates.size(); ++n )
         {
            ui32 nbDist = 0;
            f32 tmp = _similarity( refRatios, _templates[ n ].ratios, nbDist );

            if ( tmp < minDist && nbDist > CORRECTION2_NB_MIN_DIST )
            {
               minDist = tmp;
               index = n;
            }
         }

         if ( index < 0 )
            throw std::exception("can't find template");
         std::cout << "template chosen=" << index << std::endl;
         return _templates[ index ];
      }


   //private:
      // return a matrix will all interdistances between he labels
      // type is:
      //      N H L S Hi
      //    X X X X X X
      // N  X X X X X X
      // H  X   X X X X
      // L  X     X X X
      // S  X       X X
      // Hi X         X
      // matrix( x, y ) = -1 if the distance is not valid
      static Matrix _getFullDistances( const core::Buffer1D<float>& labels )
      {
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
         return ref;
      }

      float _similarity( const Vector& v1, const Vector& v2, ui32& nbDist )
      {
         ensure( v1.size() == _means.size() &&
                 v2.size() == _means.size(), "must have the same size" );

         f32 sum = 0;
         for ( ui32 n = 0; n < v1.size(); ++n )
         {
            if ( v1[ n ] < UNDEFINED_NB && v2[ n ] < UNDEFINED_NB )
            {
               ++nbDist;
               float val = v1[ n ] - v2[ n ];
               sum += val * val;
            }
         }
         if ( !nbDist )
            return (f32)INT_MAX;
         return sqrt( sum );
      }

      Vector _getFullRatios( const Matrix& interdistances )
      {
         const ui32 actualNbClass = NB_CLASS - 1;
         const ui32 nbPossibleDist = ( actualNbClass * ( actualNbClass - 1 ) ) / 2;
         Vector ref( nbPossibleDist * ( nbPossibleDist ) );
         for ( ui32 n = 0; n < ref.size(); ++n )
            ref[ n ] = UNDEFINED_NB;

         bool computeLinks = false;
         if ( _links.size() == 0 )
         {
            _links = std::vector< std::vector<ui32 > >( ref.size() );
            computeLinks = true;
         }

         ui32 nn = 0;
         for ( ui32 n = 2; n < NB_CLASS; ++n )  // start a 1, we don't care about the 0 label
         {
            for ( ui32 m = 1; m < n; ++m )
            {
               for ( ui32 o = 2; o < NB_CLASS; ++o )  // start a 1, we don't care about the 0 label
               {
                  for ( ui32 p = 1; p < o; ++p )
                  {
                     if ( o != n || p != m )
                     {
                        if ( interdistances( n, m ) < UNDEFINED_NB &&
                             interdistances( o, p ) < UNDEFINED_NB )
                        {
                           const float v1 = interdistances( n, m );
                           const float v2 = interdistances( o, p );
                           if ( fabs( v2 ) < 1e-3 )
                              ref[ nn ] = UNDEFINED_NB;
                           else
                              ref[ nn ] = v1 / v2;
                        } else {
                           ref[ nn ] = UNDEFINED_NB;
                        }
                     }
                     if ( computeLinks )
                     {
                        _links[ nn ] = core::make_vector<ui32>( n, m, o, p );
                     }
                     ++nn;
                  }
               }
            }
         }
         //ref.print( std::cout );
         return ref;
      }

      void _constructStatistics()
      {
         const ui32 actualNbClass = NB_CLASS - 1;
         const ui32 nbPossibleDist = ( actualNbClass * ( actualNbClass - 1 ) ) / 2;
         const ui32 nbInterdist = nbPossibleDist * nbPossibleDist;

         std::vector<float> v;
         _means   = Vector( nbInterdist );
         _stddev  = Vector( nbInterdist );
         for ( ui32 var = 0; var < nbInterdist; ++var )
         {
            v.clear();
            float sum = 0;
            for ( ui32 n = 0; n < _templates.size(); ++n )
            {
               if ( _templates[ n ].ratios[ var ] < UNDEFINED_NB )
               {
                  v.push_back( _templates[ n ].ratios[ var ] );
                  sum += _templates[ n ].ratios[ var ];
               }
            }
            if ( v.size() == 0 )
            {
               _means[ var ] = UNDEFINED_NB;
               _stddev[ var ] = UNDEFINED_NB;
               continue;
            }

            _means[ var ] = sum / v.size();

            
            sum = 0;
            for ( ui32 n = 0; n < v.size(); ++n )
            {
               sum += core::sqr( v[ n ] - _means[ var ] );
            }
            _stddev[ var ] = sqrt( sum / v.size() );
         }

         _means.print( std::cout );
         _stddev.print( std::cout );
      }

   private:
      Templates    _templates;
      Vector       _means;
      Vector       _stddev;
      Vector       _errorCorrection;
      std::vector< std::vector<ui32> >   _links;   // ratio interdistance link: store the labels involved for computation
   };
}
}

#endif