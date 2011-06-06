#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
   class HaarFeatures3d
   {
   public:
      /**
       @brief encode a feature.
       @param bf the bottom left coordinate, it must be in [0..1][0..1] intervals. (i.e. for scaling)
       @param tr the top right coordinate, it must be in [0..1][0..1] intervals. (i.e. for scaling)
       */
      struct Feature
      {
         enum Direction
         {
            HALFX,
            HALFY,
            HALFZ,
            DY,
            DX,
            DZ,
            DXY,
            NONE
         };

         Feature() : _direction( NONE )
         {
         }

         static double getValue( Direction direction, const IntegralImage3d& i, const core::vector3ui& bl, const core::vector3ui& tr )
         {
            const int x1 = bl[ 0 ];
            const int x2 = tr[ 0 ];

            const int y1 = bl[ 1 ];
            const int y2 = tr[ 1 ];

            const int z1 = bl[ 2 ];
            const int z2 = tr[ 2 ];

            double sump;
            double sumd;
            int tmp1, tmp2;
            ui32 mid;
            ui32 mid1, mid2, d;
            int border;

            switch ( direction )
            {
            case HALFX:
               mid = ( x1 + x2 ) / 2;
               sump = i.getSum( core::vector3ui( x1, y1, z1 ),  core::vector3ui( mid, y2, z2 ) );
               sumd = i.getSum( core::vector3ui( mid, y1, z1 ), core::vector3ui( x2, y2, z2 ) );
               return static_cast<double>( sump - sumd );

            case HALFY:
               mid = ( y1 + y2 ) / 2;
               sump = i.getSum( core::vector3ui( x1, y1, z1 ),  core::vector3ui( x2, mid, z2 ) );
               sumd = i.getSum( core::vector3ui( x1, mid, z1 ), core::vector3ui( x2, y2, z2 ) );
               return static_cast<double>( sump - sumd );

            case HALFZ:
               mid = ( z1 + z2 ) / 2;
               sump = i.getSum( core::vector3ui( x1, y1, z1 ), core::vector3ui( x2, y1, mid ) );
               sumd = i.getSum( core::vector3ui( x1, y1, mid), core::vector3ui( x2, y2, z2 ) );
               return static_cast<double>( sump - sumd );
               /*
            case VERTICAL_TRIPLE:
               //ensure( y2 - y1 + 1 == x2 - x1 + 1, "the filter must be square" );
               //
               // note that we remove the border so that a filter of size 9x9
               // is a discrete approximation of a gaussian w = 1.2
               // compute: b border weight = 0
               //          + weight = 1
               //          - weight = 2
               // b+++b
               // b---b
               // b+++b
               //
               d = ( y2 - y1 + 1 ) / 3;
               mid1 = y1 + 1 * d;
               mid2 = y1 + 2 * d;
               border = ( ( x2 - x1 + 1 ) - ( 2 * d - 1 ) ) / 2; // actualFilterSize = 2 * d - 1, d = sizey / 3

               //ensure( mid1 > 0 && mid2 > 0, "problem in feature position" );
               tmp1 = x1 + border;
               tmp2 = x2 - border;
               sump = i.getSum( core::vector2ui( tmp1, y1 ),   core::vector2ui( tmp2, y2 ) );
               sumd = i.getSum( core::vector2ui( tmp1, mid1 ), core::vector2ui( tmp2, mid2 - 1 ) );
               return static_cast<double>( sump - 3 * sumd ); // optim: 2 area computation only + weighting
               /*
            case HORIZONTAL_TRIPLE:
               //ensure( y2 - y1 + 1 == x2 - x1 + 1, "the filter must be square" );
               //
               // note that we remove the border so that a filter of size 9x9
               // is a discrete approximation of a gaussian w = 1.2
               // compute: b border weight = 0
               //          + weight = 1
               //          - weight = 2
               // bbb
               // +-+
               // +-+
               // +-+
               // bbb
               d = ( x2 - x1 + 1 ) / 3;
               mid1 = x1 + 1 * d;
               mid2 = x1 + 2 * d;
               border = ( ( y2 - y1 + 1 ) - ( 2 * d - 1 ) ) / 2; // actualFilterSize = 2 * d - 1, d = sizey / 3

               //ensure( mid1 > 0 && mid2 > 0, "problem in feature position" );
               tmp1 = y1 + border;
               tmp2 = y2 - border;
               sump = i.getSum( core::vector2ui( x1,   tmp1 ), core::vector2ui( x2,       tmp2 ) );
               sumd = i.getSum( core::vector2ui( mid1, tmp1 ), core::vector2ui( mid2 - 1, tmp2 ) );
               return static_cast<double>( sump - 3 * sumd ); // optim: 2 area computation only + weighting

            case CHECKER:
               //ensure( y2 - y1 + 1 == x2 - x1 + 1, "the filter must be square" );
               //
               // note that we remove the border so that a filter of size 9x9
               // is a discrete approximation of a gaussian w = 1.2
               // compute: b border weight = 0
               //          + weight = 1
               //          - weight = 1
               // b b b b b
               // b + b - b
               // b - b + b
               // b b b b b
               //
               d = ( x2 - x1 + 1 ) / 3;
               mid1 = ( x2 + x1 ) / 2;
               mid2 = ( y2 + y1 ) / 2;
               border = ( ( x2 - x1 + 1 ) - ( 2 * d + 1 ) ) / 2;

               sump = i.getSum( core::vector2ui( x1 + border,   y1 + border ), core::vector2ui( mid1 - 1, mid2 - 1 ) ) +
                      i.getSum( core::vector2ui( mid1 + 1, mid2 + 1 ),         core::vector2ui( x2 - border, y2 - border ) );
               sumd = i.getSum( core::vector2ui( mid1 + 1,   y1 + border ),    core::vector2ui( x2 - border, mid2 - 1 ) ) +
                      i.getSum( core::vector2ui( x1 + border, mid2 + 1 ),      core::vector2ui( mid1 - 1, y2 - border ) );
               return static_cast<double>( sump - sumd );

               */
            case NONE:
            default:
               ensure( 0, "not handled type" );
            }
         }

      private:
         Direction   _direction;
      };
   };
}
}

