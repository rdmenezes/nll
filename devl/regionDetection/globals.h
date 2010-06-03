#ifndef NLL_UTILITY_GLOBAL_H_
# define NLL_UTILITY_GLOBAL_H_

// lut used to compute the barycentre (on bone)
#define REGION_DETECTION_BARYCENTRE_LUT_MIN  -240
#define REGION_DETECTION_BARYCENTRE_LUT_MAX  320

#define REGION_DETECTION_SOURCE_IMG_X  16
#define REGION_DETECTION_SOURCE_IMG_Y  32

#define REGION_DETECTION_PCA_SIZE 4

#define DATA_PATH "../../regionDetectionTest/data/"


#define PCA_ENGINE_PATH          DATA_PATH "pca-engine.dat"
#define NN_ENGINE_PATH           DATA_PATH "nn-engine.dat"
#define DATABASE_PCA             DATA_PATH "pca-data.dat"
#define DATABASE_SOURCE          DATA_PATH "source-data.dat"

#define PREVIEW_CASE             DATA_PATH "/preview/xz-case-"
#define PREVIEW_CASE_MARK        DATA_PATH "/preview-mark/xz-case-"

#define NBCASES                 62

#endif