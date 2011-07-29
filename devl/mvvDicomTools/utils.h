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
         return getString( DCM_SeriesInstanceUID );
      }

      const char* getStudyInstanceUid()
      {
         return getString( DCM_StudyInstanceUID );
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
            spacing = nll::core::vector2f( nll::core::str2val<f32>( strs[ 0 ] ),
                                           nll::core::str2val<f32>( strs[ 1 ] ) );
         }
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

      void getImagePositionPatitient( nll::core::vector3f& x )
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

      ui32 getRows()
      {
         return getUnsignedShort( DCM_Rows );
      }

      ui32 getColumns()
      {
         return getUnsignedShort( DCM_Columns );
      }

   private:
      ui16 getUnsignedShort( const DcmTagKey& key )
      {
         ui16 val;
         OFCondition cond = _dataset.findAndGetUint16( key, val );
         if ( cond.good() )
         {
         } else {
            error( "missing tag:" + nll::core::val2str( key.getGroup() ) + " " + nll::core::val2str( key.getElement() ) );
         }
         return val;
      }

      const char* getString( const DcmTagKey& key )
      {
         const char* str = 0;
         OFCondition cond = _dataset.findAndGetString( key, str );
         if ( cond.good() )
         {
         } else {
            error( "missing tag:" + nll::core::val2str( key.getGroup() ) + " " + nll::core::val2str( key.getElement() ) );
         }
         return str;
      }

      void getStrings( const DcmTagKey& key, std::vector<std::string>& strings )
      {
         DcmStack result;
         _dataset.search( key, result );
         if ( result.empty() )
         {
            error( "missing tag:" + nll::core::val2str( key.getGroup() ) + " " + nll::core::val2str( key.getElement() ) );
         }
         const ui32 size = result.top()->getVM();
         strings = std::vector<std::string>( size );
         for ( ui32 n = 0; n < size; ++n )
         {
            OFString str;
            OFCondition cond = _dataset.findAndGetOFString( key, str, n );
            if ( !cond.good() )
            {
               error( "cannot access element:" + nll::core::val2str( n ) + " of " + nll::core::val2str( key.getGroup() ) + " " + nll::core::val2str( key.getElement() ) );
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

         sortSeriesUids();

         std::cout << "nb series UID=" << _dicomBySeriesUid.size() << std::endl;
         std::cout << "nb study UID=" << _dicomByStudyUid.size() << std::endl;
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
            wrapper1.getImagePositionPatitient( pos1 );
            wrapper2.getImagePositionPatitient( pos2 );
            const float spacing = static_cast<float>( fabs( pos1.dot( normal ) - pos2.dot( normal ) ) );
            ++robustSpacing[ spacing ];
         }

         ui32 countMax = 0;
         float zspacing = 0;
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
         wrapperLastSlice.getImagePositionPatitient( origin );

         nll::core::Matrix<f32> pst( 4, 4 );
         for ( ui32 n = 0; n < 3; ++n )
         {
            pst( n, 0 ) = x[ n ]      * pixelSpacing[ 0 ];
            pst( n, 1 ) = y[ n ]      * pixelSpacing[ 0 ];
            pst( n, 2 ) = normal[ n ] * zspacing;
         }
         pst( 0, 3 ) = origin[ 0 ];
         pst( 1, 3 ) = origin[ 1 ];
         pst( 2, 3 ) = origin[ 2 ];
         pst( 3, 3 ) = 1;

         // create the volume
         return new nll::imaging::VolumeSpatial<T>( size, pst, backgroundValue );
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
            nll::core::vector3f refx, refy, normal;
            std::vector< std::pair< double, size_t > > list;
            for ( size_t slice = 0; slice < _dicomBySeriesUid[ n ].size(); ++slice )
            {
               DcmDataset& dataset = *_dicomBySeriesUid[ n ][ slice ].getDataset();
               DicomWrapper wrapper( dataset, true );

               nll::core::vector3f x, y, pos;
               wrapper.getImageOrientationPatient( x, y );
               wrapper.getImagePositionPatitient( pos );

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
                        throw std::runtime_error( "Slices do not have the same orientation!" );
                     }
                  }
               }

               // finally assign a distance to this slice
               const double d = normal.dot( pos );
               list.push_back( std::make_pair( d, slice ) );
            }

            std::sort( list.rbegin(), list.rend() );
         }
      }

   private:
      std::vector< DicomFiles >             _dicomBySeriesUid;      // each entry represents a SeriesInstanceUid
      std::vector< std::vector< size_t > >  _dicomByStudyUid;       // [ x ][ y ], [ x ] is a set of index representing the set of SeriesUid belonging to the same Study Uid
   };
}

#endif