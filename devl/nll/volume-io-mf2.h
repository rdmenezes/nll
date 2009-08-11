#ifndef NLL_IMAGING_VOLUME_IO_H_
# define NLL_IMAGING_VOLUME_IO_H_

namespace nll
{
namespace imaging
{
   /**
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

      // only with RSI type is handled
      ensure( dataType == 1, "file format not handled" );

      // Create an RSI to maximise dynamic range on each slice
      std::vector< std::pair<double, double> > rsi( depth );

      core::Matrix<double> id( 3, 3 );
      for ( ui32 n = 0; n < 3; ++n )
         id( n, n ) = 1;
      core::Matrix<double> pst = Volume::createPatientSpaceTransform( id, core::vector3d( originX, originY, originZ ), core::vector3d( colSp, rowSp, sliceSp ) );

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
               output( i, j, k ) = static_cast<double>( value ) * rsi[ k ].first + rsi[ k ].second;
            }
         }
      }

      vol = output;
      return true;
   }
}
}

#endif