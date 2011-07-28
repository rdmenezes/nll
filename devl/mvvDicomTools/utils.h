#ifndef MVV_DICOM_TOOLS_UTILS_H_
# define MVV_DICOM_TOOLS_UTILS_H_

# include <Boost/filesystem.hpp>
# include <dcmtk/dcfilefo.h>
# include <dcmtk/dcdeftag.h>
# include <dcmtk/dcmimage.h>
# include <mvvScript/function-runnable.h>

using namespace boost::filesystem;
using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

namespace
{
   /**
    @brief Utility to recursively return all files in a given source folder
    */
   inline void recursiveFileList( const std::string& folder, std::vector<std::string>& files )
   {
      try
      {
         files.clear();
         path p = path( folder );
         if ( exists( p ) )
         {
            if ( is_directory(p) )
            {
               recursive_directory_iterator it = recursive_directory_iterator(p);
               while ( it != recursive_directory_iterator() )
               {
                  if ( is_regular_file( it->status() ) )
                  {
                     files.push_back( it->path().file_string() );
                  }
                  ++it;
               }
            } else {
               throw std::runtime_error( folder + " is not a directory" );
            }
         } else {
            throw std::runtime_error( "directory doesn't exist:" + folder );
         }
      } catch ( const filesystem_error& ex )
      {
         throw std::runtime_error( "exception: " + std::string( ex.what() ) );
      }
   }

   /**
    @brief Simple wrapper to decouple Dcmtk classes and this module. This would also take care of the Dicom
           subtilities (e.g., DICOM tag missing and find an equivalent one)
    */
   class DicomWrapper
   {
   public:
      DicomWrapper( DcmDataset& dataset, bool throwIfMissingTag = false ) : _dataset( dataset ), _throwIfMissingTag( throwIfMissingTag )
      {
      }

      const char* getSeriesInstanceUid()
      {
         const char* seriesInstanceUid = 0;
         OFCondition cond = _dataset.findAndGetString( DCM_SeriesInstanceUID, seriesInstanceUid );
         if ( cond.good() )
         {
         } else {
            error( "missing DCM_SeriesInstanceUID tag" );
         }
         return seriesInstanceUid;
      }

      const char* getStudyInstanceUid()
      {
         const char* instanceUid = 0;
         OFCondition cond = _dataset.findAndGetString( DCM_StudyInstanceUID, instanceUid );
         if ( cond.good() )
         {
         } else {
            error( "missing DCM_SeriesInstanceUID tag" );
         }
         return instanceUid;
      }

      void getImageOrientationPatient( nll::core::vector3f& x, nll::core::vector3f& y )
      {
      }

   private:
      void error( const std::string& msg )
      {
         if ( _throwIfMissingTag )
         {
            throw std::runtime_error( msg );
         }
      }

   private:
      bool        _throwIfMissingTag;
      DcmDataset  _dataset;
   };

   /**
    * Utility class for handling DICOM datasets
    */
   class DicomDatasets
   {
   public:
      typedef std::vector<DcmFileFormat>  DicomFiles;

