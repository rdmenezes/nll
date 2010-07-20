// regionDetection.cpp : Defines the entry point for the DLL.
//

#include "regionDetection.h"
#include "types.h"
#include "read-result.h"
#include "features.h"
#include "test.h"

REGIONDETECTION_API int dummy = 0;

#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif


nll::ui8 colors[ NB_CLASS ][ 3 ] = 
{
   { 255, 255, 255},
   { 0, 0, 255 },
   { 255, 0, 0 },
   { 0, 255, 0 },
   { 0, 255, 255 },
   { 255, 255, 0 }
};

const nll::ui8 colors_src[ NB_CLASS ][ 3 ] = 
{
   { 255, 255, 255},
   { 0, 0, 255 },
   { 255, 0, 0 },
   { 0, 255, 0 },
   { 0, 255, 255 },
   { 255, 255, 0 }
};