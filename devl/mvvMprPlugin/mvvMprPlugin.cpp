// mvvMprPlugin.cpp : Defines the entry point for the DLL.
//

#include "mvvMprPlugin.h"

#include "annotation.h"
#include "annotation-point.h"
#include "engine-mpr.h"
#include "segment.h"
#include "layout-segment.h"
#include "context-segments.h"
#include "segment-tool-pointer.h"
#include "segment-tool-camera.h"
#include "segment-tool-points.h"
#include "mip.h"
#include "layout-mip.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

MVVMPRPLUGIN_API int dummy = 0;