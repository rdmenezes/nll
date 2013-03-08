#ifndef MVV_DICOM_TOOLS_DICOM_ATTRIBUTS_H_
# define MVV_DICOM_TOOLS_DICOM_ATTRIBUTS_H_

# include <mvvPlatform/context-tools.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include <dcmtk/dcmimgle/dcmimage.h>
# include "utils.h"
# include <dicom/dicom-wrapper.h>

using namespace mvv::platform;
using namespace mvv::parser;

//
// Note: this module relies on the perfect synchronization of the <mvvDicomTools.ludo> header,
// and the <DicomAttributs> class. The DICOM tags will be extracted and persisted in the volume via <DicomAttributs>
// this may then exported to a <RuntimeValue> to be usable in the environment.
//
// 3 parts must be synchronized:
//   - <DicomAttributs>
//   - <DicomAttributs> in <mvvDicomTools.ludo>
//   - <DicomAttributs::isTagHandled> that registers all the tag used
//
namespace mvv
{
   extern ui32 dicomVolumeId;

   /**
    @brief The most common attributs that are extracted from the DICOM files

    This is done as a convenience utility that helps creating/modifying DICOM files

    @note this must perfectly reflect the DicomAttributs defined in <mvvDicomTools.ludo>
    */
   class DicomAttributs
   {
   public:
      std::string patientName;        // 1 - [ 0010-0010 ] Patient name
      std::string patientID;          // 2 - [ 0010-4000 ] Patient ID
      std::string patientSex;         // 3 - [ 0010-0040 ] Patient Sex
      float patientAge;          // 4 - [ 0010-1010 ] Patient Age
      float patientWeight;       // 5 - [ 0010-1030 ] Patient Weight
      std::string studyDate;          // 6 - [ 0008-0020 ] Study date
      std::string studyTime;          // 7 - [ 0008-0030 ] Study Time
      std::string studyDescription;   // 8 - [ 0008-1030 ] Study description
      std::string studyID;            // 9 - [ 0020-0010 ] Study ID
      std::string seriesDate;         // 10 - [ 0008-0021 ] Series Date
      std::string seriesTime;         // 11 - [ 0008-0031 ] Series Time
      std::string seriesDescription;  // 12 - [ 0008-103E ] Series Description
      std::string acquisitionDate;    // 13 - [ 0008-0022 ] Acquisition Date
      std::string acquisitionTime;    // 14 - [ 0008-0032 ] Acquisition Time
      std::string modality;           // 15 - [ 0008-0070 ] Modality
      std::string studyInstanceUid;   // 16 - [ 0020-000D ] Study Instance UID
      std::string seriesInstanceUid;  // 17 - [ 0020-000E ] Series Instance UID
      nll::core::vector3f imagePositionPatient;   // 18 - [ 0020-0032 ] Image Position Patient
      int rows;                  // 19 - [ 0028-0010 ] Rows
      int columns;               // 20 - [ 0028-0011 ] Columns
      nll::core::vector2f pixelSpacing;  // 21 - [ 0028-0030 ] pixel spacing
      int instanceNumber;        // 22 - [ 0020-0013 ] Instance number
      float slope;               // 23 - [ 0028-1053] Rescale slope
      float intercept;           // 24 - [ 0028-1053] Rescale intercept
      std::string frameOfReference;   // 25 - [ 0020-0052 ] Frame of Reference UID
      float extraSliceSpacing;   // 26 - Not a DICOM tag, extra info to write a DICOM volume // NOT USED
      nll::core::vector3f imageOrientationPatientX;  // 27 - [ 0020-0037 ] Image Orientation Patient, axis X
      nll::core::vector3f imageOrientationPatientY;  // 28 - [ 0020-0037 ] Image Orientation Patient, axis Y
      std::string sopInstanceUid;     // 29 - [ 0008, 0018 ] SOP Instance UID
      std::string sopClassUid;        // 30 - [ 0008, 0016 ] SOP Class UID

