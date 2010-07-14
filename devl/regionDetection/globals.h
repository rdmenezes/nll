#ifndef NLL_UTILITY_GLOBAL_H_
# define NLL_UTILITY_GLOBAL_H_

// lut used to compute the barycentre (on bone)
#define REGION_DETECTION_BARYCENTRE_LUT_MIN  -240
#define REGION_DETECTION_BARYCENTRE_LUT_MAX  320

#define REGION_DETECTION_SOURCE_IMG_X  66
#define REGION_DETECTION_SOURCE_IMG_Y  ( static_cast<ui32>( REGION_DETECTION_SOURCE_IMG_X / 1.5 ) )

#define DATABASE_MIN_INTERVAL_ROI   15
#define DATABASE_MIN_INTERVAL       10

#define DATA_PATH                "../../regionDetectionTest/data/"
#define REGISTRATION_DATA_PATH   "D:/Devel/RegionDetectionAlgorithm/data/"

#define PREVIEW_CASE             DATA_PATH "preview/xz-case-"
#define PREVIEW_CASE_MARK        DATA_PATH "preview-mark/xz-case-"
#define PREVIEW_CASE_REG         DATA_PATH "preview-registration/xz-case-"
#define PREVIEW_CASE_VALIDATION  DATA_PATH "preview-validation/xz-case-"
#define PREVIEW_CASE_CORRECTION  DATA_PATH "preview-correction/case"

#define DATABASE_SOURCE          DATA_PATH "source.database"
#define CASES_DESC               DATA_PATH "cases.txt"
#define VALIDATION_CASES_DESC    DATA_PATH "validation-cases.txt"
#define VALIDATION_OVERWEIGHT    DATA_PATH "validation-cases2.txt"
#define VALIDATION_COMAPRE       DATA_PATH "validation-cases3.txt"
#define HAAR_FEATURES            DATA_PATH "haar-features.bin"
#define DATABASE_HAAR            DATA_PATH "haar.database"
#define PREPROCESSING_HAAR       DATA_PATH "haar-preprocessing.bin"
#define NORMALIZED_HAAR          DATA_PATH "haar-normalized.database"
#define DATABASE_FULL_CASE(n)    ( DATA_PATH "rawmpr" + nll::core::val2str(n) + ".database" )

#define REGISTRATION_INPUT       REGISTRATION_DATA_PATH "results-register-choice.txt"

#define HAAR_FEATURE_SIZE        4096

#define FEATURE_SELECTION_SIZE   std::min( 1024, HAAR_FEATURE_SIZE ) //20*20
#define HAAR_SELECTION           DATA_PATH "haar-normalized-selected.bin"
#define HAAR_SELECTION_DATABASE  DATA_PATH "haar-normalized-selected.database"
#define DATABASE_MEASURES        DATA_PATH "measures.txt"
#define DATABASE_MEASURES_CORRECTION DATA_PATH "measures-correction.txt"

#define NB_CLASS                 6
#define FINAL_SVM_CLASSIFIER     DATA_PATH "svm.bin"

#define CORRECTION_MAX_PIVOT_TEMPLATE_ERROR  0.1      // the maximum error possible for the pivot points compared the the template points
//#define CORRECTION_DETECTION_RATE            0.2      // the maximum error possible for the distance ratio ROI template/test
#define CORRECTION_MIN_SIMILARITY            2.5      // the minimum threshold to detect a matching template

#define LEARNING_NB_BINS                     10       // the number of bins for crossvalidation

#define CORRECTION2_NB_MIN_DIST              5

#endif
