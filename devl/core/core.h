#ifndef CORE_H
#define CORE_H

#include <mvvScript/Compiler.h>

#ifdef _MSC_VER
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CORE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CORE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#pragma comment(lib, "core.lib")
#endif
#else
#define CORE_API
#endif

#endif

extern "C" _declspec(dllexport) void importFunctions( mvv::parser::CompilerFrontEnd&, mvv::platform::Context& );