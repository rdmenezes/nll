#ifndef NLL_BOUNDS_H_
# define NLL_BOUNDS_H_

# include <nll/nll.h>

using namespace nll;

template <class Classifier, class Database>
core::Image<ui8> findBounds( const Classifier& classifier, const core::vector2d& min , const core::vector2d& max, const Database& dat, ui32 size )
{
   ui8 colors[][3] =
   {
      {255, 0, 0},
      {0, 255, 0},
      {0, 0, 255},
      {255, 255, 0},
      {255, 255, 255},
      {255, 0, 255},
      {255, 255, 0},
      {0, 255, 255},
   };

   //core::Image<ui8> i( max[ 0 ] - min[ 0 ], max[ 1 ] - min[ 1 ], 3 );
   core::Image<ui8> i( size, size, 3 );
   double px = min[ 0 ];
   double py = min[ 1 ];
   const double stepx = ( max[ 0 ] - min[ 0 ] ) / size;
   const double stepy = ( max[ 1 ] - min[ 1 ] ) / size;

   for ( i32 y = 0; y < (i32)size; ++y, py += stepy )
   {
      px = min[ 0 ];
      for ( i32 x = 0; x < (i32)size; ++x, px += stepx )
      {
         core::Buffer1D<double> buf( 2 );
         buf[ 0 ] = px;
         buf[ 1 ] = py;
         ui32 c = classifier.test( buf );

         ui8* p = i.point( x, y );
         assert( c < 8 );
         p[ 0 ] = (ui8)( colors[ c ][ 0 ] * 0.75 );
         p[ 1 ] = (ui8)( colors[ c ][ 1 ] * 0.75 );
         p[ 2 ] = (ui8)( colors[ c ][ 2 ] * 0.75 );
      }
   }

   for ( ui32 n = 0; n < dat.size(); ++n )
   {
      if ( dat[ n ].input[ 0 ] >= min[ 0 ] && dat[ n ].input[ 0 ] < max[ 0 ] &&
           dat[ n ].input[ 1 ] >= min[ 1 ] && dat[ n ].input[ 1 ] < max[ 1 ] )
      {
         ui32 c = dat[ n ].output;
         ui8* p = i.point( ( dat[ n ].input[ 0 ] - min[ 0 ] ) / stepx, ( dat[ n ].input[ 1 ] - min[ 1 ] ) / stepy );
         p[ 0 ] = colors[ c ][ 0 ];
         p[ 1 ] = colors[ c ][ 1 ];
         p[ 2 ] = colors[ c ][ 2 ];
      }
   }
   return i;
}

#endif