      std::string photometricInterpretation; // 31 [0028, 0004] Photometric Interpretation
      std::string acquisitionNumber;         // 32 [0020, 0012] Acquisition Number
      std::string kvp;                       // 33 [0018, 0060] KVP
      std::vector<std::string> imageType;    // 34 [0008, 0008] Image Type
      std::string sliceThickness;            // 35 [0018, 0050] Slice Thickness
      std::string manufacturer;              // 36 [0008, 0070] Manufacturer
      std::string positionReferenceIndicator;// 37 [0020, 1040] Position Reference Indicator
      std::string seriesNumber;              // 38 [0020, 0011] Series Number
      std::string patientPosition;           // 39 [0018, 5100] Patient Position
      std::string referringPhysician;        // 40 [0008, 0090] Referring Physician's Name
      std::string accessionNumber;           // 41 [0008, 0050] Accession Number
      std::string patientsBirthDate;         // 42 [0012, 0030] Patient's Birth Date
      std::string rescaleType;               // 43 [0028, 1054] RescaleType

      /**
       @brief Returns the index of the tag <group, element> defined in <mvvDicomTools.ludo>, -1 if it is not handled
       */
      static int isTagHandled( unsigned group, unsigned element, ui32* nbTags_out )
      {
         // The structure must have EXACTLY the same size as the number of fields in <DicomAttributs>
         // as it is used in part of the code to count the number of fileds
         static unsigned tags[][2] =
         {
            { 0x10, 0x0010 }, // 1
            { 0x10, 0x4000 },
            { 0x10, 0x0040 },
            { 0x10, 0x1010 },
            { 0x10, 0x1030 },
            { 0x08, 0x0020 }, // 5
            { 0x08, 0x0030 },
            { 0x08, 0x1030 },
            { 0x20, 0x0010 },
            { 0x08, 0x0021 },
            { 0x08, 0x0031 }, // 10
            { 0x08, 0x103E },
            { 0x08, 0x0022 },
            { 0x08, 0x0032 },
            { 0x08, 0x0070 },
            { 0x20, 0x000D }, // 15
            { 0x20, 0x000E },
            { 0x20, 0x0032 },
            { 0x28, 0x0010 },
            { 0x28, 0x0011 },
            { 0x28, 0x0030 }, // 20
            { 0x20, 0x0013 }, 
            { 0x28, 0x1053 },
            { 0x28, 0x1052 },
            { 0x20, 0x0052 }, // 25 // here we have removed the non DICOM tag, so will be index - 1
            { 0x20, 0x0037 },
            { 0x20, 0x0037 },
            { 0x08, 0x0018 },
            { 0x08, 0x0016 },

            { 0x0028, 0x0004 }, // 30
            { 0x0020, 0x0012 },
            { 0x0018, 0x0060 },
            { 0x0008, 0x0008 },
            { 0x0018, 0x0050 },
            { 0x0008, 0x0070 }, // 35
            { 0x0020, 0x1040 },
            { 0x0020, 0x0011 },
            { 0x0018, 0x5100 },
            { 0x0008, 0x0090 },
            { 0x0008, 0x0050 }, // 40
            { 0x0012, 0x0030 },
            { 0x0028, 0x1054 },
            { 0, 0 }                   // END TAG
         };

         static ui32 nbTags = nll::core::getStaticBufferSize( tags );

         if ( nbTags_out )
         {
            *nbTags_out = nbTags;
         }

         for ( ui32 n = 0; n < nbTags; ++n )
         {
            if ( tags[ n ][ 0 ] == group && tags[ n ][ 1 ] == element )
            {
               return n;
            }
         }
         return -1;
      }

      static ui32 getNumberOfHandledTags()
      {
         ui32 nb = 0;
         isTagHandled( 0, 0, &nb );
         return nb;
      }