   public:
      /**
       @brief Load a dicom directory and populate the different series and study instance UIDs
       */
      void loadDicomDirectory( const std::string& dir )
      {
         std::cout << "loading DICOM directory..." << std::endl;

         // get the potential DICOM files
         std::vector< std::string > files;
         recursiveFileList( dir, files );

         // load the DICOM hearders and sort them by series instance UID
         typedef std::map<mvv::Symbol, size_t> DicomUids;

         DicomUids seriesUids;
         for ( ui32 n = 0; n < files.size(); ++n )
         {
            DcmFileFormat dcm;
            OFCondition cond = dcm.loadFile( files[ n ].c_str() );
            if ( cond.good() )
            {
               DicomWrapper wrapper( *dcm.getDataset() );
               const char* seriesInstanceUid = wrapper.getSeriesInstanceUid();
               if ( seriesInstanceUid )
               {
                  size_t index;
                  mvv::Symbol seriesInstanceUidSymbol = mvv::Symbol::create( seriesInstanceUid );
                  DicomUids::iterator it = seriesUids.find( seriesInstanceUidSymbol );
                  if ( it == seriesUids.end() ) // it is a new serie instance UID
                  {
                     std::cout << "new SUID=" << seriesInstanceUid << std::endl;
                     index = _dicomBySeriesUid.size(); // create a new one
                     _dicomBySeriesUid.push_back( DicomFiles() );
                     seriesUids[ seriesInstanceUidSymbol ] = index;
                  } else {
                     index = it->second;  // retrieve the position
                  }
                  
                  // finally add the slice at its correct location
                  _dicomBySeriesUid[ index ].push_back( dcm );
               }
            }
         }

         // finally, sort by study UID
         DicomUids studyUids;
         for ( size_t n = 0; n < _dicomBySeriesUid.size(); ++n )
         {
            DicomWrapper wrapper( *_dicomBySeriesUid[ n ][ 0 ].getDataset() );
            const char* studyInstanceUid = wrapper.getStudyInstanceUid();
            if ( studyInstanceUid )
            {
               size_t index;
               mvv::Symbol studyInstanceUidSymbol = mvv::Symbol::create( studyInstanceUid );
               DicomUids::iterator it = studyUids.find( studyInstanceUidSymbol );
               if ( it == studyUids.end() )
               {
                  std::cout << "new IUID=" << studyInstanceUidSymbol << std::endl;
                  index = _dicomByStudyUid.size();
                  _dicomByStudyUid.push_back( std::vector< size_t >() );
                  studyUids[ studyInstanceUidSymbol ] = index;
               } else {
                  index = it->second;
               }

               _dicomByStudyUid[ index ].push_back( n );
            }
         }

         std::cout << "nb series UID=" << _dicomBySeriesUid.size() << std::endl;
         std::cout << "nb study UID=" << _dicomByStudyUid.size() << std::endl;
      }

      void clear()
      {
         _dicomBySeriesUid.clear();
         _dicomByStudyUid.clear();
      }

      /**
       @brief Correctly arrange the slices.

        This is using two tags ImageOrientationPatient <0020,0037>
                           and ImagePositionPatient <0020,0032>
        The image orientation for a specific UID must match, then the image position is projected on the
        normal of the slice. Finally the slices are sorted according to this distance.
       */
      void sortSeriesUids()
      {
         // determine if we have the tag <0020, 0013> Instance Number to sort the image
         std::vector< std::pair< ui32, ui32 > > list;
         for ( size_t n = 0; n < _dicomBySeriesUid.size(); ++n )
         {
            f64* orientation;
            for ( size_t slice = 0; slice < _dicomBySeriesUid[ n ].size(); ++slice )
            {

               DcmDataset& dataset = *_dicomBySeriesUid[ n ][ slice ].getDataset();
               OFString x1, y1, z1;
               OFCondition cond1 = dataset.findAndGetOFString( DCM_ImageOrientationPatient, x1, 0 );
               OFCondition cond2 = dataset.findAndGetOFString( DCM_ImageOrientationPatient, y1, 1 );
               OFCondition cond3 = dataset.findAndGetOFString( DCM_ImageOrientationPatient, z1, 2 );
               //OFCondition cond = dataset.findAndGetFloat64Array( DCM_ImageOrientationPatient, orientation );
               if ( cond1.good() && cond2.good() && cond3.good() )
               {
                  std::cout << "ImageOrientationPatient=" << x1 << " " << y1 << " " << z1 << std::endl;
               } else {
                  std::cout << "error=" << cond1.text() << std::endl;
               }
               //findAndGetFloat32Array 
            }
         }
         //DicomImage i;
      }

      /**
       @brief return all the series UID found
       */
      std::vector< DicomFiles >& getSeriesUids()
      {
         return _dicomBySeriesUid;
      }

      /**
       @brief return all the series UID found
       */
      const std::vector< DicomFiles >& getSeriesUids() const
      {
         return _dicomBySeriesUid;
      }

      /**
       @brief return all the study UID found
       */
      const std::vector< std::vector< size_t > >& getStudyUids() const
      {
         return _dicomByStudyUid;
      }

   private:
      std::vector< DicomFiles >             _dicomBySeriesUid;      // each entry represents a SeriesInstanceUid
      std::vector< std::vector< size_t > >  _dicomByStudyUid;       // [ x ][ y ], [ x ] is a set of index representing the set of SeriesUid belonging to the same Study Uid
   };
}

#endif