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

#ifndef NLL_IMAGING_VOLUME_DISTANCETRANSFORM_H_
# define NLL_IMAGING_VOLUME_DISTANCETRANSFORM_H_

namespace nll
{
namespace imaging
{
   namespace impl
   {
      /**
       @ingroup imaging
       @given a specific position (row, slices) it maps the column as a buffer
       */
      template <class T, class VolumeMemoryBufferType>
      class AbstractColumnBufferImage
      {
      public:
         typedef T value_type;
         typedef Volume<T, VolumeMemoryBufferType> VolumeT;

         AbstractColumnBufferImage( VolumeT& i, unsigned r, unsigned s ) : _img( i ), _row( r ), _slice( s )
         {
         }

         T operator[]( unsigned n ) const
         {
            return _img( n, _row, _slice );
         }

         T& operator[]( unsigned n )
         {
            return _img( n, _row, _slice );
         }

      private:
         // not intended to be copied
         AbstractColumnBufferImage operator=( const AbstractColumnBufferImage& );

         VolumeT&                      _img;
         unsigned                      _row;
         unsigned                      _slice;
      };

      /**
       @given a specific position (column, slices) it maps the row as a buffer
       */
      template <class T, class VolumeMemoryBufferType>
      class AbstractRowBufferImage
      {
      public:
         typedef T value_type;
         typedef Volume<T, VolumeMemoryBufferType> VolumeT;

         AbstractRowBufferImage( VolumeT& i, unsigned c, unsigned s ) : _img( i ), _col( c ), _slice( s )
         {
         }

         T operator[]( unsigned n ) const
         {
            return _img( _col, n, _slice );
         }

         T& operator[]( unsigned n )
         {
            return _img( _col, n, _slice );
         }

      private:
         // not intended to be copied
         AbstractRowBufferImage operator=( const AbstractRowBufferImage& );

         VolumeT&                      _img;
         unsigned                      _col;
         unsigned                      _slice;
      };

      /**
       @given a specific position (column, row) it maps the row as a buffer
       */
      template <class T, class VolumeMemoryBufferType>
      class AbstractSliceBufferImage
      {
      public:
         typedef T value_type;
         typedef Volume<T, VolumeMemoryBufferType> VolumeT;

         AbstractSliceBufferImage( VolumeT& i, unsigned c, unsigned r ) : _img( i ), _col( c ), _row( r )
         {
         }

         T operator[]( unsigned n ) const
         {
            return _img( _col, _row, n );
         }

         T& operator[]( unsigned n )
         {
            return _img( _col, _row, n );
         }

      private:
         // not intended to be copied
         AbstractSliceBufferImage operator=( const AbstractSliceBufferImage& );

         VolumeT&                      _img;
         unsigned                      _col;
         unsigned                      _row;
      };
   }

   /**
    @ingroup imaging
    @brief Compute the distance transform of the volume.
    @param img the masked image. For all voxel img(x, y, z) > 0, we will consider it as a source voxel
    */
   template <class T, class VolumeMemoryBufferType>
   std::auto_ptr< VolumeSpatial<float> > distanceTransform( const VolumeSpatial<T, VolumeMemoryBufferType>& img )
   {
      typedef VolumeSpatial<float>   VolumeOut;
      typedef VolumeSpatial<T, VolumeMemoryBufferType>       VolumeIn;

      // we are using wrappers to map 2 dimensions out of 3 so that only is not "fixed"
      // with this, we can design a mechanism avoiding the creation of an intermediate
      // volume, saving a lot of time as the computations can now be done in-place.

      // copy the medical volume to a float volume
      // we will use 2 volumes: 1 for reading and 1 for outputting. Their role
      // will be cyclic so that we avoid the construction of a lot of intermediate volumes
      
      // Dimensions of signal
      const core::vector3ui size = img.getSize();
      const core::vector3i sizei( static_cast<int>( size[ 0 ] ),
                                  static_cast<int>( size[ 1 ] ),
                                  static_cast<int>( size[ 2 ] ) );

      VolumeOut* dt = new VolumeOut( size, img.getPst(), 0 );
      VolumeOut* dt2 = new VolumeOut( size, img.getPst(), 0 );

      try
      {
         // initialize the volume
         #if !defined(NLL_NOT_MULTITHREADED)
         #pragma omp parallel for
         #endif
         for ( int k = 0; k < sizei[ 2 ]; ++k )
         {
            for ( int j = 0; j < sizei[ 1 ]; ++j )
            {
               for ( int i = 0; i < sizei[ 0 ]; ++i )
               {
                  const double val = img( i, j, k );
                  (*dt)( i, j, k ) = val > 0 ? 0 : std::numeric_limits<VolumeOut::value_type>::max();
               }
            }
         }

         // transform along the columns
         #if !defined(NLL_NOT_MULTITHREADED)
         #pragma omp parallel for
         #endif
         for ( int k = 0; k < sizei[ 2 ]; ++k )
         {
            for ( int j = 0; j < sizei[ 1 ]; ++j )
            {
               typedef impl::AbstractColumnBufferImage<float, VolumeOut::VoxelBuffer>    ColumnBuffer;
               ColumnBuffer ColumnWrapper( *dt, j, k );
               ColumnBuffer ColumnWrapperOut( *dt2, j, k );
               core::impl::dt1d<ColumnBuffer, ColumnBuffer>( ColumnWrapper, size[ 0 ], ColumnWrapperOut, img.getSpacing()[0] );
            }
         }

         // transform along the rows
         #if !defined(NLL_NOT_MULTITHREADED)
         #pragma omp parallel for
         #endif
         for ( int k = 0; k < sizei[ 2 ]; ++k )
         {
            for ( int i = 0; i < sizei[ 0 ]; ++i )
            {
               typedef impl::AbstractRowBufferImage<float, VolumeOut::VoxelBuffer>    RowBuffer;
               RowBuffer RowWrapper( *dt2, i, k );
               RowBuffer RowWrapperOut( *dt, i, k );
               core::impl::dt1d<RowBuffer, RowBuffer>( RowWrapper, size[ 1 ], RowWrapperOut, img.getSpacing()[1] );
            }
         }

         // transform along the slices
         #if !defined(NLL_NOT_MULTITHREADED)
         #pragma omp parallel for
         #endif
         for ( int j = 0; j < sizei[ 1 ]; ++j )
         {
            for ( int i = 0; i < sizei[ 0 ]; ++i )
            {
               typedef impl::AbstractSliceBufferImage<float, VolumeOut::VoxelBuffer>    SliceBuffer;
               SliceBuffer sliceWrapper( *dt, i, j );
               SliceBuffer sliceWrapperOut( *dt2, i, j );
               core::impl::dt1d<SliceBuffer, SliceBuffer>( sliceWrapper, size[ 2 ], sliceWrapperOut, img.getSpacing()[2] );
            }
         }
      } catch(...)
      {}

      delete dt;
      return std::auto_ptr< VolumeSpatial<float> >( dt2 );
   }
}
}

#endif