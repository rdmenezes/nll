#ifndef NLL_UTILITY_GLOBAL_H_
# define NLL_UTILITY_GLOBAL_H_

// lut used to compute the barycentre (on bone)
#define REGION_DETECTION_BARYCENTRE_LUT_MIN  -100
#define REGION_DETECTION_BARYCENTRE_LUT_MAX  300

#define REGION_DETECTION_SOURCE_IMG_X  25
#define REGION_DETECTION_SOURCE_IMG_Y  50

#define REGION_DETECTION_PCA_SIZE 128/8

#define DATA_PATH "../../regionDetectionTest/data/"


#define PCA_ENGINE_PATH          DATA_PATH "pca-engine.dat"
#define NN_ENGINE_PATH           DATA_PATH "nn-engine.dat"
#define DATABASE_PCA             DATA_PATH "pca-data.dat"
#define DATABASE_SOURCE          DATA_PATH "source-data.dat"

#define PREVIEW_CASE             DATA_PATH "/preview/xz-case-"
#define PREVIEW_CASE_MARK        DATA_PATH "/preview-mark/xz-case-"

#endif
