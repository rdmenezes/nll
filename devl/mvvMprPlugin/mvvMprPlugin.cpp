// mvvMprPlugin.cpp : Defines the entry point for the DLL.
//

#include "mvvMprPlugin.h"

#include "annotation.h"
#include "annotation-point.h"
#include "annotation-colors.h"
#include "engine-mpr.h"
#include "segment.h"
#include "layout-segment.h"
#include "context-segments.h"
#include "segment-tool-pointer.h"
#include "segment-tool-camera.h"
#include "segment-tool-annotations.h"
#include "segment-tool-autocenter.h"
#include "mip.h"
#include "mip-tool.h"
#include "mip-tool-pointer.h"
#include "layout-mip.h"
#include "annotation-line.h"
#include "annotation-line-pos.h"
#include "segment-tool-postprocessing.h"
#include "mip-tool-postprocessing.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

MVVMPRPLUGIN_API int dummy = 0;