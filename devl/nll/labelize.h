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

#ifndef NLL_LABELIZE_H_
# define NLL_LABELIZE_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief labelise an image
    @todo improve the speed of the relabeling
    @param T type of the image
    @param Mapper mapper of the image
    @param Different specified if 2 pixels are different
    */
   template <  class T,
               class Mapper = core::IndexMapperRowMajorFlat2DColorRGBn,
               class Different = RegionPixelDifferent<T> >
   class Labelize
   {
   public:
      typedef Different DifferentPixel;
      struct Component
      {
         Component( ui32 px, ui32 py, ui32 s, ui32 i ) : posx( px ), posy( py ), size( s ), id( i ){}
         /// position of the component
		   ui32 posx;
         /// position of the component
         ui32 posy;
         /// size of the component
		   ui32 size;
         /// value in the ouput buffer of the component
		   ui32 id;
      };
      typedef std::vector<Component> Components;

      /**
       @brief result of the label algorithm
       */
      struct ComponentsInfo
      {
         /// components details. If extendedInfo is not set to true, components won't be computed
         Components        components;

         /// resulting mask
         core::ImageMask   labels;
      };

   private:
      // point index id => those of the mask, not image
      typedef std::pair<ui32, ui32>    Edge;
      typedef std::set<Edge>           Edges;

   public:
      Labelize( const Different& diff ) : _diff( diff ){}

      /**
       @brief run the algorithm on an image.

       Label of the first column and first line are discarded

       @param extendedInfo if set, adds information on the labels : size, position
      */
      ComponentsInfo run( core::Image<T, Mapper>& img, bool extendedInfo = false )
      {
         ComponentsInfo info;
         Edges edges;
         ui32 indexId = 1;
         core::ImageMask mask( img.sizex(), img.sizey(), 1 );

         ui32 neighbourTest[ 2 ];
         ui32 neighbourMaskTest[ 2 ];
         ui32 neighbourMaskVal[ 2 ];
         ui32 index;
         for ( ui32 ny = 1; ny < img.sizey(); ++ny )
            for ( ui32 nx = 1; nx < img.sizex(); ++nx )
            {
               ui32 neighbour = 0;
               ui32 pos = img.index( nx, ny, 0 );
               ui32 indexMask = mask.index( nx, ny, 0 );

               index = img.index( nx - 1, ny, 0 );
               if ( _diff( &img[ index ], &img[ pos ] ) )
               {
                  neighbourTest[ neighbour ] = index;
                  neighbourMaskTest[ neighbour ] = mask.index( nx - 1, ny, 0 );
                  neighbourMaskVal[ neighbour ] = mask[ neighbourMaskTest[ neighbour ] ];
                  neighbour += ( neighbourMaskVal[ neighbour ] != 0 );
               }

               index = img.index( nx , ny - 1, 0 );
               if ( _diff( &img[ index ], &img[ pos ] ) )
               {
                  neighbourTest[ neighbour ] = index;
                  neighbourMaskTest[ neighbour ] = mask.index( nx , ny - 1, 0 );
                  neighbourMaskVal[ neighbour ] = mask[ neighbourMaskTest[ neighbour ] ];
                  neighbour += ( neighbourMaskVal[ neighbour ] != 0 );
               }

               if ( neighbour == 0 )
               {
                  mask[ indexMask ] = indexId;
                  ++indexId;
               } else {
                  if ( neighbour == 1 )
                     mask[ indexMask ] = neighbourMaskVal[ 0 ];
                  else
                  {
                     if ( neighbourMaskVal[ 0 ] != neighbourMaskVal[ 1 ])
                     {
                        edges.insert( std::make_pair( neighbourMaskVal[ 0 ], neighbourMaskVal[ 1 ] ) );
                        mask[ indexMask ] = neighbourMaskVal[ 0 ];
                     } else {
                        mask[ indexMask ] = neighbourMaskVal[ 0 ];
                     }
                  }
               } 
            }

         core::Buffer1D<ui32> connectedComponent( indexId, false );
         for ( ui32 n = 0; n < indexId; ++n )
            connectedComponent[ n ] = n;
         core::Buffer1D<bool> runned( indexId );
         for ( ui32 n = 0; n < indexId; ++n )
            _relabel( edges, connectedComponent, runned, n, n );
         core::Buffer1D<ui32> sizeComponent( indexId );
         core::Buffer1D<core::vector2i> posComponent( indexId );
         for ( core::ImageMask::iterator it = mask.begin(); it != mask.end(); ++it )
         {
            ui32 val = connectedComponent[ *it ];
            *it = val;
         }

         if ( extendedInfo )
         {
            for ( ui32 ny = 0; ny < mask.sizey(); ++ny )
               for ( ui32 nx = 0; nx < mask.sizex(); ++nx )
               {
                  ui32 val = connectedComponent[ mask( nx, ny, 0 ) ];
                  ++sizeComponent[ val ];
                  if ( posComponent[ val ][ 0 ] == 0 && posComponent[ val ][ 1 ] == 0 )
                     posComponent[ val ] = core::vector2i( nx, ny );
               }
            for ( ui32 n = 0; n < indexId; ++n )
               if ( sizeComponent[ n ] )
                  info.components.push_back( Component( posComponent[ n ][ 0 ], posComponent[ n ][ 1 ], sizeComponent[ n ], n ) );
         }

         info.labels = mask;
         //debug::writeDebug( "data/after.txt", mask.getBuf(), mask.sizex(), mask.sizey() );
         return info;
      }

   private:
      void _relabel( const Edges& edges, core::Buffer1D<ui32>& connectedComponent, core::Buffer1D<bool>& runned, ui32 generation, ui32 edge )
      {
         for ( Edges::const_iterator it = edges.begin(); it != edges.end(); ++it )
         {
            if ( edge == it->first || edge == it->second )
            {
               if ( !runned[ it->first ] )
               {
                  connectedComponent[ it->first ] = generation;
                  runned[ it->first ] = true;
                  _relabel( edges, connectedComponent, runned, generation, it->first );
               }
               if ( !runned[ it->second ] )
               {
                  connectedComponent[ it->second ] = generation;
                  runned[ it->second ] = true;
                  _relabel( edges, connectedComponent, runned, generation, it->second );
               }
            }
         }
      }

   private:
      Different   _diff;
   };
}
}

#endif
