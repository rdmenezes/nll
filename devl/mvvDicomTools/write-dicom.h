#ifndef MVV_DICOM_TOOLS_WRITE_DICOM
# define MVV_DICOM_TOOLS_WRITE_DICOM

# include "utils.h"
# include "dicom-attributs.h"

namespace mvv
{
   template <class T, class Buffer>
   void writeDicomVolume( const nll::imaging::VolumeSpatial<T, Buffer>& volume, const std::string& directory )
   {
      mvv::ContextInstanceDicomInfo* dicomTags = NULL;
      volume->getContext().get( dicomTags );
      ensure( dicomTags, "no ContextInstanceDicomInfo found in volume context." ); // TODO: create a minimal context to save the volume?

      writeDicomVolume( volume, directory, *dicomTags );
   }

   template <class T, class Buffer>
   void writeDicomVolume( const nll::imaging::VolumeSpatial<T, Buffer>& volume,
                          const std::string& directory,
                          const DicomAttributs& additionalAttributs )
   {
      typedef nll::imaging::VolumeSpatial<T, Buffer> Volume;

      const T minVal = *std::min_element( volume.begin(), volume.end() );
      const T maxVal = *std::max_element( volume.begin(), volume.end() );

      const double size = ( (ui32)1 << 16 ) - 1;
      const double slope = ( maxVal - minVal ) / size;
      const double intercept = minVal;

      std::shared_ptr<ui16> sliceBuf( new ui16[ volume.getSize()[ 0 ] * volume.getSize()[ 1 ] ] );

      // first fill up the DICOM info using the <additionalAttributs>
      DcmDataset dataset;
      DicomAttributs::exportTagsToDataset( additionalAttributs, dataset );
      DicomWrapper dicomWrapper( dataset );

      // then for each slice, fill up the correct info
      const core::vector3f dx( volume.getPst()( 0, 0 ) / volume.getSpacing()[ 0 ],
                               volume.getPst()( 1, 0 ) / volume.getSpacing()[ 0 ],
                               volume.getPst()( 2, 0 ) / volume.getSpacing()[ 0 ] );
      const core::vector3f dy( volume.getPst()( 0, 1 ) / volume.getSpacing()[ 1 ],
                               volume.getPst()( 1, 1 ) / volume.getSpacing()[ 1 ],
                               volume.getPst()( 2, 1 ) / volume.getSpacing()[ 1 ] );
      const core::vector3f dz( volume.getPst()( 0, 2 ),     // here we want the spacing!
                               volume.getPst()( 1, 2 ),
                               volume.getPst()( 2, 2 ) );
      for ( ui32 slice = 0; slice < volume.getSize()[ 2 ]; ++slice )
      {
         // write volume info
         core::vector3f position = volume.getOrigin() + dz * slice;
         dicomWrapper.setImagePositionPatient( position );
         dicomWrapper.setImageOrientationPatient( dx, dy );
         dicomWrapper.setPixelSpacing( core::vector2f( volume.getSpacing()[ 0 ], volume.getSpacing()[ 1 ] ) );
         dicomWrapper.setRescaleIntercept( intercept );
         dicomWrapper.setRescaleSlope( slope );
         dicomWrapper.setColumns( volume.getSize()[ 0 ] );
         dicomWrapper.setRows( volume.getSize()[ 1 ] );
         dicomWrapper.setPixelRepresentation( 0 ); // unsigned
         dicomWrapper.setBitsAllocated( 16 );
         dicomWrapper.setBitsStored( 16 );
         dicomWrapper.setHighBit( 15 );
         dicomWrapper.setSamplesPerPixel( 1 );

         for ( ui32 y = 0; y < volume.getSize()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < volume.getSize()[ 0 ]; ++x )
            {
               const double val = ( volume( x, y, slice ) - intercept ) / slope;
               assert( val >= 0 && val <= size );
               const ui16 vali = static_cast<ui16>( val );

               sliceBuf.get()[ x + y * volume.getSize()[ 0 ] ] = vali;
            }
         }
         dicomWrapper.setPixelData( sliceBuf.get() );

         std::stringstream ss;
         ss << directory;
         if ( *directory.rbegin() != '/' && *directory.rbegin() != '\\' )
         {
            ss << "/";
         }
         ss << dicomWrapper.getSeriesInstanceUid() << "-" << slice << ".dcm";
         OFCondition result = dataset.saveFile( ss.str().c_str(), EXS_LittleEndianImplicit );
         ensure( result.good(), "the DICOM file could not be created." );
      }
   }
}

#endif