      // this must follow exaclty the order in which the members are defined in <mvvDicomTools.ludo>
      static void exportTagsToRuntime( RuntimeValue& val, const DicomAttributs& src )
      {
         //std::cout << "nbTagsCreated=" << DicomAttributs::getNumberOfHandledTags() << std::endl;
         const ui32 nbTags = DicomAttributs::getNumberOfHandledTags();
         val.vals = RuntimeValue::RefcountedValues( 0, 0, new RuntimeValues( nbTags ) );
         val.type = RuntimeValue::TYPE;
         
         (*val.vals)[ 0 ].type = RuntimeValue::STRING;
         (*val.vals)[ 0 ].stringval = src.patientName;

         (*val.vals)[ 1 ].type = RuntimeValue::STRING;
         (*val.vals)[ 1 ].stringval = src.patientID;

         (*val.vals)[ 2 ].type = RuntimeValue::STRING;
         (*val.vals)[ 2 ].stringval = src.patientSex;

         (*val.vals)[ 3 ].type = RuntimeValue::CMP_FLOAT;
         (*val.vals)[ 3 ].floatval = src.patientAge;

         (*val.vals)[ 4 ].type = RuntimeValue::CMP_FLOAT;
         (*val.vals)[ 4 ].floatval = src.patientWeight;

         (*val.vals)[ 5 ].type = RuntimeValue::STRING;
         (*val.vals)[ 5 ].stringval = src.studyDate;

         (*val.vals)[ 6 ].type = RuntimeValue::STRING;
         (*val.vals)[ 6 ].stringval = src.studyTime;

         (*val.vals)[ 7 ].type = RuntimeValue::STRING;
         (*val.vals)[ 7 ].stringval = src.studyDescription;

         (*val.vals)[ 8 ].type = RuntimeValue::STRING;
         (*val.vals)[ 8 ].stringval = src.studyID;

         (*val.vals)[ 9 ].type = RuntimeValue::STRING;
         (*val.vals)[ 9 ].stringval = src.seriesDate;

         (*val.vals)[ 10 ].type = RuntimeValue::STRING;
         (*val.vals)[ 10 ].stringval = src.seriesTime;

         (*val.vals)[ 11 ].type = RuntimeValue::STRING;
         (*val.vals)[ 11 ].stringval = src.seriesDescription;

         (*val.vals)[ 12 ].type = RuntimeValue::STRING;
         (*val.vals)[ 12 ].stringval = src.acquisitionDate;

         (*val.vals)[ 13 ].type = RuntimeValue::STRING;
         (*val.vals)[ 13 ].stringval = src.acquisitionTime;

         (*val.vals)[ 14 ].type = RuntimeValue::STRING;
         (*val.vals)[ 14 ].stringval = src.modality;

         (*val.vals)[ 15 ].type = RuntimeValue::STRING;
         (*val.vals)[ 15 ].stringval = src.studyInstanceUid;

         (*val.vals)[ 16 ].type = RuntimeValue::STRING;
         (*val.vals)[ 16 ].stringval = src.seriesInstanceUid;

         mvv::parser::createVector3f( (*val.vals)[ 17 ], src.imagePositionPatient[ 0 ], 
                                                         src.imagePositionPatient[ 1 ],
                                                         src.imagePositionPatient[ 2 ] );

         (*val.vals)[ 18 ].type = RuntimeValue::CMP_INT;
         (*val.vals)[ 18 ].intval = src.rows;

         (*val.vals)[ 19 ].type = RuntimeValue::CMP_INT;
         (*val.vals)[ 19 ].intval = src.columns;

         mvv::parser::createVector2f( (*val.vals)[ 20 ], src.pixelSpacing[ 0 ], 
                                                         src.pixelSpacing[ 1 ] );

         (*val.vals)[ 21 ].type = RuntimeValue::CMP_INT;
         (*val.vals)[ 21 ].intval = src.instanceNumber;

         (*val.vals)[ 22 ].type = RuntimeValue::CMP_FLOAT;
         (*val.vals)[ 22 ].floatval = src.slope;

         (*val.vals)[ 23 ].type = RuntimeValue::CMP_FLOAT;
         (*val.vals)[ 23 ].floatval = src.intercept;

         (*val.vals)[ 24 ].type = RuntimeValue::STRING;
         (*val.vals)[ 24 ].stringval = src.frameOfReference;

         (*val.vals)[ 25 ].type = RuntimeValue::CMP_FLOAT;
         (*val.vals)[ 25 ].floatval = src.extraSliceSpacing;

         mvv::parser::createVector3f( (*val.vals)[ 26 ], src.imageOrientationPatientX[ 0 ], 
                                                         src.imageOrientationPatientX[ 1 ],
                                                         src.imageOrientationPatientX[ 2 ] );

         mvv::parser::createVector3f( (*val.vals)[ 27 ], src.imageOrientationPatientY[ 0 ], 
                                                         src.imageOrientationPatientY[ 1 ],
                                                         src.imageOrientationPatientY[ 2 ] );

         (*val.vals)[ 28 ].type = RuntimeValue::STRING;
         (*val.vals)[ 28 ].stringval = src.sopInstanceUid;

         (*val.vals)[ 29 ].type = RuntimeValue::STRING;
         (*val.vals)[ 29 ].stringval = src.sopClassUid;

         (*val.vals)[ 30 ].type = RuntimeValue::STRING;
         (*val.vals)[ 30 ].stringval = src.photometricInterpretation;

         (*val.vals)[ 31 ].type = RuntimeValue::STRING;
         (*val.vals)[ 31 ].stringval = src.acquisitionNumber;

         (*val.vals)[ 32 ].type = RuntimeValue::STRING;
         (*val.vals)[ 32 ].stringval = src.kvp;

         (*val.vals)[ 33 ].type = RuntimeValue::TYPE;
         if ( src.imageType.size() )
         {
            RuntimeValues* vals = new RuntimeValues( src.imageType.size() );
            for ( size_t n = 0; n < src.imageType.size(); ++n )
            {
               (*vals)[ n ].type = RuntimeValue::STRING;
               (*vals)[ n ].stringval = src.imageType[ n ];
            }
            (*val.vals)[ 33 ].vals = RuntimeValue::RefcountedValues( 0, 0, vals, true );
         }

         (*val.vals)[ 34 ].type = RuntimeValue::STRING;
         (*val.vals)[ 34 ].stringval = src.sliceThickness;

         (*val.vals)[ 35 ].type = RuntimeValue::STRING;
         (*val.vals)[ 35 ].stringval = src.manufacturer;

         (*val.vals)[ 36 ].type = RuntimeValue::STRING;
         (*val.vals)[ 36 ].stringval = src.positionReferenceIndicator;

         (*val.vals)[ 37 ].type = RuntimeValue::STRING;
         (*val.vals)[ 37 ].stringval = src.seriesNumber;

         (*val.vals)[ 38 ].type = RuntimeValue::STRING;
         (*val.vals)[ 38 ].stringval = src.patientPosition;

         (*val.vals)[ 39 ].type = RuntimeValue::STRING;
         (*val.vals)[ 39 ].stringval = src.referringPhysician;

         (*val.vals)[ 40 ].type = RuntimeValue::STRING;
         (*val.vals)[ 40 ].stringval = src.accessionNumber;

         (*val.vals)[ 41 ].type = RuntimeValue::STRING;
         (*val.vals)[ 41 ].stringval = src.patientsBirthDate;

         (*val.vals)[ 42 ].type = RuntimeValue::STRING;
         (*val.vals)[ 42 ].stringval = src.rescaleType;
      }

