/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
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
      enum Direction
      {
         DX,
         DY,
         DZ,
         D2X,
         D2Y,
         D2Z,
         D2XY,
         D2XZ,
         D2YZ,
         D3XYZ
      };

      static double getValue( const Direction direction, const IntegralImage3d& i, const core::vector3i& position, const int lobeSize )
      {
         // To simplify the following proofs, let's have a gaussian function defined
         // as g(x, y) = exp( - ( x^2 + y^2 + z^2 ) )


         if ( direction == D2X )
         {
            // d g(x, y, z) / dx^2 = 4*x^2*e^(-x^2 - y^2 - z^2) - 2*e^(-x^2 - y^2 - z^2)
            // we approximate this as below

            // computes: (XY) view
            // X 0 0 0 0 0 0 0 0     X = (0, 0)
            // 0 0 0 0 0 0 0 0 0
            // p p p n n n p p p 
            // p p p n n n p p p
            // p p p n n n p p p
            // p p p n n n p p p
            // p p p n n n p p p
            // 0 0 0 0 0 0 0 0 0
            // 0 0 0 0 0 0 0 0 0
            //       <=+=> lobeSize

            #ifdef NLL_SECURE
            ensure( ( lobeSize % 2 ) == 1, "must be a odd" );
            #endif

            const int halfLobe = lobeSize / 2;
            const int halfx = lobeSize + halfLobe;
            const int halfy = ( 2 * lobeSize - 1 ) / 2;

            const core::vector3i min( position[ 0 ] - halfx, position[ 1 ] - halfy, position[ 2 ] - halfy);
            const core::vector3i max( position[ 0 ] + halfx, position[ 1 ] + halfy, position[ 2 ] + halfy );

            const core::vector3i subMin( position[ 0 ] - halfLobe, position[ 1 ] - halfy, position[ 2 ] - halfy );
            const core::vector3i subMax( position[ 0 ] + halfLobe, position[ 1 ] + halfy, position[ 2 ] + halfy );

            const double sump = i.getSum( min, max );
            const double sumd = i.getSum( subMin, subMax );
            return static_cast<double>( sump - 3 * sumd ); // optim: 2 area computation only + weighting
         }


         if ( direction == D2Y )
         {
            // d g(x, y, z) / dy^2 = 4*y^2*e^(-x^2 - y^2 - z^2) - 2*e^(-x^2 - y^2 - z^2)
            // we approximate this as below

            // computes: (XY plane)
            // X 0 p p p p p 0 0     X = (0, 0)
            // 0 0 p p p p p 0 0
            // 0 0 p p p p p 0 0
            // 0 0 n n n n n 0 0  -
            // 0 0 n n n n n 0 0  |  lobeSize
            // 0 0 n n n n n 0 0  -
            // 0 0 p p p p p 0 0
            // 0 0 p p p p p 0 0
            // 0 0 p p p p p 0 0

            #ifdef NLL_SECURE
            ensure( ( lobeSize % 2 ) == 1, "must be a odd" );
            #endif

            const int halfLobe = lobeSize / 2;
            const int halfy = lobeSize + halfLobe;
            const int halfx = ( 2 * lobeSize - 1 ) / 2;

            const core::vector3i min( position[ 0 ] - halfx, position[ 1 ] - halfy, position[ 2 ] - halfx );
            const core::vector3i max( position[ 0 ] + halfx, position[ 1 ] + halfy, position[ 2 ] + halfx );

            const core::vector3i subMin( position[ 0 ] - halfx, position[ 1 ] - halfLobe, position[ 2 ] - halfx );
            const core::vector3i subMax( position[ 0 ] + halfx, position[ 1 ] + halfLobe, position[ 2 ] + halfx );

            const double sump = i.getSum( min, max );
            const double sumd = i.getSum( subMin, subMax );
            return static_cast<double>( sump - 3 * sumd ); // optim: 2 area computation only + weighting
         }

         if ( direction == D2Z )
         {
            // d g(x, y, z) / dz^2 = 4*z^2*e^(-x^2 - y^2 - z^2) - 2*e^(-x^2 - y^2 - z^2)
            // we approximate this as below

            // computes: (YZ plane) or (XZ)
            // X 0 p p p p p 0 0     X = (0, 0)
            // 0 0 p p p p p 0 0
            // 0 0 p p p p p 0 0
            // 0 0 n n n n n 0 0  -
            // 0 0 n n n n n 0 0  |  lobeSize
            // 0 0 n n n n n 0 0  -
            // 0 0 p p p p p 0 0
            // 0 0 p p p p p 0 0
            // 0 0 p p p p p 0 0

            #ifdef NLL_SECURE
            ensure( ( lobeSize % 2 ) == 1, "must be a odd" );
            #endif

            const int halfLobe = lobeSize / 2;
            const int halfy = lobeSize + halfLobe;
            const int halfx = ( 2 * lobeSize - 1 ) / 2;

            const core::vector3i min( position[ 0 ] - halfx, position[ 1 ] - halfx, position[ 2 ] - halfy );
            const core::vector3i max( position[ 0 ] + halfx, position[ 1 ] + halfx, position[ 2 ] + halfy );

            const core::vector3i subMin( position[ 0 ] - halfx, position[ 1 ] - halfx, position[ 2 ] - halfLobe );
            const core::vector3i subMax( position[ 0 ] + halfx, position[ 1 ] + halfx, position[ 2 ] + halfLobe );

            const double sump = i.getSum( min, max );
            const double sumd = i.getSum( subMin, subMax );
            return static_cast<double>( sump - 3 * sumd ); // optim: 2 area computation only + weighting
         }

         if ( direction == DX )
         {
            // d g(x, y) / dx = -2*x*e^(-x^2 - y^2 - z^2)
            // we approximate this as below

            // copnutes: (XY plane)
            // X p p p 0 n n n n     X = (0, 0)
            // p p p p 0 n n n n
            // p p p p 0 n n n n
            // p p p p 0 n n n n
            // p p p p 0 n n n n
            // p p p p 0 n n n n
            // p p p p 0 n n n n
            // p p p p 0 n n n n
            // p p p p 0 n n n n
            // <===============> lobeSize

            #ifdef NLL_SECURE
            ensure( lobeSize % 2 == 1, "must be a odd" );
            #endif

            const int half = lobeSize / 2;

            const core::vector3i min( position[ 0 ] - half, position[ 1 ] - half, position[ 2 ] - half );
            const core::vector3i max( position[ 0 ] - 1,    position[ 1 ] + half, position[ 2 ] + half );

            const core::vector3i subMin( position[ 0 ] + 1,    position[ 1 ] - half, position[ 2 ] - half );
            const core::vector3i subMax( position[ 0 ] + half, position[ 1 ] + half, position[ 2 ] + half );

            const double sump = i.getSum( min, max );
            const double sumd = i.getSum( subMin, subMax );
            return static_cast<double>( sump - sumd );
         }

         if ( direction == DY )
         {
            // d g(x, y) / dy = -2*y*e^(-x^2 - y^2 - z^2)
            // we approximate this as below

            // computes: (XY plane)
            // X p p p p p p p p     X = (0, 0)
            // p p p p p p p p p 
            // p p p p p p p p p
            // p p p p p p p p p
            // 0 0 0 0 0 0 0 0 0
            // n n n n n n n n n
            // n n n n n n n n n
            // n n n n n n n n n
            // n n n n n n n n n
            // <===============> lobeSize

            #ifdef NLL_SECURE
            ensure( lobeSize % 2 == 1, "must be a odd" );
            #endif

            const int half = lobeSize / 2;

            const core::vector3i min( position[ 0 ] - half, position[ 1 ] - half, position[ 2 ] - half );
            const core::vector3i max( position[ 0 ] + half, position[ 1 ] - 1,    position[ 2 ] + half );

            const core::vector3i subMin( position[ 0 ] - half, position[ 1 ] + 1,    position[ 2 ] - half );
            const core::vector3i subMax( position[ 0 ] + half, position[ 1 ] + half, position[ 2 ] + half );

            const double sump = i.getSum( min, max );
            const double sumd = i.getSum( subMin, subMax );
            return static_cast<double>( sump - sumd );
         }

         if ( direction == DZ )
         {
            // d g(x, y) / dz = -2*z*e^(-x^2 - y^2 - z^2)
            // we approximate this as below

            // computes: (XZ plane) or (YZ plane)
            // X p p p p p p p p     X = (0, 0)
            // p p p p p p p p p 
            // p p p p p p p p p
            // p p p p p p p p p
            // 0 0 0 0 0 0 0 0 0
            // n n n n n n n n n
            // n n n n n n n n n
            // n n n n n n n n n
            // n n n n n n n n n
            // <===============> lobeSize

            #ifdef NLL_SECURE
            ensure( lobeSize % 2 == 1, "must be a odd" );
            #endif

            const int half = lobeSize / 2;

            const core::vector3i min( position[ 0 ] - half, position[ 1 ] - half, position[ 2 ] - half );
            const core::vector3i max( position[ 0 ] + half, position[ 1 ] + half, position[ 2 ] - 1 );

            const core::vector3i subMin( position[ 0 ] - half, position[ 1 ] - half, position[ 2 ] + 1 );
            const core::vector3i subMax( position[ 0 ] + half, position[ 1 ] + half, position[ 2 ] + half );

            const double sump = i.getSum( min, max );
            const double sumd = i.getSum( subMin, subMax );
            return static_cast<double>( sump - sumd );
         }

         if ( direction == D2XY )
         {
            // d g(x, y, z) / dxy = 4*x*y*e^(-x^2 - y^2 - z^2)
            // we approximate this as below

            // computes:
            // X 0 0 0 0 0 0 0 0    X = (0, 0)
            // 0 p p p 0 n n n 0
            // 0 p 1 p 0 n 2 n 0
            // 0 p p p 0 n n n 0
            // 0 0 0 0 0 0 0 0 0
            // 0 n n n 0 p p p 0
            // 0 n 3 n 0 p 4 p 0
            // 0 n n n 0 p p p 0
            // 0 0 0 0 0 0 0 0 0
            //           <=+=> lobeSize

            #ifdef NLL_SECURE
            ensure( ( lobeSize % 2 ) == 1, "must be a odd" );
            #endif

            const core::vector3i min1( position[ 0 ] - lobeSize, position[ 1 ] - lobeSize, position[ 2 ] - lobeSize );
            const core::vector3i max1( position[ 0 ] - 1,        position[ 1 ] - 1,        position[ 2 ] + lobeSize );

            const core::vector3i min2( position[ 0 ] + 1,        position[ 1 ] - lobeSize, position[ 2 ] - lobeSize );
            const core::vector3i max2( position[ 0 ] + lobeSize, position[ 1 ] - 1,        position[ 2 ] + lobeSize );

            const core::vector3i min3( position[ 0 ] - lobeSize, position[ 1 ] + 1,        position[ 2 ] - lobeSize );
            const core::vector3i max3( position[ 0 ] - 1,        position[ 1 ] + lobeSize, position[ 2 ] + lobeSize );

            const core::vector3i min4( position[ 0 ] + 1,        position[ 1 ] + 1,        position[ 2 ] - lobeSize );
            const core::vector3i max4( position[ 0 ] + lobeSize, position[ 1 ] + lobeSize, position[ 2 ] + lobeSize );

            const double sum1 = i.getSum( min1, max1 );
            const double sum2 = i.getSum( min2, max2 );
            const double sum3 = i.getSum( min3, max3 );
            const double sum4 = i.getSum( min4, max4 );
            return static_cast<double>( sum1 + sum4 - sum2 - sum3 ); // optim: 2 area computation only + weighting
         }

         if ( direction == D2XZ )
         {
            // d g(x, y, z) / dxz = 4*x*z*e^(-x^2 - y^2 - z^2)
            // we approximate this as below

            // computes:
            // X 0 0 0 0 0 0 0 0    X = (0, 0)
            // 0 p p p 0 n n n 0
            // 0 p 1 p 0 n 2 n 0
            // 0 p p p 0 n n n 0
            // 0 0 0 0 0 0 0 0 0
            // 0 n n n 0 p p p 0
            // 0 n 3 n 0 p 4 p 0
            // 0 n n n 0 p p p 0
            // 0 0 0 0 0 0 0 0 0
            //           <=+=> lobeSize

            #ifdef NLL_SECURE
            ensure( ( lobeSize % 2 ) == 1, "must be a odd" );
            #endif

            const core::vector3i min1( position[ 0 ] - lobeSize, position[ 1 ] - lobeSize, position[ 2 ] - lobeSize );
            const core::vector3i max1( position[ 0 ] - 1,        position[ 1 ] + lobeSize, position[ 2 ] - 1 );

            const core::vector3i min2( position[ 0 ] + 1,        position[ 1 ] - lobeSize, position[ 2 ] - lobeSize );
            const core::vector3i max2( position[ 0 ] + lobeSize, position[ 1 ] + lobeSize, position[ 2 ] - 1 );

            const core::vector3i min3( position[ 0 ] - lobeSize, position[ 1 ] - lobeSize, position[ 2 ] + 1 );
            const core::vector3i max3( position[ 0 ] - 1,        position[ 1 ] + lobeSize, position[ 2 ] + lobeSize );

            const core::vector3i min4( position[ 0 ] + 1,        position[ 1 ] - lobeSize, position[ 2 ] + 1 );
            const core::vector3i max4( position[ 0 ] + lobeSize, position[ 1 ] + lobeSize, position[ 2 ] + lobeSize );

            const double sum1 = i.getSum( min1, max1 );
            const double sum2 = i.getSum( min2, max2 );
            const double sum3 = i.getSum( min3, max3 );
            const double sum4 = i.getSum( min4, max4 );
            return static_cast<double>( sum1 + sum4 - sum2 - sum3 ); // optim: 2 area computation only + weighting
         }
        
         if ( direction == D2YZ )
         {
            // d g(x, y, z) / dyz = 4*y*z*e^(-x^2 - y^2 - z^2)
            // we approximate this as below

            // computes:
            //      Y axis
            // X 0 0 0 0 0 0 0 0    X = (0, 0)
            // 0 p p p 0 n n n 0
            // 0 p 1 p 0 n 2 n 0
            // 0 p p p 0 n n n 0
            // 0 0 0 0 0 0 0 0 0       Z axis
            // 0 n n n 0 p p p 0
            // 0 n 3 n 0 p 4 p 0
            // 0 n n n 0 p p p 0
            // 0 0 0 0 0 0 0 0 0
            //           <=+=> lobeSize

            #ifdef NLL_SECURE
            ensure( ( lobeSize % 2 ) == 1, "must be a odd" );
            #endif

            const core::vector3i min1( position[ 0 ] - lobeSize, position[ 1 ] - lobeSize,   position[ 2 ] - lobeSize );
            const core::vector3i max1( position[ 0 ] + lobeSize, position[ 1 ] - 1,          position[ 2 ] - 1 );

            const core::vector3i min2( position[ 0 ] - lobeSize, position[ 1 ] + 1,          position[ 2 ] - lobeSize );
            const core::vector3i max2( position[ 0 ] + lobeSize, position[ 1 ] + lobeSize,   position[ 2 ] - 1 );

            const core::vector3i min3( position[ 0 ] - lobeSize, position[ 1 ] - lobeSize,   position[ 2 ] + 1 );
            const core::vector3i max3( position[ 0 ] + lobeSize, position[ 1 ] - 1,          position[ 2 ] + lobeSize );

            const core::vector3i min4( position[ 0 ] - lobeSize, position[ 1 ] + 1,          position[ 2 ] + 1 );
            const core::vector3i max4( position[ 0 ] + lobeSize, position[ 1 ] + lobeSize,   position[ 2 ] + lobeSize );

            const double sum1 = i.getSum( min1, max1 );
            const double sum2 = i.getSum( min2, max2 );
            const double sum3 = i.getSum( min3, max3 );
            const double sum4 = i.getSum( min4, max4 );
            return static_cast<double>( sum1 + sum4 - sum2 - sum3 ); // optim: 2 area computation only + weighting
         }

         if ( direction == D3XYZ )
         {
            // d g(x, y, z) / dxyz = -8*x*y*z*e^(-x^2 - y^2 - z^2)
            // we approximate this as below

            // computes:
            // view (a) XY
            // X 0 0 0 0 0 0 0 0    X = (0, 0)
            // 0 p p p 0 n n n 0
            // 0 p 1 p 0 n 2 n 0
            // 0 p p p 0 n n n 0
            // 0 0 0 0 0 0 0 0 0
            // 0 n n n 0 p p p 0
            // 0 n 3 n 0 p 4 p 0
            // 0 n n n 0 p p p 0
            // 0 0 0 0 0 0 0 0 0
            //           <=+=> lobeSize

            #ifdef NLL_SECURE
            ensure( ( lobeSize % 2 ) == 1, "must be a odd" );
            #endif

            const core::vector3i min1a( position[ 0 ] - lobeSize, position[ 1 ] - lobeSize, position[ 2 ] - lobeSize );
            const core::vector3i max1a( position[ 0 ] - 1,        position[ 1 ] - 1,        position[ 2 ] - 1 );

            const core::vector3i min2a( position[ 0 ] + 1,        position[ 1 ] - lobeSize, position[ 2 ] - lobeSize );
            const core::vector3i max2a( position[ 0 ] + lobeSize, position[ 1 ] - 1,        position[ 2 ] - 1 );

            const core::vector3i min3a( position[ 0 ] - lobeSize, position[ 1 ] + 1,        position[ 2 ] - lobeSize );
            const core::vector3i max3a( position[ 0 ] - 1,        position[ 1 ] + lobeSize, position[ 2 ] - 1 );

            const core::vector3i min4a( position[ 0 ] + 1,        position[ 1 ] + 1,        position[ 2 ] - lobeSize );
            const core::vector3i max4a( position[ 0 ] + lobeSize, position[ 1 ] + lobeSize, position[ 2 ] - 1 );

            // computes:
            // view (b) XY
            // X 0 0 0 0 0 0 0 0    X = (0, 0)
            // 0 n n n 0 p p p 0
            // 0 n 1 n 0 p 2 p 0
            // 0 n n n 0 p p p 0
            // 0 0 0 0 0 0 0 0 0
            // 0 p p p 0 n n n 0
            // 0 p 3 p 0 n 4 n 0
            // 0 p p p 0 n n n 0
            // 0 0 0 0 0 0 0 0 0
            //           <=+=> lobeSize

            const core::vector3i min1b( position[ 0 ] - lobeSize, position[ 1 ] - lobeSize, position[ 2 ] + 1 );
            const core::vector3i max1b( position[ 0 ] - 1,        position[ 1 ] - 1,        position[ 2 ] + lobeSize );

            const core::vector3i min2b( position[ 0 ] + 1,        position[ 1 ] - lobeSize, position[ 2 ] + 1 );
            const core::vector3i max2b( position[ 0 ] + lobeSize, position[ 1 ] - 1,        position[ 2 ] + lobeSize );

            const core::vector3i min3b( position[ 0 ] - lobeSize, position[ 1 ] + 1,        position[ 2 ] + 1 );
            const core::vector3i max3b( position[ 0 ] - 1,        position[ 1 ] + lobeSize, position[ 2 ] + lobeSize );

            const core::vector3i min4b( position[ 0 ] + 1,        position[ 1 ] + 1,        position[ 2 ] + 1 );
            const core::vector3i max4b( position[ 0 ] + lobeSize, position[ 1 ] + lobeSize, position[ 2 ] + lobeSize );

            const double sum1a = i.getSum( min1a, max1a );
            const double sum2a = i.getSum( min2a, max2a );
            const double sum3a = i.getSum( min3a, max3a );
            const double sum4a = i.getSum( min4a, max4a );

            const double sum1b = i.getSum( min1b, max1b );
            const double sum2b = i.getSum( min2b, max2b );
            const double sum3b = i.getSum( min3b, max3b );
            const double sum4b = i.getSum( min4b, max4b );
            return static_cast<double>( sum1a + sum4a - sum2a - sum3a - sum1b - sum4b + sum2b + sum3b );
         }

         throw std::runtime_error( "unknown 3D Haar feature type!");
      }
   };
}
}

#endif