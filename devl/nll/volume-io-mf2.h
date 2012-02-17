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

#ifndef NLL_IMAGING_VOLUME_IO_H_
# define NLL_IMAGING_VOLUME_IO_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    Load a .mf2 medical volume. Not a standard file format, just a helper.
    */
   template <class T, class Buf>
   bool loadSimpleFlatFile( const std::string& filename, VolumeSpatial<T, Buf>& vol )
   {
      typedef VolumeSpatial<T, Buf> Volume;

      // open the file
      std::ifstream file( filename.c_str(), std::ios::binary | std::ios::in );
      if ( !file.good() )
      {
         return false;
      }

      // First of all, work out if we've got the new format that contains more information or not
      unsigned int firstWord = 0;
      file.read( (char*)&firstWord, sizeof( unsigned int ) );
      
      // If new format
      unsigned int width  = 0;
      unsigned int height = 0;
      unsigned int depth  = 0;
      unsigned int dataType = 0;
      if ( firstWord == 0 )
      {
         // Get the version number
         unsigned int formatVersionNumber = 0;
         file.read( (char*)&formatVersionNumber, sizeof( unsigned int ) );
         if ( formatVersionNumber >= 2 )
         {
            // Read datatype flag
            file.read( (char*)&dataType, sizeof( unsigned int ) );
         }

         // Dimensions
         file.read( (char*)&width, sizeof( unsigned int ) );
         file.read( (char*)&height, sizeof( unsigned int ) );
         file.read( (char*)&depth, sizeof( unsigned int ) );
      }
      else
      {
         // Old format - we've actually read the width...
         width = firstWord;
         file.read( (char*)&height, sizeof( unsigned int ) );
         file.read( (char*)&depth, sizeof( unsigned int ) );
      }

      // Spacing
      double colSp = 0;
      double rowSp = 0;
      double sliceSp = 0;
      file.read( (char*)&colSp, sizeof( double ) );
      file.read( (char*)&rowSp, sizeof( double ) );
      file.read( (char*)&sliceSp, sizeof( double ) );

      // Origin
      double originX = 0;
      double originY = 0;
      double originZ = 0;
      file.read( (char*)&originX, sizeof( double ) );
      file.read( (char*)&originY, sizeof( double ) );
      file.read( (char*)&originZ, sizeof( double ) );

      // If we're expecting uint16 data with RSI in the file
      if ( dataType == 1 )
      {
         // Create an RSI to maximise dynamic range on each slice
         std::vector< std::pair<double, double> > rsi( depth );

         core::Matrix<float> id( 3, 3 );
         for ( ui32 n = 0; n < 3; ++n )
            id( n, n ) = 1;
         // origin is from worldorigin->volumeorigin
         core::Matrix<float> pst = Volume::createPatientSpaceTransform( id, core::vector3f( (float)-originX, (float)-originY, (float)-originZ ), core::vector3f( (float)colSp, (float)rowSp, (float)sliceSp ) );

         for ( unsigned int k = 0; k < depth; ++k )
         {
            double slope = 0;
            double intercept = 0;
            file.read( (char*)&slope, sizeof( double ) );
            file.read( (char*)&intercept, sizeof( double ) );
            rsi[k] = std::make_pair( slope, intercept );
         }

         Volume output( core::vector3ui( width,
                                         height,
                                         depth ),
                        pst );

         for ( unsigned int k = 0; k < depth; ++k )
         {
            for ( unsigned int j = 0; j < height; ++j )
            {
               for ( unsigned int i = 0; i < width; ++i )
               {
                  ensure( !file.eof(), "unexpected eof" );
                  unsigned short value = 0;
                  file.read( (char*)&value, sizeof( unsigned short ) );
                  output( i, j, k ) = static_cast<T>( value * rsi[ k ].first + rsi[ k ].second );
               }
            }
         }

         vol = output;
         return true;
      }

      // expecting a simple float buffer
      if ( dataType == 0 )
      {
         core::Matrix<float> id( 3, 3 );
         for ( ui32 n = 0; n < 3; ++n )
            id( n, n ) = 1;
         core::Matrix<float> pst = Volume::createPatientSpaceTransform( id, core::vector3f( (float)-originX, (float)-originY, (float)-originZ ), core::vector3f( (float)colSp, (float)rowSp, (float)sliceSp ) );
         Volume output( core::vector3ui( width,
                                         height,
                                         depth ),
                        pst );

         // Data
         for ( unsigned int k = 0; k < depth; ++k )
         {
            for ( unsigned int j = 0; j < height; ++j )
            {
               for ( unsigned int i = 0; i < width; ++i )
               {
                  float value = 0;
                  file.read( (char*)&value, sizeof( float ) );
                  output( i, j, k ) = value;
               }
            }
         }

         vol = output;
         return true;
      }

      return false;
   }

   /**
    @ingroup imaging
    Write a .mf2 medical volume. Not a standard file format, just a helper.
    */
   template <class T, class Buf>
   bool saveSimpleFlatFile( const std::string& filename, VolumeSpatial<T, Buf>& vol )
   {
      typedef VolumeSpatial<T, Buf> Volume;

      // open the file
      std::ofstream file( filename.c_str(), std::ios::binary | std::ios::out );
      if ( !file.good() )
      {
         return false;
      }

      // 0 : new format
      unsigned int firstWord = 0;
      file.write( (char*)&firstWord, sizeof( unsigned int ) );
      
      // If new format
      unsigned int width  = vol.getSize()[ 0 ];
      unsigned int height = vol.getSize()[ 1 ];
      unsigned int depth  = vol.getSize()[ 2 ];
      unsigned int dataType = 0; // we don't want RSI info

      // Get the version number
      unsigned int formatVersionNumber = 3;
      file.write( (char*)&formatVersionNumber, sizeof( unsigned int ) );
      file.write( (char*)&dataType, sizeof( unsigned int ) );

      // Dimensions
      file.write( (char*)&width, sizeof( unsigned int ) );
      file.write( (char*)&height, sizeof( unsigned int ) );
      file.write( (char*)&depth, sizeof( unsigned int ) );
      

      // Spacing
      double colSp = vol.getSpacing()[ 0 ];
      double rowSp = vol.getSpacing()[ 1 ];
      double sliceSp = vol.getSpacing()[ 2 ];
      file.write( (char*)&colSp, sizeof( double ) );
      file.write( (char*)&rowSp, sizeof( double ) );
      file.write( (char*)&sliceSp, sizeof( double ) );

      // Origin
      double originX = - vol.getOrigin()[ 0 ];   // in MF2 origin = (1 voxel)->(0,0,0), volume is opposite!
      double originY = - vol.getOrigin()[ 1 ];
      double originZ = - vol.getOrigin()[ 2 ];
      file.write( (char*)&originX, sizeof( double ) );
      file.write( (char*)&originY, sizeof( double ) );
      file.write( (char*)&originZ, sizeof( double ) );

      // Data
      for ( unsigned int k = 0; k < depth; ++k )
      {
         for ( unsigned int j = 0; j < height; ++j )
         {
            for ( unsigned int i = 0; i < width; ++i )
            {
               float value = static_cast<float>( vol( i, j, k ) );
               file.write( (char*)&value, sizeof( float ) );
            }
         }
      }

      return true;
   }
}
}

#endif