      static void exportTagsToDataset( const DicomAttributs& val, DcmDataset& out )
      {
         nll::dicom::DicomWrapperDcmtk wrapper( out );
         wrapper.setPatientName( val.patientName.c_str() );  
         wrapper.setPatientId( val.patientID.c_str() );
         wrapper.setPatientSex( val.patientSex.c_str() );
         wrapper.setPatientAge( val.patientAge );
         wrapper.setPatientWeight( val.patientWeight );
         wrapper.setStudyDate( val.studyDate.c_str() );
         wrapper.setStudyTime( val.studyTime.c_str() );
         wrapper.setStudyDescription( val.studyDescription.c_str() );
         wrapper.setStudyId( val.studyID.c_str() );
         wrapper.setSeriesDate( val.seriesDate.c_str() );
         wrapper.setSeriesTime( val.seriesTime.c_str() );
         wrapper.setSeriesDescription( val.seriesDescription.c_str() );
         wrapper.setAcquisitionDate( val.acquisitionDate.c_str() );
         wrapper.setAcquisitionTime( val.acquisitionTime.c_str() );
         wrapper.setModality( val.modality.c_str() );
         wrapper.setStudyInstanceUid( val.studyInstanceUid.c_str() );
         wrapper.setSeriesInstanceUid( val.seriesInstanceUid.c_str() );
         wrapper.setImagePositionPatient( val.imagePositionPatient );
         wrapper.setRows( val.rows );
         wrapper.setColumns( val.columns );
         wrapper.setPixelSpacing( val.pixelSpacing );
         wrapper.setInstanceNumber( val.instanceNumber );
         wrapper.setRescaleSlope( val.slope );
         wrapper.setRescaleIntercept( val.intercept );
         wrapper.setFrameOfReference( val.frameOfReference.c_str() );
         wrapper.setImageOrientationPatient( val.imageOrientationPatientX, val.imageOrientationPatientY );
         wrapper.setSopInstanceUid( val.sopInstanceUid.c_str() );
         wrapper.setSopClassUid( val.sopClassUid.c_str() );
         wrapper.setPhotometricInterpretation( val.photometricInterpretation.c_str() );
         wrapper.setAcquisitionNumber( val.acquisitionNumber.c_str() );
         wrapper.setKvp( val.kvp.c_str() );
         wrapper.setImageType( val.imageType );
         wrapper.setSliceThickness( val.sliceThickness.c_str() );
         wrapper.setManufacturer( val.manufacturer.c_str() );
         wrapper.setPositionReferenceIndicator( val.positionReferenceIndicator.c_str() );
         wrapper.setSeriesNumber( val.seriesNumber.c_str() );
         wrapper.setPatientPosition( val.patientPosition.c_str() );
         wrapper.setReferringPhysician( val.referringPhysician.c_str() );
         wrapper.setAccessionNumber( val.accessionNumber.c_str() );
         wrapper.setPatientBirthDate( val.patientsBirthDate.c_str() );
         wrapper.setRescaleType( val.rescaleType.c_str() );
      }
   };

