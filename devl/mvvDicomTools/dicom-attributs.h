#ifndef MVV_DICOM_TOOLS_DICOM_ATTRIBUTS_H_
# define MVV_DICOM_TOOLS_DICOM_ATTRIBUTS_H_

# include <mvvPlatform/context-tools.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include <dcmtk/dcmimage.h>
# include "utils.h"

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

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
      string patientName;        // 1 - [ 0010-0010 ] Patient name
      string patientID;          // 2 - [ 0010-4000 ] Patient ID
      string patientSex;         // 3 - [ 0010-0040 ] Patient Sex
      float patientAge;          // 4 - [ 0010-1010 ] Patient Age
      float patientWeight;       // 5 - [ 0010-1030 ] Patient Weight
      string studyDate;          // 6 - [ 0008-0020 ] Study date
      string studyTime;          // 7 - [ 0008-0030 ] Study Time
      string studyDescription;   // 8 - [ 0008-1030 ] Study description
      string studyID;            // 9 - [ 0020-0010 ] Study ID
      string seriesDate;         // 10 - [ 0008-0021 ] Series Date
      string seriesTime;         // 11 - [ 0008-0031 ] Series Time
      string seriesDescription;  // 12 - [ 0008-103E ] Series Description
      string acquisitionDate;    // 13 - [ 0008-0022 ] Acquisition Date
      string acquisitionTime;    // 14 - [ 0008-0032 ] Acquisition Time
      string modality;           // 15 - [ 0008-0070 ] Modality
      string studyInstanceUid;   // 16 - [ 0020-000D ] Study Instance UID
      string seriesInstanceUid;  // 17 - [ 0020-000E ] Series Instance UID
      nll::core::vector3f imagePositionPatient;   // 18 - [ 0020-0032 ] Image Position Patient
      int rows;                  // 19 - [ 0028-0010 ] Rows
      int columns;               // 20 - [ 0028-0011 ] Columns
      nll::core::vector2f pixelSpacing;  // 21 - [ 0028-0030 ] pixel spacing
      int instanceNumber;        // 22 - [ 0020-0013 ] Instance number
      float slope;               // 23 - [ 0028-1053] Rescale slope
      float intercept;           // 24 - [ 0028-1053] Rescale intercept
      string frameOfReference;   // 25 - [ 0020-0052 ] Frame of Reference UID
      float extraSliceSpacing;   // 26 - Not a DICOM tag, extra info to write a DICOM volume
      nll::core::vector3f imageOrientationPatientX;  // 27 - [ 0020-0037 ] Image Orientation Patient, axis X
      nll::core::vector3f imageOrientationPatientY;  // 28 - [ 0020-0037 ] Image Orientation Patient, axis Y
      string sopInstanceUid;     // 29 - [ 0008, 0018 ] SOP Instance UID

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
            { 0x20, 0x0052 }, // 25
            { 0, 0 },
            { 0x20, 0x0037 },
            { 0x20, 0x0037 },
            { 0x08, 0x0018 }
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
         val.vals = RuntimeValue::RefcountedValues( 0, 0, new RuntimeValues( DicomAttributs::getNumberOfHandledTags() ) );
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

         (*val.vals)[ 25 ].type = RuntimeValue::STRING;
         (*val.vals)[ 25 ].stringval = src.extraSliceSpacing;

         mvv::parser::createVector3f( (*val.vals)[ 26 ], src.imageOrientationPatientX[ 0 ], 
                                                         src.imageOrientationPatientX[ 1 ],
                                                         src.imageOrientationPatientX[ 2 ] );

         mvv::parser::createVector3f( (*val.vals)[ 27 ], src.imageOrientationPatientY[ 0 ], 
                                                         src.imageOrientationPatientY[ 1 ],
                                                         src.imageOrientationPatientY[ 2 ] );

         (*val.vals)[ 28 ].type = RuntimeValue::STRING;
         (*val.vals)[ 28 ].stringval = src.sopInstanceUid;
      }

      static void exportTagsToDataset( DicomAttributs& val, DcmDataset& out )
      {
         DicomWrapper wrapper( out );

         wrapper.setPatientName( val.patientName.c_str() );
      }
   };

   /**
    @brief Create a <DicomAttributs> from a dcmtk dataset
    @note this must perfectly reflect the DicomAttributs defined in <mvvDicomTools.ludo>
    */
   inline DicomAttributs createDicomAttributs( DcmDataset& dataset )
   {
      DicomWrapper wrapper( dataset, false );

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
      return attributs;
   }

   inline DicomAttributs createDicomAttributs( const RuntimeValue& val )
   {
      DicomAttributs attributs;
      attributs.patientName = (*val.vals)[ 0 ].stringval;
         //std::string( wrapper.getPatientName() );
      /*
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
      */
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

