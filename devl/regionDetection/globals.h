#ifndef NLL_UTILITY_GLOBAL_H_
# define NLL_UTILITY_GLOBAL_H_

// lut used to compute the barycentre (on bone)
#define REGION_DETECTION_BARYCENTRE_LUT_MIN  -240
#define REGION_DETECTION_BARYCENTRE_LUT_MAX  320

#define REGION_DETECTION_SOURCE_IMG_X  100
#define REGION_DETECTION_SOURCE_IMG_Y  ( REGION_DETECTION_SOURCE_IMG_X / 1.5 )

#define DATA_PATH "../../regionDetectionTest/data/"

#define PREVIEW_CASE             DATA_PATH "/preview/xz-case-"
#define PREVIEW_CASE_MARK        DATA_PATH "/preview-mark/xz-case-"

#define DATABASE_SOURCE          DATA_PATH "source.database"
#define CASES_DESC               DATA_PATH "cases.txt"

#define NBCASES                 62

#endif