   /**
    @brief Create a <DicomAttributs> from a dcmtk dataset
    @note this must perfectly reflect the DicomAttributs defined in <mvvDicomTools.ludo>
    */
   inline DicomAttributs createDicomAttributs( const nll::dicom::DicomWrapperInterface& wrapper )
   {
      DicomAttributs attributs;
      attributs.patientName = std::string( wrapper.getPatientName() );
      attributs.patientID = std::string( wrapper.getPatientId() );
      attributs.patientSex = std::string( wrapper.getPatientSex() );
      attributs.patientAge = wrapper.getPatientAge();
      attributs.patientWeight = wrapper.getPatientWeight();
      attributs.studyDate = std::string( wrapper.getStudyDate() );
      attributs.studyTime = std::string( wrapper.getStudyTime() );
      attributs.studyDescription = std::string( wrapper.getStudyDescription() );
      attributs.studyID = std::string( wrapper.getStudyId() );
      attributs.seriesDate = std::string( wrapper.getSeriesDate() );
      attributs.seriesTime = std::string( wrapper.getSeriesTime() );
      attributs.seriesDescription = std::string( wrapper.getSeriesDescription() );
      attributs.acquisitionDate = std::string( wrapper.getAcquisitionDate() );
      attributs.acquisitionTime = std::string( wrapper.getAcquisitionTime() );
      attributs.modality = std::string( wrapper.getModality() );
      attributs.studyInstanceUid = std::string( wrapper.getStudyInstanceUid() );
      attributs.seriesInstanceUid = std::string( wrapper.getSeriesInstanceUid() );
      wrapper.getImagePositionPatient( attributs.imagePositionPatient );
      attributs.rows = wrapper.getRows();
      attributs.columns = wrapper.getColumns();
      wrapper.getPixelSpacing( attributs.pixelSpacing );
      attributs.instanceNumber = wrapper.getInstanceNumber();
      attributs.slope = wrapper.getRescaleSlope();
      attributs.intercept = wrapper.getRescaleIntercept();
      attributs.frameOfReference = wrapper.getFrameOfReference();
      attributs.extraSliceSpacing = -1;
      wrapper.getImageOrientationPatient( attributs.imageOrientationPatientX, attributs.imageOrientationPatientY );
      attributs.sopInstanceUid = wrapper.getSopInstanceUid();
      attributs.sopClassUid = wrapper.getSopClassUid();
      attributs.photometricInterpretation = wrapper.getPhotometricInterpretation();
      attributs.acquisitionNumber = wrapper.getAcquisitionNumber();
      attributs.kvp = wrapper.getKvp();
      attributs.imageType = wrapper.getImageType();
      attributs.sliceThickness = wrapper.getSliceThickness();
      attributs.manufacturer = wrapper.getManufacturer();
      attributs.positionReferenceIndicator = wrapper.getPositionReferenceIndicator();
      attributs.seriesNumber = wrapper.getSeriesNumber();
      attributs.patientPosition = wrapper.getPatientPosition();
      attributs.referringPhysician = wrapper.getReferringPhysician();
      attributs.accessionNumber = wrapper.getAccessionNumber();
      attributs.patientsBirthDate = wrapper.getPatientBirthDate();
      attributs.rescaleType = wrapper.getRescaleType();
      return attributs;
   }

