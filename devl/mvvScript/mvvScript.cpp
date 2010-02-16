// mvvScript.cpp : Defines the entry point for the DLL.
//

#include "mvvScript.h"
#include "error.h"
#include "ast-files.h"
#include "visitor-print.h"
#include "visitor-default.h"
#include "visitor-bind.h"
#include "symbol-table.h"


int MVVSCRIPT_API dummy = 0;

#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif
