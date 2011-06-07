/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_ALGORITHM_HAAR_FEATURES_3D_H_
# define NLL_ALGORITHM_HAAR_FEATURES_3D_H_

# pragma warning( push )
# pragma warning( disable:4127 ) // conditional expression is constant

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Helper to compute 3D Haar features in the most common type
    */ 
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
            DXZ,
            DYZ,
            DXYZ,
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
            int tmp1, tmp2, tmp3, tmp4;
            ui32 mid;
            ui32 mid1, mid2, mid3, d;
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
               
            case DY:
               assert( y2 - y1 + 1 == x2 - x1 + 1 &&
                       y2 - y1 + 1 == z2 - z1 + 1 ); //"the filter must be square"
               //
               // note that we remove the border so that a filter of size 9x9x9
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

               tmp1 = x1 + border;
               tmp2 = x2 - border;
               tmp3 = z1 + border;
               tmp4 = z2 - border;
               sump = i.getSum( core::vector3ui( tmp1, y1, tmp3 ),   core::vector3ui( tmp2, y2, tmp4 ) );
               sumd = i.getSum( core::vector3ui( tmp1, mid1, tmp3 ), core::vector3ui( tmp2, mid2 - 1, tmp4 ) );
               return static_cast<double>( sump - 3 * sumd ); // optim: 2 area computation only + weighting
               
            case DX:
               assert( y2 - y1 + 1 == x2 - x1 + 1 &&
                       y2 - y1 + 1 == z2 - z1 + 1 ); //"the filter must be square"
               //
               // note that we remove the border so that a filter of size 9x9x9
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
               tmp3 = z1 + border;
               tmp4 = z2 - border;
               sump = i.getSum( core::vector3ui( x1,   tmp1, tmp3 ), core::vector3ui( x2,       tmp2, tmp4 ) );
               sumd = i.getSum( core::vector3ui( mid1, tmp1, tmp3 ), core::vector3ui( mid2 - 1, tmp2, tmp4 ) );
               return static_cast<double>( sump - 3 * sumd ); // optim: 2 area computation only + weighting

            case DXY:
               assert( y2 - y1 + 1 == x2 - x1 + 1 &&
                       y2 - y1 + 1 == z2 - z1 + 1 ); //"the filter must be square"
               //
               // note that we remove the border so that a filter of size 9x9x9
               // is a discrete approximation of a gaussian w = 1.2
               // compute: b border weight = 0
               //          + weight = 1
               //          - weight = 1
               // b b b b b
               // b + b - b     in XY plane
               // b - b + b
               // b b b b b
               //
               d = ( x2 - x1 + 1 ) / 3;
               mid1 = ( x2 + x1 ) / 2;
               mid2 = ( y2 + y1 ) / 2;
               border = ( ( x2 - x1 + 1 ) - ( 2 * d + 1 ) ) / 2;

               sump = i.getSum( core::vector3ui( x1 + border,   y1 + border, z1 + border ), core::vector3ui( mid1 - 1,    mid2 - 1,    z2 - border ) ) +
                      i.getSum( core::vector3ui( mid1 + 1, mid2 + 1,         z1 + border ), core::vector3ui( x2 - border, y2 - border, z2 - border ) );
               sumd = i.getSum( core::vector3ui( mid1 + 1,   y1 + border,    z1 + border ), core::vector3ui( x2 - border, mid2 - 1,    z2 - border ) ) +
                      i.getSum( core::vector3ui( x1 + border, mid2 + 1,      z1 + border ), core::vector3ui( mid1 - 1,    y2 - border, z2 - border ) );
               return static_cast<double>( sump - sumd );

            case DXZ:
               assert( y2 - y1 + 1 == x2 - x1 + 1 &&
                       y2 - y1 + 1 == z2 - z1 + 1 ); //"the filter must be square"
               //
               // note that we remove the border so that a filter of size 9x9x9
               // is a discrete approximation of a gaussian w = 1.2
               // compute: b border weight = 0
               //          + weight = 1
               //          - weight = 1
               // b b b b b
               // b + b - b  in XZ plane
               // b - b + b
               // b b b b b
               //
               d = ( x2 - x1 + 1 ) / 3;
               mid1 = ( x2 + x1 ) / 2;
               mid2 = ( z2 + z1 ) / 2;
               border = ( ( x2 - x1 + 1 ) - ( 2 * d + 1 ) ) / 2;

               sump = i.getSum( core::vector3ui( x1 + border,   y1 + border, z1 + border ), core::vector3ui( mid1 - 1,    y2 - border, mid2 - 1 ) ) +
                      i.getSum( core::vector3ui( mid1 + 1,      y1 + border, mid2 + 1 ),    core::vector3ui( x2 - border, y2 - border, z2 - border ) );
               sumd = i.getSum( core::vector3ui( mid1 + 1,      y1 + border, z1 + border ), core::vector3ui( x2 - border, y2 - border, mid2 - 1 ) ) +
                      i.getSum( core::vector3ui( x1 + border,   y1 + border, mid2 + 1 ),    core::vector3ui( mid1 - 1,    y2 - border, z2 - border ) );
               return static_cast<double>( sump - sumd );

            case DYZ:
               assert( y2 - y1 + 1 == x2 - x1 + 1 &&
                       y2 - y1 + 1 == z2 - z1 + 1 ); //"the filter must be square"
               //
               // note that we remove the border so that a filter of size 9x9x9
               // is a discrete approximation of a gaussian w = 1.2
               // compute: b border weight = 0
               //          + weight = 1
               //          - weight = 1
               // b b b b b
               // b + b - b in XY plane
               // b - b + b
               // b b b b b
               //
               d = ( y2 - y1 + 1 ) / 3;
               mid2 = ( y2 + y1 ) / 2;
               mid1 = ( z2 + z1 ) / 2;
               border = ( ( y2 - y1 + 1 ) - ( 2 * d + 1 ) ) / 2;

               sump = i.getSum( core::vector3ui( x1 + border, y1 + border, z1 + border ), core::vector3ui( x2 - border, mid2 - 1,    mid1 - 1 ) ) +
                      i.getSum( core::vector3ui( x1 + border, mid2 + 1,    mid1 + 1  ),   core::vector3ui( x2 - border, y2 - border, z2 - border ) );
               sumd = i.getSum( core::vector3ui( x1 + border, y1 + border, mid1 + 1 ),    core::vector3ui( x2 - border, mid2 - 1,    z2 - border ) ) +
                      i.getSum( core::vector3ui( x1 + border, mid2 + 1,    z1 + border ), core::vector3ui( x2 - border, y2 - border, mid1 - 1 ) );
               return static_cast<double>( sump - sumd );

            case DXYZ:
               assert( y2 - y1 + 1 == x2 - x1 + 1 &&
                       y2 - y1 + 1 == z2 - z1 + 1 ); //"the filter must be square"
               //
               // note that we remove the border so that a filter of size 9x9x9
               // is a discrete approximation of a gaussian w = 1.2
               // compute: b border weight = 0
               //          + weight = 1
               //          - weight = 1
               // b b b b b
               // b + b - b in XY plane until half z, after the positif and negative are swapped
               // b - b + b
               // b b b b b
               //
               d = ( x2 - x1 + 1 ) / 3;
               mid1 = ( x2 + x1 ) / 2;
               mid2 = ( y2 + y1 ) / 2;
               mid3 = ( z2 + z1 ) / 2;
               border = ( ( x2 - x1 + 1 ) - ( 2 * d + 1 ) ) / 2;

               sump = i.getSum( core::vector3ui( x1 + border,   y1 + border, z1 + border ), core::vector3ui( mid1 - 1,    mid2 - 1,    mid3 - 1 ) ) +
                      i.getSum( core::vector3ui( mid1 + 1, mid2 + 1,         z1 + border ), core::vector3ui( x2 - border, y2 - border, mid3 - 1 ) ) +
                      i.getSum( core::vector3ui( mid1 + 1,   y1 + border,    mid3 + 1 ), core::vector3ui( x2 - border, mid2 - 1,    z2 - border ) ) +
                      i.getSum( core::vector3ui( x1 + border, mid2 + 1,      mid3 + 1 ), core::vector3ui( mid1 - 1,    y2 - border, z2 - border ) );

               sumd = i.getSum( core::vector3ui( mid1 + 1,   y1 + border,    z1 + border ), core::vector3ui( x2 - border, mid2 - 1,    mid3 - 1 ) ) +
                      i.getSum( core::vector3ui( x1 + border, mid2 + 1,      z1 + border ), core::vector3ui( mid1 - 1,    y2 - border, mid3 - 1 ) ) +
                      i.getSum( core::vector3ui( x1 + border,   y1 + border, mid3 + 1 ), core::vector3ui( mid1 - 1,    mid2 - 1,    z2 - border ) ) +
                      i.getSum( core::vector3ui( mid1 + 1, mid2 + 1,         mid3 + 1 ), core::vector3ui( x2 - border, y2 - border, z2 - border ) );
               return static_cast<double>( sump - sumd );

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

#endif