#ifndef NLL_IMAGING_VOLUME_IO_BIN_H_
# define NLL_IMAGING_VOLUME_IO_BIN_H_

#define NLL_IMAGING_BINARY_VOLUME_READER_VERSION   0x12000001

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief Export a spatial volume to a stream
    @note 2 files are actually exported: the binary data and a TXT header.
          The format is the following:
          header: version
                  size
                  background color
                  PST
          binary: line by line, and slice by slice

          T must be a POD

    @param f the name of the file. `f`.hdr and `f`.bin will be created
    */ 
   template <class T, class Storage>
   bool writeVolumeBinary( const VolumeSpatial<T, Storage>&  volume, const std::string& f )
   {
      std::ofstream hdr( ( f + ".hdr" ).c_str() );
      std::ofstream bin( ( f + ".bin" ).c_str(), std::ios::binary );
      if ( hdr.eof() || bin.eof() )
         return false;

      // header
      hdr << "version:" << NLL_IMAGING_BINARY_VOLUME_READER_VERSION << std::endl;
      hdr << "size:" << volume.getSize()[ 0 ] << " " << volume.getSize()[ 1 ] << " " << volume.getSize()[ 2 ] << std::endl;
      hdr << "background:" << volume.getBackgroundValue() << std::endl;
      for ( ui32 ny = 0; ny < 4; ++ny )
      {
         for ( ui32 nx = 0; nx < 4; ++nx )
         {
            hdr << volume.getPst()( ny, nx );
            if ( nx != 3 )
               hdr << " ";
         }
         hdr << std::endl;
      }

      // binary
      for ( ui32 z = 0; z < volume.getSize()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < volume.getSize()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < volume.getSize()[ 0 ]; ++x )
            {
               const T val = volume( x, y, z );
               bin.write( (const char*)&val, sizeof( T ) );
            }
         }
      }

      return true;
   }

   /**
    @ingroup imaging
    @brief Read a volume from a header file and a data file
    */
   template <class T, class Storage>
   bool readVolumeBinary( VolumeSpatial<T, Storage>&  volume, const std::string& f )
   {
      std::ifstream hdr( ( f + ".hdr" ).c_str() );
      std::ifstream bin( ( f + ".bin" ).c_str(), std::ios::binary );
      if ( hdr.eof() || bin.eof() )
         return false;

      std::string line;

      //
      // read header
      //

      // get the version
      std::getline( hdr, line );
      std::vector<const char*> lineSpt = core::split( line, ':' );
      if ( lineSpt.size() != 2 )
         throw std::runtime_error( "error: can't parse volume header: version" );

      ui32 version = atoi( lineSpt[ 1 ] );
      if ( version != NLL_IMAGING_BINARY_VOLUME_READER_VERSION )
         throw std::runtime_error( "error: header version not recognized" );

      // get size
      std::getline( hdr, line );
      lineSpt = core::split( line, ':' );
      if ( lineSpt.size() != 2 )
         throw std::runtime_error( "error: can't parse volume header: size" );
      std::string sizeStr( lineSpt[ 1 ] );
      lineSpt = core::split( sizeStr, ' ' );
      if ( lineSpt.size() != 3 )
         throw std::runtime_error( "error: volume header's size is malformed" );
      core::vector3ui size( atoi( lineSpt[ 0 ] ), atoi( lineSpt[ 1 ] ), atoi( lineSpt[ 2 ] ) );

      // get background
      T background;
      std::stringstream ss;
      
      std::getline( hdr, line );
      lineSpt = core::split( line, ':' );
      if ( lineSpt.size() != 2 )
         throw std::runtime_error( "error: can't parse volume header: background" );
      ss << lineSpt[ 1 ];
      ss >> background;

      // get PSR
      typename VolumeSpatial<T, Storage>::Matrix tfm( 4, 4 );
      for ( ui32 y = 0; y < 4; ++y )
      {

         std::getline( hdr, line );
         lineSpt = core::split( line, ' ' );
         if ( lineSpt.size() != 4 )
            throw std::runtime_error( "error: can't parse volume header: transformation matrix" );
         for ( ui32 x = 0; x < 4; ++x )
         {
            tfm( y, x ) = static_cast<typename VolumeSpatial<T, Storage>::Matrix::value_type>( atof( lineSpt[ x ] ) );
         }
      }
   
      //
      // read binary data
      //
      volume = VolumeSpatial<T, Storage>( size, tfm );
      for ( ui32 z = 0; z < volume.getSize()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < volume.getSize()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < volume.getSize()[ 0 ]; ++x )
            {
               if ( bin.eof() )
                  throw std::runtime_error( "error: read all data of the volume" );
               T val;
               bin.read( (char*)&val, sizeof( T ) );
               volume( x, y, z ) = val;
            }
         }
      }
      return true;
   }
}
}

#endif