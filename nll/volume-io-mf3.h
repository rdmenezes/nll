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

#ifndef NLL_IMAGING_VOLUME_IO_MF3_H_
# define NLL_IMAGING_VOLUME_IO_MF3_H_

namespace nll
{
namespace imaging
{
   /**
    @brief Format using a very simple compression technique
    
    It is simply taking advange that most of the voxels are identical when linearly read. Note that the compression used is
    slightly destructive
    */
   class VolumeMf3Format
   {
      typedef ui16   DiscretizationType;
      enum Mf3ID
      {
         formatVersionNumber = 3
      };

   public:
      template <class T, class VolumeMemoryBufferType>
      void write( const imaging::VolumeSpatial<T, VolumeMemoryBufferType>& vol, std::ostream& file ) const
      {
         
         if ( !file.good() )
            throw std::runtime_error( "file error" );

         // first discretize the volume with to the desired level
         VolumeDiscretizer discretizer;

         std::vector<Rsi> rsi;
         imaging::VolumeMemoryBuffer<DiscretizationType> buffer;
         discretizer.discretize( vol, buffer, rsi );

         // encode the volume's data
         algorithm::DataCompressorCount compressor( 15 );
         algorithm::DataCompressorCount::Chunks<DiscretizationType> chunks;
         compressor.compress<const DiscretizationType*>( buffer.begin(), buffer.end(), chunks );

         // export the volume's data
         // 0 : the format used
         unsigned int firstWord = (unsigned int)formatVersionNumber;
         file.write( (char*)&firstWord, sizeof( unsigned int ) );

         // Dimensions
         const unsigned int width  = vol.getSize()[ 0 ];
         const unsigned int height = vol.getSize()[ 1 ];
         const unsigned int depth  = vol.getSize()[ 2 ];

         file.write( (char*)&width, sizeof( unsigned int ) );
         file.write( (char*)&height, sizeof( unsigned int ) );
         file.write( (char*)&depth, sizeof( unsigned int ) );

         core::write< std::vector<Rsi> >( rsi, file );

         // PST
         vol.getPst().write( file );
         
         // now the chunks
         chunks.write( file );
      }

      template <class T, class VolumeMemoryBufferType>
      void read( imaging::VolumeSpatial<T, VolumeMemoryBufferType>& vol, std::istream& file ) const
      {
         typedef imaging::VolumeSpatial<T, VolumeMemoryBufferType> Volume;

         if ( !file.good() )
            throw std::runtime_error( "file error" );

         // export the volume's data
         // 0 : the format used
         unsigned int firstWord = 0;
         file.read( (char*)&firstWord, sizeof( unsigned int ) );
         ensure( firstWord == formatVersionNumber, "wrong file: version number doesn't match" );

         // Dimensions
         unsigned int width;
         unsigned int height;
         unsigned int depth;

         file.read( (char*)&width,  sizeof( unsigned int ) );
         file.read( (char*)&height, sizeof( unsigned int ) );
         file.read( (char*)&depth,  sizeof( unsigned int ) );

         std::vector<Rsi> rsi;
         core::read< std::vector<Rsi> >( rsi, file );
         
         typename Volume::Matrix pst;
         pst.read( file );

         
         // now the chunks
         algorithm::DataCompressorCount::Chunks<DiscretizationType> chunks;
         chunks.read( file );

         std::auto_ptr<DiscretizationType> vals( new DiscretizationType[ chunks.dataSize() ] );
         chunks.decode( vals.get() );


         size_t index = 0;
         imaging::VolumeSpatial<T, VolumeMemoryBufferType> volumeTmp( core::vector3ui( width, height, depth ), pst );
         vol = volumeTmp;
         for ( unsigned int k = 0; k < depth; ++k )
         {
            for ( unsigned int j = 0; j < height; ++j )
            {
               for ( unsigned int i = 0; i < width; ++i )
               {
                  vol( i, j, k ) = static_cast<T>( rsi[ k ].slope * vals.get()[ index++ ] + rsi[ k ].intercept );
               }
            }
         }
      }
   };
}
}

#endif