class TestIntegral3D
{
public:
   void testPos()
   {
      srand(0);

      imaging::Volume<double> volume( 16, 32, 22 );
      volume.fill( 1 );
      for ( ui32 z = 0; z < volume.size()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < volume.size()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < volume.size()[ 0 ]; ++x )
            {
               volume( x, y, z ) = rand() % 100;
            }
         }
      }

      algorithm::IntegralImage3d integral;
      core::Timer t1;
      integral.construct( volume );

      for ( ui32 z = 0; z < volume.size()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < volume.size()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < volume.size()[ 0 ]; ++x )
            {
               double val = 0;
               for ( ui32 za = 0; za <= z; ++za )
               {
                  for ( ui32 ya = 0; ya <= y; ++ya )
                  {
                     for ( ui32 xa = 0; xa <= x; ++xa )
                        val += volume( xa, ya, za );
                  }
               }

               const double valfound = integral( x, y, z );
               TESTER_ASSERT( fabs( val - valfound ) < 1e-2 );

            }
         }
      }
   }

   void testSum()
   {
      srand(0);

      imaging::Volume<double> volume( 16, 32, 22 );
      
      for ( ui32 z = 0; z < volume.size()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < volume.size()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < volume.size()[ 0 ]; ++x )
            {
               volume( x, y, z ) = rand() % 100;
            }
         }
      }

      algorithm::IntegralImage3d integral;
      core::Timer t1;
      integral.construct( volume );
      for ( ui32 n = 0; n < 5000; ++n )
      {
         
         core::vector3ui bl( rand() % volume.size()[ 0 ],
                             rand() % volume.size()[ 1 ],
                             rand() % volume.size()[ 2 ] );
         core::vector3ui tr( rand() % volume.size()[ 0 ],
                             rand() % volume.size()[ 1 ],
                             rand() % volume.size()[ 2 ] );
                             
         for ( ui32 c = 0; c < 3; ++c )
         {
            if ( bl[ c ] > tr[ c ] )
               std::swap( bl[ c ], tr[ c ] );
         }

         double sum = 0;
         for ( ui32 z = bl[ 2 ]; z <= tr[ 2 ]; ++z )
         {
            for ( ui32 y = bl[ 1 ]; y <= tr[ 1 ]; ++y )
            {
               for ( ui32 x = bl[ 0 ]; x <= tr[ 0 ]; ++x )
               {
                  sum += volume( x, y, z );
               }
            }
         }

         const double sumFound = integral.getSum( bl, tr );
         TESTER_ASSERT( fabs( sumFound - sum ) < 1e-5 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIntegral3D);
TESTER_TEST(testPos);
TESTER_TEST(testSum);
TESTER_TEST_SUITE_END();
#endif
