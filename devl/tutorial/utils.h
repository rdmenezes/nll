#ifndef NLL_TUTORIAL_UTILS_H_
# define NLL_TUTORIAL_UTILS_H_

#include <nll/nll.h>

namespace nll
{
namespace utility
{
   /**
    @ingroup utility
    @brief Print the projection of a high dimentional space to a 2D using Sammon's projection
    */
   template <class Database>
   core::Image<ui8> printProjection( ui32 sizex, ui32 sizey, const Database& dat )
   {
      sizex /= 2;
      sizey /= 2;

      ensure( dat.size() && sizex && sizey, "invalid parameters" );

      static ui8 colors[][ 3 ] =
      {
         {255, 0, 0},
         {0, 255, 0},
         {0, 0, 255},
         {0, 255, 255},
         {255, 0, 255},
         {255, 255, 0},
         {64, 0, 0},
         {0, 64, 0},
         {0, 0, 64},
         {0, 64, 64},
         {64, 0, 64},
         {64, 64, 0},
         {255, 255, 255}
      };

      typedef typename Database::Sample::Input  Input;
      typedef nll::core::DatabaseInputAdapterRead<Database> Adapter;
      typedef nll::algorithm::SammonProjection Projection;

      Adapter points( dat );
      Projection p;
      std::vector<Input> ppoints = p.project( points );

      // determine min and max to scale the data
      core::vector2d min( std::numeric_limits<int>::max(), std::numeric_limits<int>::max() );
      core::vector2d max( std::numeric_limits<int>::min(), std::numeric_limits<int>::min() );

      const ui32 size = static_cast<ui32>( dat.size() );
      for ( ui32 n = 0; n < size; ++n )
      {
         min[ 0 ] = std::min<double>( ppoints[ n ][ 0 ], min[ 0 ] );
         min[ 1 ] = std::min<double>( ppoints[ n ][ 1 ], min[ 1 ] );

         max[ 0 ] = std::max<double>( ppoints[ n ][ 0 ], max[ 0 ] );
         max[ 1 ] = std::max<double>( ppoints[ n ][ 1 ], max[ 1 ] );
      }

      ensure( min[ 0 ] < max[ 0 ] && min[ 1 ] < max[ 1 ], "invalid points" );
      const double ratiox = ( max[ 0 ] - min[ 0 ] + 1 ) / ( sizex );
      const double ratioy = ( max[ 1 ] - min[ 1 ] + 1 ) / ( sizey );
      
      core::Image<ui8> im( sizex, sizey, 3 );
      for ( ui32 n = 0; n < size; ++n )
      {
         ui32 x = static_cast<ui32>( ( ppoints[ n ][ 0 ] - min[ 0 ] ) / ratiox );
         ui32 y = static_cast<ui32>( ( ppoints[ n ][ 1 ] - min[ 1 ] ) / ratioy );

         ensure( dat[ n ].output < 13, "too many classes, not enough colors defined" );
         ensure( x < sizex && y < sizey, "out of bound!" );

         ui8* c = im.point( x, y );
         c[ 0 ] = colors[ dat[ n ].output ][ 0 ];
         c[ 1 ] = colors[ dat[ n ].output ][ 1 ];
         c[ 2 ] = colors[ dat[ n ].output ][ 2 ];
      }

      core::rescaleNearestNeighbor( im, sizex * 2, sizey * 2 );
      return im;
   }

   template <class Database>
   core::Image<ui8> printProjection( ui32 sizex, ui32 sizey, const Database& dat, const algorithm::Classifier<typename Database::Sample::Input>& classifier )
   {
      sizex /= 2;
      sizey /= 2;

      ensure( dat.size() && sizex && sizey, "invalid parameters" );

      static ui8 colors[][ 3 ] =
      {
         {255, 0, 0},
         {0, 255, 0},
         {0, 0, 255},
         {0, 255, 255},
         {255, 0, 255},
         {255, 255, 0},
         {64, 0, 0},
         {0, 64, 0},
         {0, 0, 64},
         {0, 64, 64},
         {64, 0, 64},
         {64, 64, 0},
         {255, 255, 255}
      };

      typedef typename Database::Sample::Input  Input;
      typedef nll::core::DatabaseInputAdapterRead<Database> Adapter;
      typedef nll::algorithm::SammonProjection Projection;

      Adapter points( dat );
      Projection p;
      std::vector<Input> ppoints = p.project( points );

      // determine min and max to scale the data
      core::vector2d min( std::numeric_limits<int>::max(), std::numeric_limits<int>::max() );
      core::vector2d max( std::numeric_limits<int>::min(), std::numeric_limits<int>::min() );

      const ui32 size = static_cast<ui32>( dat.size() );
      for ( ui32 n = 0; n < size; ++n )
      {
         min[ 0 ] = std::min<double>( ppoints[ n ][ 0 ], min[ 0 ] );
         min[ 1 ] = std::min<double>( ppoints[ n ][ 1 ], min[ 1 ] );

         max[ 0 ] = std::max<double>( ppoints[ n ][ 0 ], max[ 0 ] );
         max[ 1 ] = std::max<double>( ppoints[ n ][ 1 ], max[ 1 ] );
      }

      ensure( min[ 0 ] < max[ 0 ] && min[ 1 ] < max[ 1 ], "invalid points" );
      const double ratiox = ( max[ 0 ] - min[ 0 ] + 1 ) / ( sizex );
      const double ratioy = ( max[ 1 ] - min[ 1 ] + 1 ) / ( sizey );

      core::Image<ui8> im( sizex, sizey, 3 );
      for ( ui32 n = 0; n < size; ++n )
      {
         ui32 x = static_cast<ui32>( ( ppoints[ n ][ 0 ] - min[ 0 ] ) / ratiox );
         ui32 y = static_cast<ui32>( ( ppoints[ n ][ 1 ] - min[ 1 ] ) / ratioy );

         ui32 cc = classifier.test( dat[ n ].input );

         ensure( dat[ n ].output < 13, "too many classes, not enough colors defined" );
         ensure( x < sizex && y < sizey, "out of bound!" );

         if ( cc == dat[ n ].output )
         {
            ui8* c = im.point( x, y );
            c[ 0 ] = colors[ dat[ n ].output ][ 0 ];
            c[ 1 ] = colors[ dat[ n ].output ][ 1 ];
            c[ 2 ] = colors[ dat[ n ].output ][ 2 ];
         } else {
            ui8* c = im.point( x, y );
            c[ 0 ] = colors[ dat[ n ].output ][ 0 ] / 2;
            c[ 1 ] = colors[ dat[ n ].output ][ 1 ] / 2;
            c[ 2 ] = colors[ dat[ n ].output ][ 2 ] / 2;
         }
      }

      core::rescaleNearestNeighbor( im, sizex * 2, sizey * 2 );
      return im;
   }
}
}
#endif