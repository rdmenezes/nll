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

#ifndef NLL_ALGORITHM_INTEGRAL_IMAGE_3D_H_
# define NLL_ALGORITHM_INTEGRAL_IMAGE_3D_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Integral image allowing for sum computation independent of the number of voxels to be checked

    It is defined such as i(x, y, z) = sum( image( x', y', z'))_{x' <= x && y' <= y && z' <= z}

    Todo that, we need to pass:
    - 1 pass) all the slices are summed in 2D exactly as in the 2D case
    - 2 pass) add the sum of the previous slice to the current slice

    The processing is intensively done on the XY plane
    */
   class IntegralImage3d
   {
   public:
      typedef double                         value_type;
      typedef imaging::Volume<value_type>    Storage;

   public:
      value_type operator()( ui32 x, ui32 y, ui32 z ) const
      {
         return _img( x, y, z );
      }

      value_type& operator()( ui32 x, ui32 y, ui32 z )
      {
         return _img( x, y, z );
      }

      value_type& operator()( const core::vector3ui& p )
      {
         return _img( p[ 0 ], p[ 1 ], p[ 2 ] );
      }

      value_type operator()( const core::vector3ui& p ) const
      {
         return _img( p[ 0 ], p[ 1 ], p[ 2 ] );
      }

      template <class Volume>
      void construct( const Volume& v )
      {
         typedef typename Volume::DirectionalIterator Iterator;
         typedef typename Storage::DirectionalIterator IteratorImg;

         // first pass: compute the sum independently for all slices
         const ui32 sizex = v.size()[ 0 ];
         const ui32 sizey = v.size()[ 1 ];
         const ui32 sizez = v.size()[ 2 ];
         _img = Storage( sizex, sizey, sizez, 0, false );

         std::vector<value_type> s( sizex );
         std::vector<value_type> ii( sizex );
         for ( int z = 0; z < (int)sizez; ++z )
         {
            // init first line
            IteratorImg it = _img.getIterator( 0, 0, z );
            Iterator itSrc = v.getIterator( 0, 0, z );
            
            s[ 0 ] = *itSrc;
            *it = *itSrc;
            value_type previous_ii = s[ 0 ];
            it.addx();
            itSrc.addx();
            for ( ui32 x = 1; x < sizex; ++x )
            {
               s[ x ] = *itSrc;
               *it = previous_ii + s[ x ];

               previous_ii = *it;
               it.addx();
               itSrc.addx();
            }
            
            // computes the other rows
            for ( ui32 y = 1; y < sizey; ++y )
            {
               // init
               itSrc = v.getIterator( 0, y, z );
               it = _img.getIterator( 0, y, z );
               
            
               value_type tmp = *itSrc;
               s[ 0 ] = s[ 0 ] + static_cast<value_type>( tmp );

               // first step
               *it = s[ 0 ];
               previous_ii = s[ 0 ];

               it.addx();
               itSrc.addx();

               // main loop
               for ( ui32 x = 1; x < sizex; ++x )
               {
                  s[ x ] = s[ x ] + static_cast<value_type>( *itSrc );
                  *it = previous_ii + s[ x ];
                  previous_ii = *it;

                  it.addx();
                  itSrc.addx();
               }
            }
         }

         // second pass: accumulate the values of the preceding slices
         for ( ui32 z = 1; z < sizez; ++z )
         {
            for ( ui32 y = 0; y < sizey; ++y )
            {
               Iterator itSrc = _img.getIterator( 0, y, z );
               Iterator itSrcPrev = _img.getIterator( 0, y, z - 1 );

               Iterator itSrcX = itSrc;
               Iterator itSrcPrevX = itSrcPrev;
               for ( ui32 x = 0; x < sizex; ++x )
               {
                  *itSrcX += *itSrcPrevX;
                  itSrcX.addx();
                  itSrcPrevX.addx();
               }
               itSrc.addy();
               itSrcPrev.addy();
            }
         }
      }

      /**
       @brief Return the storage of the integral image
       */
      const Storage& getStorage() const
      {
         return _img;
      }

      /**
       @brief Returns of the voxel bounded by <bottomLeft> and <topRight>. The bounds are _inclusive_
       */
      value_type getSum( const core::vector3ui& bottomLeft,
                         const core::vector3ui& topRight ) const
      {
         core::vector3ui bl( bottomLeft[ 0 ] - 1, bottomLeft[ 1 ] - 1, bottomLeft[ 2 ] - 1 );

         // special case if one of the index is 0, we nee to check everything...
         if ( bottomLeft[ 0 ] == 0 || bottomLeft[ 1 ] == 0 || bottomLeft[ 2 ] == 0 )
         {
            // reset the wrong coordinate (will loop over ui32 max value)
            value_type v111 = 0, v110 = 0, v101 = 0, v100 = 0, v011 = 0, v010 = 0, v001 = 0, v000 = 0;

            if ( bottomLeft[ 0 ] != 0 )
            {
               v011 = _img( bl[ 0 ], topRight[ 1 ], topRight[ 2 ] );
               if ( bottomLeft[ 2 ] != 0 )
                  v010 = _img( bl[ 0 ], topRight[ 1 ], bl[ 2 ] );

               if ( bottomLeft[ 1 ] != 0 )
               {
                  v001 = _img( bl[ 0 ], bl[ 1 ], topRight[ 2 ] );
                  if ( bottomLeft[ 2 ] != 0 )
                     v000 = _img( bl[ 0 ], bl[ 1 ], bl[ 2 ] );
               }
            }

            v111 = _img( topRight[ 0 ], topRight[ 1 ], topRight[ 2 ] );

            if ( bottomLeft[ 2 ] != 0 )
               v110 = _img( topRight[ 0 ], topRight[ 1 ], bl[ 2 ] );

            if ( bottomLeft[ 1 ] != 0 )
            {
               v101 = _img( topRight[ 0 ], bl[ 1 ], topRight[ 2 ] );
               if ( bottomLeft[ 2 ] != 0 )
               {
                  v100 = _img( topRight[ 0 ], bl[ 1 ], bl[ 2 ]);
               }
            }
            return v111 - v110 + v001 - v000 - ( v101 - v100 ) - ( v011 - v010 );
         }

         // else in the vast majority of the case
         value_type a = _img( topRight[ 0 ], topRight[ 1 ], topRight[ 2 ] ) -
                        _img( topRight[ 0 ], topRight[ 1 ], bl[ 2 ] );
         value_type b = _img( topRight[ 0 ], bl[ 1 ], topRight[ 2 ] ) -
                        _img( topRight[ 0 ], bl[ 1 ], bl[ 2 ]);
         value_type c = _img( bl[ 0 ], topRight[ 1 ], topRight[ 2 ] ) -
                        _img( bl[ 0 ], topRight[ 1 ], bl[ 2 ] );
         value_type d = _img( bl[ 0 ], bl[ 1 ], topRight[ 2 ] ) -
                        _img( bl[ 0 ], bl[ 1 ], bl[ 2 ] );
         return a + d - b - c;
      }

   private:
      Storage     _img;
   };
}
}

#endif