   inline DicomAttributs createDicomAttributs( const RuntimeValue& val )
   {
      DicomAttributs attributs;
      attributs.patientName =       (*val.vals)[ 0 ].stringval;
      attributs.patientID =         (*val.vals)[ 1 ].stringval;
      attributs.patientSex =        (*val.vals)[ 2 ].stringval;
      attributs.patientAge =        (*val.vals)[ 3 ].floatval;
      attributs.patientWeight =     (*val.vals)[ 4 ].floatval;
      attributs.studyDate =         (*val.vals)[ 5 ].stringval;
      attributs.studyTime =         (*val.vals)[ 6 ].stringval;
      attributs.studyDescription =  (*val.vals)[ 7 ].stringval;
      attributs.studyID =           (*val.vals)[ 8 ].stringval;
      attributs.seriesDate =        (*val.vals)[ 9 ].stringval;
      attributs.seriesTime =        (*val.vals)[ 10 ].stringval;
      attributs.seriesDescription = (*val.vals)[ 11 ].stringval;
      attributs.acquisitionDate =   (*val.vals)[ 12 ].stringval;
      attributs.acquisitionTime =   (*val.vals)[ 13 ].stringval;
      attributs.modality =          (*val.vals)[ 14 ].stringval;
      attributs.studyInstanceUid =  (*val.vals)[ 15 ].stringval;
      attributs.seriesInstanceUid = (*val.vals)[ 16 ].stringval;
      getVector3fValues( (*val.vals)[ 17 ], attributs.imagePositionPatient );
      attributs.rows =              (*val.vals)[ 18 ].intval;
      attributs.columns =           (*val.vals)[ 19 ].intval;
      getVector2fValues(            (*val.vals)[ 20 ], attributs.pixelSpacing );
      attributs.instanceNumber =    (*val.vals)[ 21 ].intval;
      attributs.slope =             (*val.vals)[ 22 ].floatval;
      attributs.intercept =         (*val.vals)[ 23 ].floatval;;
      attributs.frameOfReference =  (*val.vals)[ 24 ].stringval;
      attributs.extraSliceSpacing = (*val.vals)[ 25 ].floatval;
      getVector3fValues(            (*val.vals)[ 26 ], attributs.imageOrientationPatientX );
      getVector3fValues(            (*val.vals)[ 27 ], attributs.imageOrientationPatientY );
      attributs.sopInstanceUid =    (*val.vals)[ 28 ].stringval;
      attributs.sopClassUid =       (*val.vals)[ 29 ].stringval;

      attributs.photometricInterpretation = (*val.vals)[ 30 ].stringval;
      attributs.acquisitionNumber = (*val.vals)[ 31 ].stringval;
      attributs.kvp = (*val.vals)[ 32 ].stringval;
      if ( !(*val.vals)[ 33 ].vals.isEmpty() && (*val.vals)[ 33 ].vals.getData().size() )
      {
         std::vector<std::string> strs;
         for ( size_t n = 0; n < (*val.vals)[ 33 ].vals.getData().size(); ++n )
         {
            strs.push_back( (*val.vals)[ 33 ].vals.getData()[ n ].stringval );
         }
         attributs.imageType = strs;
      }

      attributs.sliceThickness = (*val.vals)[ 34 ].stringval;
      attributs.manufacturer = (*val.vals)[ 35 ].stringval;
      attributs.positionReferenceIndicator = (*val.vals)[ 36 ].stringval;
      attributs.seriesNumber = (*val.vals)[ 37 ].stringval;
      attributs.patientPosition = (*val.vals)[ 38 ].stringval;
      attributs.referringPhysician = (*val.vals)[ 39 ].stringval;
      attributs.accessionNumber = (*val.vals)[ 40 ].stringval;
      attributs.patientsBirthDate = (*val.vals)[ 41 ].stringval;
      attributs.rescaleType = (*val.vals)[ 42 ].stringval;
      return attributs;
   }

   class ContextInstanceDicomInfo : public nll::core::ContextInstance
   {
   public:
      ContextInstanceDicomInfo( DicomAttributs a ) : attributs( a )
      {}

      DicomAttributs attributs;
   };
}

#endif

