#ifndef MVV_DICOM_TOOLS_UTILS_H_
# define MVV_DICOM_TOOLS_UTILS_H_

# include "mvvDicomTools.h"
# include <Boost/filesystem.hpp>
# include <dcmtk/dcmdata/dcfilefo.h>
# include <dcmtk/dcmdata/dcdeftag.h>
# include <dcmtk/dcmimgle/dcmimage.h>
# include <dcmtk/dcmdata/dcxfer.h>
# include <memory>
# include <mvvScript/function-runnable.h>

using namespace boost::filesystem;
using namespace mvv::platform;
using namespace mvv::parser;

// TODO this is a quick fix for <RegistrationImpl> project: the tool to export DICOM volumes inverts y axis, if the DICOM is not reading for this particular
// the flag should be disabled!
#define ORIENTATION_FOR_MATLAB_COMPABILITY

namespace mvv
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
         return getString( DCM_SeriesInstanceUID );
      }

      void setSeriesInstanceUid( const char* s )
      {
         setString( DCM_SeriesInstanceUID, s );
      }

      const char* getPatientName()
      {
         return getString( DCM_PatientName );
      }

      void setPatientName( const char* name )
      {
         setString( DCM_PatientName, name );
      }

      const char* getPhotometricInterpretation()
      {
         return getString( DCM_PhotometricInterpretation );
      }

      void setPhotometricInterpretation( const char* name )
      {
         setString( DCM_PhotometricInterpretation, name );
      }

      const char* getAcquisitionNumber()
      {
         return getString( DCM_AcquisitionNumber );
      }

      void setAcquisitionNumber( const char* name )
      {
         setString( DCM_AcquisitionNumber, name );
      }

      const char* getKvp()
      {
         return getString( DCM_KVP );
      }

      void setKvp( const char* name )
      {
         setString( DCM_KVP, name );
      }

      std::vector<std::string> getImageType()
      {
         std::vector<std::string> strs;
         getStrings( DCM_ImageType, strs );
         return strs;
      }

      void setImageType( const std::vector<std::string>& strings )
      {
         setStrings( DCM_ImageType, strings );
      }

      const char* getSliceThickness()
      {
         return getString( DCM_SliceThickness );
      }

      void setSliceThickness( const char* name )
      {
         setString( DCM_SliceThickness, name );
      }

      const char* getManufacturer()
      {
         return getString( DCM_Manufacturer );
      }

      void setManufacturer( const char* name )
      {
         setString( DCM_Manufacturer, name );
      }

      const char* getPositionReferenceIndicator()
      {
         return getString( DCM_PositionReferenceIndicator );
      }

      void setPositionReferenceIndicator( const char* name )
      {
         setString( DCM_PositionReferenceIndicator, name );
      }

      const char* getSeriesNumber()
      {
         return getString( DCM_SeriesNumber );
      }

      void setSeriesNumber( const char* name )
      {
         setString( DCM_SeriesNumber, name );
      }

      const char* getPatientPosition()
      {
         return getString( DCM_PatientPosition );
      }

      void setPatientPosition( const char* name )
      {
         setString( DCM_PatientPosition, name );
      }

      const char* getReferringPhysician()
      {
         return getString( DCM_ReferringPhysicianName );
      }

      void setReferringPhysician( const char* name )
      {
         setString( DCM_ReferringPhysicianName, name );
      }

      const char* getAccessionNumber()
      {
         return getString( DCM_AccessionNumber );
      }

      void setAccessionNumber( const char* name )
      {
         setString( DCM_AccessionNumber, name );
      }

      const char* getRescaleType()
      {
         return getString( DCM_RescaleType );
      }

      void setRescaleType( const char* name )
      {
         setString( DCM_RescaleType, name );
      }

      const char* getPatientBirthDate()
      {
         return getString( DCM_PatientBirthDate );
      }

      void setPatientBirthDate( const char* name )
      {
         setString( DCM_PatientBirthDate, name );
      }

      const char* getPatientId()
      {
         return getString( DCM_PatientID );
      }

      void setPatientId( const char* s )
      {
         setString( DCM_PatientID, s );
      }

      const char* getPatientSex()
      {
         return getString( DCM_PatientSex );
      }

      void setPatientSex( const char* s )
      {
         setString( DCM_PatientSex, s );
      }

      const char* getStudyInstanceUid()
      {
         return getString( DCM_StudyInstanceUID );
      }

      void setStudyInstanceUid( const char* s )
      {
         setString( DCM_StudyInstanceUID, s );
      }

      float getPatientAge()
      {
         return convertPatientAge( getString( DCM_PatientAge ) );
      }

      void setPatientAge( float s )
      {
         // TODO: do the real conversion!!!
         setString( DCM_PatientAge, "001Y" );
      }

      const char* getStudyDate()
      {
         return getString( DCM_StudyDate );
      }

      void setStudyDate( const char* s )
      {
         setString( DCM_StudyDate, s );
      }

      const char* getStudyTime()
      {
         return getString( DCM_StudyTime );
      }

      void setStudyTime( const char* s )
      {
         setString( DCM_StudyTime, s );
      }

      const char* getStudyDescription()
      {
         return getString( DCM_StudyDescription );
      }

      void setStudyDescription( const char* s )
      {
         setString( DCM_StudyDescription, s );
      }

      const char* getStudyId()
      {
         return getString( DCM_StudyID );
      }

      void setStudyId( const char* s )
      {
         setString( DCM_StudyID, s );
      }

      const char* getSopInstanceUid()
      {
         return getString( DCM_SOPInstanceUID );
      }

      void setSopInstanceUid( const char* s )
      {
         setString( DCM_SOPInstanceUID, s );
      }

      void setSopClassUid( const char* s )
      {
         setString( DCM_SOPClassUID, s );
      }

      const char* getSopClassUid()
      {
         return getString( DCM_SOPClassUID );
      }

      const char* getSeriesDate()
      {
         return getString( DCM_SeriesDate );
      }

      void setSeriesDate( const char* s )
      {
         setString( DCM_SeriesDate, s );
      }

      const char* getSeriesTime()
      {
         return getString( DCM_SeriesTime );
      }

      void setSeriesTime( const char* s )
      {
         setString( DCM_SeriesTime, s );
      }

      const char* getSeriesDescription()
      {
         return getString( DCM_SeriesDescription );
      }

      void setSeriesDescription( const char* s )
      {
         setString( DCM_SeriesDescription, s );
      }

      const char* getAcquisitionDate()
      {
         return getString( DCM_AcquisitionDate );
      }

      void setAcquisitionDate( const char* s )
      {
         setString( DCM_AcquisitionDate, s );
      }

      const char* getAcquisitionTime()
      {
         return getString( DCM_AcquisitionTime );
      }

      void setAcquisitionTime( const char* s )
      {
         setString( DCM_AcquisitionTime, s );
      }

      const char* getModality()
      {
         return getString( DCM_Modality );
      }

      void setModality( const char* s )
      {
         setString( DCM_Modality, s );
      }

      float getPatientWeight()
      {
         return nll::core::str2val<float>( getString( DCM_PatientWeight ) );
      }

      void setPatientWeight( float s )
      {
         setFloat( DCM_PatientWeight, s );
      }

      int getInstanceNumber()
      {
         return nll::core::str2val<int>( getString( DCM_InstanceNumber ) );
      }

      void setInstanceNumber( int s )
      {
         setInt( DCM_InstanceNumber, s );
      }

      void getPixelSpacing( nll::core::vector2f& spacing )
      {
         std::vector<std::string> strs;
         getStrings( DCM_PixelSpacing, strs );
         if ( strs.size() != 2 )
         {
            error( "expecting VM 2!" );
            spacing = nll::core::vector2f( -1, -1 );
         } else {
            spacing = nll::core::vector2f( nll::core::str2val<f32>( strs[ 1 ] ),
                                           nll::core::str2val<f32>( strs[ 0 ] ) );  // the spacing is defined (row, column)
         }
      }

      void setPixelSpacing( const nll::core::vector2f& spacing )
      {
         setVector2f( DCM_PixelSpacing, spacing );
      }

      void getImageOrientationPatient( nll::core::vector3f& x, nll::core::vector3f& y )
      {
         std::vector<std::string> strs;
         getStrings( DCM_ImageOrientationPatient, strs );
         if ( strs.size() != 6 )
         {
            error( "expecting VM 6!" );
            x = nll::core::vector3f( -1, -1, -1 );
            y = nll::core::vector3f( -1, -1, -1 );
         } else {
            x = nll::core::vector3f( nll::core::str2val<f32>( strs[ 0 ] ),
                                     nll::core::str2val<f32>( strs[ 1 ] ),
                                     nll::core::str2val<f32>( strs[ 2 ] ) );
            y = nll::core::vector3f( nll::core::str2val<f32>( strs[ 3 ] ),
                                     nll::core::str2val<f32>( strs[ 4 ] ),
                                     nll::core::str2val<f32>( strs[ 5 ] ) );
         }
      }

      void setImageOrientationPatient( const nll::core::vector3f& x, const nll::core::vector3f& y )
      {
         setVector6f( DCM_ImageOrientationPatient, x, y );
      }

      void getImagePositionPatient( nll::core::vector3f& x )
      {
         std::vector<std::string> strs;
         getStrings( DCM_ImagePositionPatient, strs );
         if ( strs.size() != 3 )
         {
            error( "expecting VM 3!" );
            x = nll::core::vector3f( -1, -1, -1 );
         } else {
            x = nll::core::vector3f( nll::core::str2val<f32>( strs[ 0 ] ),
                                     nll::core::str2val<f32>( strs[ 1 ] ),
                                     nll::core::str2val<f32>( strs[ 2 ] ) );
         }
      }

      void setImagePositionPatient( const nll::core::vector3f& s )
      {
         setVector3f( DCM_ImagePositionPatient, s );
      }

      ui32 getRows()
      {
         return getUnsignedShort( DCM_Rows );
      }

      void setRows( int s )
      {
         setInt( DCM_Rows, s );
      }

      ui32 getColumns()
      {
         return getUnsignedShort( DCM_Columns );
      }

      void setColumns( int s )
      {
         setInt( DCM_Columns, s );
      }

      float getRescaleIntercept()
      {
         const char* str = getString( DCM_RescaleIntercept );
         if ( strlen( str ) == 0 )
            return 0;
         return nll::core::str2val<float>( str );
      }

      void setRescaleIntercept( float s )
      {
         setFloat( DCM_RescaleIntercept, s );
      }

      float getRescaleSlope()
      {
         const char* str = getString( DCM_RescaleSlope );
         if ( strlen( str ) == 0 )
            return 1;
         return nll::core::str2val<float>( str );
      }

      void setRescaleSlope( float s )
      {
         setFloat( DCM_RescaleSlope, s );
      }

      void getPixelData( ui16* allocatedOutput )
      {
         const ui16* array = getUint16Array( DCM_PixelData );
         memcpy( allocatedOutput, array, sizeof( ui16 ) * getRows() * getColumns() );
      }

      void setPixelData( const ui16* allocatedOutput )
      {
         setUint16Array( DCM_PixelData, allocatedOutput, getRows() * getColumns() );
      }

      void setBitsAllocated( ui16 bits )
      {
         setUnsignedShort( DCM_BitsAllocated, bits );
      }

      void setBitsStored( ui16 bits )
      {
         setUnsignedShort( DCM_BitsStored, bits );
      }

      void setHighBit( ui16 bits )
      {
         setUnsignedShort( DCM_HighBit, bits );
      }

      void setSamplesPerPixel( ui16 bits )
      {
         setUnsignedShort( DCM_SamplesPerPixel, bits );
      }



      void setPixelRepresentation( ui16 pixelRepresentation )
      {
         setUnsignedShort( DCM_PixelRepresentation, pixelRepresentation );
      }

      const char* getFrameOfReference()
      {
         return getString( DCM_FrameOfReferenceUID );
      }

      void setFrameOfReference( const char* s )
      {
         setString( DCM_FrameOfReferenceUID, s );
      }

   private:
      float convertPatientAge( const char* str )
      {
         // TODO: handle the string representation
         return 0;
      }

      ui16 getUnsignedShort( const DcmTagKey& key )
      {
         ui16 val;
         OFCondition cond = _dataset.findAndGetUint16( key, val );
         if ( cond.good() )
         {
         } else {
            error( "missing tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
         return val;
      }

      const ui16* getUint16Array( const DcmTagKey& key )
      {
         const ui16* val;
         OFCondition cond = _dataset.findAndGetUint16Array( key, val );
         if ( cond.good() )
         {
         } else {
            error( "missing tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
         return val;
      }

      void setUint16Array( const DcmTagKey& key, const ui16* data, ui32 size )
      {
         OFCondition cond = _dataset.putAndInsertUint16Array( key, data, size );
         if ( cond.good() )
         {
         } else {
            error( "missing tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
      }

      const char* getString( const DcmTagKey& key )
      {
         const char* str = 0;
         OFCondition cond = _dataset.findAndGetString( key, str );
         if ( cond.good() )
         {
         } else {
            error( "missing tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
         if ( !str )
            return "";
         return str;
      }

      void setString( const DcmTagKey& key, const char* v )
      {
         OFCondition cond = _dataset.putAndInsertString( key, v );
         if ( cond.bad() )
         {
            error( "can't set tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
      }

      void setFloat( const DcmTagKey& key, float v )
      {
         OFCondition cond = _dataset.putAndInsertString( key, nll::core::val2str( v ).c_str() );
         if ( cond.bad() )
         {
            error( "can't set tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
      }

      void setUnsignedShort( const DcmTagKey& key, ui16 v )
      {
         OFCondition cond = _dataset.putAndInsertUint16( key, v );
         if ( cond.good() )
         {
         } else {
            error( "can't set tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
      }

      void setVector3f( const DcmTagKey& key, const nll::core::vector3f& v )
      {
         std::stringstream ss;
         ss << v[ 0 ] << "\\" << v[ 1 ] << "\\" << v[ 2 ];
         
         OFCondition cond = _dataset.putAndInsertString( key, ss.str().c_str() );
         if ( cond.bad() )
         {
            error( "can't set tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
      }

      void setVector6f( const DcmTagKey& key, const nll::core::vector3f& v1, const nll::core::vector3f& v2 )
      {
         std::stringstream ss;
         ss << v1[ 0 ] << "\\" << v1[ 1 ] << "\\" << v1[ 2 ] << "\\"
            << v2[ 0 ] << "\\" << v2[ 1 ] << "\\" << v2[ 2 ];
         OFCondition cond = _dataset.putAndInsertString( key, ss.str().c_str() );
         if ( cond.bad() )
         {
            error( "can't set tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
      }

      void setVector2f( const DcmTagKey& key, const nll::core::vector2f& v )
      {
         std::stringstream ss;
         ss << v[ 0 ] << "\\" << v[ 1 ];
         
         OFCondition cond = _dataset.putAndInsertString( key, ss.str().c_str() );
         if ( cond.bad() )
         {
            error( "can't set tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
      }

      void setInt( const DcmTagKey& key, int v )
      {

         OFCondition cond = _dataset.putAndInsertString( key, nll::core::val2str( v ).c_str() );
         if ( cond.bad() )
         {
            error( "can't set tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
      }

      void setStrings( const DcmTagKey& key, const std::vector<std::string>& strings )
      {
         std::stringstream ss;
         for ( size_t n = 0; n < strings.size(); ++n )
         {
            ss << strings[ n ];
            if ( n + 1 < strings.size() )
            {
               ss << "\\";
            }
         }
         OFCondition cond = _dataset.putAndInsertString( key, ss.str().c_str() );
         if ( cond.bad() )
         {
            error( "can't set tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }
      }

      void getStrings( const DcmTagKey& key, std::vector<std::string>& strings )
      {
         DcmStack result;
         _dataset.search( key, result );
         if ( result.empty() || !result.top() )
         {
            error( "missing or empty tag (hex):" + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
         }

         const ui32 size = result.top()->getVM();
         strings = std::vector<std::string>( size );
         for ( ui32 n = 0; n < size; ++n )
         {
            OFString str;
            OFCondition cond = _dataset.findAndGetOFString( key, str, n );
            if ( !cond.good() )
            {
               error( "cannot access element (hex):" + nll::core::val2str( n ) + " of " + nll::core::val2strHex( key.getGroup() ) + " " + nll::core::val2strHex( key.getElement() ) );
            }
            strings[ n ] = str.c_str();
         }
      }

      void error( const std::string& msg )
      {
         if ( _throwIfMissingTag )
         {
            throw std::runtime_error( msg );
         }
      }

   private:
      // copy disabled
      DicomWrapper& operator=( const DicomWrapper& );
      DicomWrapper( const DicomWrapper& );

   private:
      bool        _throwIfMissingTag;
      DcmDataset& _dataset;
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
         clear(); // we need to erase previous DICOM volumes

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
               DicomWrapper wrapper( *dcm.getDataset(), true );
               try
               {
                  nll::core::vector3f x;
                  wrapper.getImageOrientationPatient( x, x );
               } catch (...)
               {
                  std::cout << "warning: slice removed, missing mandatory tag" << std::endl;
                  continue;
               }

               if ( std::string( wrapper.getModality() ) != "OT" )   // we don't handle unknown modality...
               {
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
               } else {
                  std::cout << "warning: slice removed, unknown modality" << std::endl;
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

         sortSeriesUids();

         std::cout << "nb series UID=" << _dicomBySeriesUid.size() << std::endl;
         std::cout << "nb study UID=" << _dicomByStudyUid.size() << std::endl;

         for ( ui32 n = 0; n < _dicomByStudyUid.size(); ++n )
         {
            std::cout << "study[" << n << "]" << std::endl;
            for ( ui32 nn = 0; nn < _dicomByStudyUid[ n ].size(); ++nn )
            {
               std::cout << "series[" << _dicomByStudyUid[ n ][ nn ] << "], numberOfSlices=" << _dicomBySeriesUid[ _dicomByStudyUid[ n ][ nn ] ].size() << std::endl;
            }
         }
      }

      void clear()
      {
         _dicomBySeriesUid.clear();
         _dicomByStudyUid.clear();
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

      /**
       @brief Construct a volume from a Series index (in <_dicomBySeriesUid>)
       @note this is not handling missing slices!!!! It will only display a warning if this happens!
             it is assuming the infos are consistent within a seriesUids
       */
      template <class T>
      nll::imaging::VolumeSpatial<T>* constructVolumeFromSeries( ui32 seriesIndex, T backgroundValue = 0 )
      {
         nll::core::Timer timerConstruction;

         typedef nll::imaging::VolumeSpatial<T> Volume;
         DicomFiles& suids = _dicomBySeriesUid[ seriesIndex ];

         // consider the first slice, all the other slices must be consistent
         DicomWrapper wrapper( *suids[ 0 ].getDataset(), true );
         const ui32 nbSlices = static_cast<ui32>( suids.size() );

         // get the size
         nll::core::vector3ui size( wrapper.getColumns(),
                                    wrapper.getRows(),
                                    nbSlices );

         // construct the PST
         nll::core::vector2f pixelSpacing;
         wrapper.getPixelSpacing( pixelSpacing );

         nll::core::vector3f x, y, normal;
         wrapper.getImageOrientationPatient( x, y );
         normal = nll::core::cross( x, y );

         //  find the slice spacing: go through the slices, project the image position on the normal
         //  to determine the spacing and take the one that is consistent on 10 measures
         std::map<float, ui32> robustSpacing;
         for ( ui32 n = 0; n < std::min<ui32>( 20, nbSlices - 1 ); ++n )
         {
            DicomWrapper wrapper1( *suids[ n ].getDataset(), true );
            DicomWrapper wrapper2( *suids[ n + 1 ].getDataset(), true );

            nll::core::vector3f pos1, pos2;
            wrapper1.getImagePositionPatient( pos1 );
            wrapper2.getImagePositionPatient( pos2 );
            const float spacing = static_cast<float>( fabs( pos1.dot( normal ) - pos2.dot( normal ) ) );
            ++robustSpacing[ spacing ];
         }

         ui32 countMax = 0;
         float zspacing = 0;
         if ( nbSlices == 1 )
         {
            // we have only one slice, we can't compute the spacing! any value is fine...
            zspacing = 1;
         }
         for ( std::map<float, ui32>::const_iterator it = robustSpacing.begin(); it != robustSpacing.end(); ++it )
         {
            if ( it->second > countMax )
            {
               countMax = it->second;
               zspacing = it->first;
            }
         }
         if ( zspacing <= 0 )
            throw std::runtime_error( "spacing cannot be <= 0" );

         // compute the volume origin
         DicomWrapper wrapperLastSlice( *suids[ suids.size() - 1 ].getDataset(), true );
         nll::core::vector3f origin;
         wrapperLastSlice.getImagePositionPatient( origin );

         nll::core::Matrix<f32> pst( 4, 4 );
         for ( ui32 n = 0; n < 3; ++n )
         {
            pst( n, 0 ) = x[ n ]      * pixelSpacing[ 0 ];
            pst( n, 1 ) = y[ n ]      * pixelSpacing[ 1 ];
            pst( n, 2 ) = normal[ n ] * zspacing;
         }

         // TODO: check which corner should be the origin?
#ifdef ORIENTATION_FOR_MATLAB_COMPABILITY
         pst( 0, 3 ) = origin[ 0 ];
         pst( 1, 3 ) = -origin[ 1 ];
         pst( 2, 3 ) = -origin[ 2 ];
#else
         pst( 0, 3 ) = -origin[ 0 ];
         pst( 1, 3 ) = -origin[ 1 ];
         pst( 2, 3 ) = origin[ 2 ];
#endif
         pst( 3, 3 ) = 1;

         // fill the DICOM header in case we need it later for some algorithms...
         std::auto_ptr<nll::core::Context> context( new nll::core::Context() );
         DicomAttributs dicomHeader = createDicomAttributs( *suids[ 0 ].getDataset() );
         context->add( new ContextInstanceDicomInfo( dicomHeader ) );

         // fill the slices
         Volume* volume = new Volume( size, pst, backgroundValue, false, context );
         std::auto_ptr<ui16> ptr( new ui16[ size[ 0 ] * size[ 1 ] ] );
         for ( ui32 z = 0; z < size[ 2 ]; ++z )
         {
            DicomWrapper wrapper( *suids[ z ].getDataset(), false );
            wrapper.getPixelData( ptr.get() ); // we really don't want this to be in the multithreaded loop as it is reading slices from disk
            float slope = 1.0f;
            float intercept = 0.0f;

            try
            {
               slope = wrapper.getRescaleSlope();
               intercept = wrapper.getRescaleIntercept();
            } catch (...)
            {
               // missing RSI tag, but that's ok (e.g., MR data)
               //std::cout << "warning: RSI tag is missing!" << std::endl;
            }

            #pragma omp parallel for
            for ( int y = 0; y < (int)size[ 1 ]; ++y )
            {
               typename Volume::DirectionalIterator iter = volume->getIterator( 0, y, z );
               for ( ui32 x = 0; x < size[ 0 ]; ++x )
               {
                  const float val = ( (float)ptr.get()[ x + y * size[ 0 ] ] ) * slope + intercept;
                  *iter = val;
                  ++iter;
               }
            }
         }

         std::cout << "volume construction time=" << timerConstruction.getCurrentTime() << std::endl;

         // create the volume
         return volume;
      }

   private:
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
         for ( size_t n = 0; n < _dicomBySeriesUid.size(); ++n )
         {
            if ( _dicomBySeriesUid[ n ].size() == 1 )
               continue;   // no need to sort!
            nll::core::vector3f refx, refy, normal;
            std::vector< std::pair< double, size_t > > list;
            const size_t nbSlices = _dicomBySeriesUid[ n ].size();
            bool skipped = false;
            for ( size_t slice = 0; slice < nbSlices; ++slice )
            {
               DcmDataset& dataset = *_dicomBySeriesUid[ n ][ slice ].getDataset();
               DicomWrapper wrapper( dataset, true );

               nll::core::vector3f x, y, pos;
               wrapper.getImageOrientationPatient( x, y );
               wrapper.getImagePositionPatient( pos );

               // check the slices have the same orientation
               if ( slice == 0 )
               {
                  refx = x;
                  refy = y;
                  normal = nll::core::cross( x, y );
               } else {
                  for ( ui32 n = 0; n < 3; ++n )
                  {
                     if ( !nll::core::equal<f32>( refx[ n ], x[ n ], 1e-5f ) ||
                          !nll::core::equal<f32>( refy[ n ], y[ n ], 1e-5f ) )
                     {
                        std::stringstream ss;
                        ss << "Slices do not have the same orientation! Series skipped! ";
                        ss << "dx=" << refx[ n ] << " " << x[ n ];
                        ss << "dy=" << refy[ n ] << " " << y[ n ];
                        ss << "nbSlices=" << nbSlices;
                        std::cout << ss.str() << std::endl;
                        //throw std::runtime_error( ss.str() );
                        skipped = true;
                        break;
                     }
                  }
               }

               if ( skipped )
                  break;

               // finally assign a distance to this slice
               const double d = normal.dot( pos );
               list.push_back( std::make_pair( d, slice ) );
            }

            if ( skipped )
            {
               _dicomBySeriesUid[ n ] = DicomFiles(); // the volume is not valid for some reason
               continue;
            }

            std::sort( list.begin(), list.end() );
            DicomFiles sortedFiles; //( list.size() );
            for ( size_t t = 0; t < list.size(); ++t )
            {
               sortedFiles.push_back( _dicomBySeriesUid[ n ][ list[ t ].second ] );
            }
            _dicomBySeriesUid[ n ] = DicomFiles(); // use this to prevent a memleak in dcmtk
            _dicomBySeriesUid[ n ] = sortedFiles;
         }
      }

   private:
      std::vector< DicomFiles >             _dicomBySeriesUid;      // each entry represents a SeriesInstanceUid
      std::vector< std::vector< size_t > >  _dicomByStudyUid;       // [ x ][ y ], [ x ] is a set of index representing the set of SeriesUid belonging to the same Study Uid
   };

   
}

#endif