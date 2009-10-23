#ifndef MVV_LOADER_HEADER_H_
# define MVV_LOADER_HEADER_H_

# include <nll/nll.h>
# include "resource.h"

namespace mvv
{
   /**
    @brief Defines the volume properties, compatible with nll::volume::VolumeSpatial definitions
    */
   struct VolumeHeader
   {
      typedef std::vector<std::string>    Strings;

      nll::core::Matrix<double>     rotation;   /// the 3x3 rotation matrix
      nll::core::vector3d           origin;     /// the origin of the volume in mm
      nll::core::vector3d           spacing;    /// the spacing of a voxel in mm
      nll::core::vector3ui          size;       /// the number of voxels of the volume
      Strings                       files;      /// the files to read
   };

   /**
    @brief Read the volume's header from a .txt file

    it must be formates as follow:
    origin:
    x1 x2 x3
    spacing:
    spx spy spz
    size:
    sx sy sz
    rotation:
    x1 x2 x3
    y1 y2 y3
    z1 z2 z3
    files:
    fx1
    fx2
    [...]
    */
   inline bool readVolumeHeader( std::istream& i, VolumeHeader& outVolumeHeader )
   {
      if ( !i.good() )
         return false;
      std::string line;
      
      // origin
      std::getline( i, line );
      if ( line != "origin:" )
         return false;
      std::getline( i, line );
      std::vector<const char*> s = nll::core::split( line );
      if ( s.size() != 3 )
         return false;
      outVolumeHeader.origin[ 0 ] = atof( s[ 0 ] );
      outVolumeHeader.origin[ 1 ] = atof( s[ 1 ] );
      outVolumeHeader.origin[ 2 ] = atof( s[ 2 ] );

      // spacing
      std::getline( i, line );
      if ( line != "spacing:" )
         return false;
      std::getline( i, line );
      s = nll::core::split( line );
      if ( s.size() != 3 )
         return false;
      outVolumeHeader.spacing[ 0 ] = atof( s[ 0 ] );
      outVolumeHeader.spacing[ 1 ] = atof( s[ 1 ] );
      outVolumeHeader.spacing[ 2 ] = atof( s[ 2 ] );

      // size
      std::getline( i, line );
      if ( line != "size:" )
         return false;
      std::getline( i, line );
      s = nll::core::split( line );
      if ( s.size() != 3 )
         return false;
      outVolumeHeader.size[ 0 ] = atoi( s[ 0 ] );
      outVolumeHeader.size[ 1 ] = atoi( s[ 1 ] );
      outVolumeHeader.size[ 2 ] = atoi( s[ 2 ] );

      // rotation
      std::getline( i, line );
      if ( line != "rotation:" )
         return false;
      outVolumeHeader.rotation = nll::core::Matrix<double>( 3, 3 );
      for ( unsigned n = 0; n < 3; ++n )
      {
         std::getline( i, line );
         s = nll::core::split( line );
         if ( s.size() != 3 )
            return false;
         outVolumeHeader.rotation( n, 0 ) = atof( s[ 0 ] );
         outVolumeHeader.rotation( n, 1 ) = atof( s[ 1 ] );
         outVolumeHeader.rotation( n, 2 ) = atof( s[ 2 ] );
      }

      // files
      std::getline( i, line );
      if ( line != "files:" )
         return false;
      while ( !i.eof() )
      {
         std::getline( i, line );
         outVolumeHeader.files.push_back( line );
      }
      return outVolumeHeader.files.size() > 0;
   }

   /**
    @brief Load a RAW double volume.

    Binary data are stored line by line, then slice by slice

    @param i points to the header of the volume.
    @return 0 if error
    */
   inline MedicalVolume* loadBinaryRawVolume( std::istream& i )
   {
      VolumeHeader hd;
      bool success = readVolumeHeader( i, hd );
      if ( !success || hd.files.size() != 1 )
         return 0;

      std::ifstream  f( hd.files[ 0 ].c_str(), std::ios::binary );
      if ( !f.good() )
         return 0;

      MedicalVolume::Matrix trans = MedicalVolume::createPatientSpaceTransform( hd.rotation,
                                                                                hd.origin,
                                                                                hd.spacing );
      MedicalVolume* volume = new MedicalVolume( hd.size,
                                                 trans,
                                                 0,
                                                 false );
      for ( ui32 nz = 0; nz < hd.size[ 2 ]; ++nz )
      {
         for ( ui32 ny = 0; ny < hd.size[ 1 ]; ++ny )
         {
            for ( ui32 nx = 0; nx < hd.size[ 0 ]; ++nx )
            {
               double val;
               nll::core::read<double>( val, f );
               ( *volume )( nx, ny, nz ) = val;
            }
         }
      }
      return volume;
   }

   /**
    @brief Load a text volume.

    Binary data are stored line by line, then slice by slice

    @param i points to the header of the volume.
    @return 0 if error
    */
   inline MedicalVolume* loadTextRawVolume( std::istream& i, const std::string& base )
   {
      VolumeHeader hd;
      bool success = readVolumeHeader( i, hd );

      // the number of files must be equal to the number of slices
      if ( !success || hd.files.size() != hd.size[ 2 ] )
         return 0;
      MedicalVolume::Matrix trans = MedicalVolume::createPatientSpaceTransform( hd.rotation,
                                                                                hd.origin,
                                                                                hd.spacing );
      MedicalVolume* volume = new MedicalVolume( hd.size,
                                                 trans,
                                                 0,
                                                 false );

      for ( ui32 nz = 0; nz < hd.size[ 2 ]; ++nz )
      {
         std::ifstream  f( ( base + hd.files[ nz ] ).c_str() );
         if ( !f.good() )
         {
            delete volume;
            return 0;
         }

      
         for ( ui32 ny = 0; ny < hd.size[ 1 ]; ++ny )
         {
            for ( ui32 nx = 0; nx < hd.size[ 0 ]; ++nx )
            {
               if ( f.eof() )
               {
                  delete volume;
                  return 0;
               }

               double val;
               f >> val;
               ( *volume )( nx, ny, nz ) = val;
            }
         }
      }
      return volume;
   }

   inline MedicalVolume* loadTextRawVolume( const std::string& file )
   {
      std::ifstream f( file.c_str() );
      size_t l = file.find_last_of( '/' );
      std::string directory = "";
      if ( l != std::string::npos )
         directory = file.substr( 0, l ) + "/";
      if ( !f.good() )
         return 0;
      return loadTextRawVolume( f, directory );
   }
}

#endif