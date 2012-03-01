#ifndef MVVDICOMTOOLS_H
#define MVVDICOMTOOLS_H

#include <mvvScript/Compiler.h>

#ifdef _MSC_VER

#pragma comment(lib, "dcmimgle.lib")
#pragma comment(lib, "dcmnet.lib")
#pragma comment(lib, "dcmdata.lib")
#pragma comment(lib, "oflog.lib")
#pragma comment(lib, "ofstd.lib")

// windows specific lib for DCMTK
#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "wsock32.lib")

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MVVDICOMTOOLS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MVVDICOMTOOLS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MVVDICOMTOOLS_EXPORTS
#define MVVDICOMTOOLS_API __declspec(dllexport)
#else
#define MVVDICOMTOOLS_API __declspec(dllimport)
#pragma comment(lib, "mvvDicomTools.lib")
#endif
#else
#define MVVDICOMTOOLS_API
#endif

#endif

extern "C" _declspec(dllexport) void importFunctions( mvv::parser::CompilerFrontEnd&, mvv::platform::Context& );