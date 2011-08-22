#include <mvvScript/Compiler.h>

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

#include <mvvScript/function-runnable.h>
#include "dataset.h"
#include <mvvDicomTools/utils.h>
#include <mvvDicomTools/dicom-attributs.h>

namespace mvv
{
namespace mapper
{
   std::auto_ptr<LandmarkDataset::Volume> LandmarkDataset::loadData( unsigned id ) const
   {
      const std::string path = _dataPath[ id ];
      size_t last = path.find_last_of( '.' );
      if ( last != std::string::npos )
      {
         std::string extension( path.begin() + last, path.end() );
         if ( extension == ".mf2" )
         {
            std::auto_ptr<Volume> data( new Volume() );
            bool loaded = nll::imaging::loadSimpleFlatFile( path, *data );
            ensure( loaded, "data loading failed:" + path );
            return data;
         } else {
            throw std::runtime_error( "unkonown extension:" + extension );
         }
      }

      DicomDatasets dicom;
      dicom.loadDicomDirectory( path );
      ensure( dicom.getSeriesUids().size() == 1, "data problem: Dicom directory contains several volumes:" + path );
      return std::auto_ptr<Volume>( dicom.constructVolumeFromSeries<float>( 0 ) );
   }
}
}