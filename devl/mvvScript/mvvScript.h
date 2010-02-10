#ifndef MVVSCRIPT_H
#define MVVSCRIPT_H

#ifdef _MSC_VER
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MVVSCRIPT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MVVSCRIPT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MVVSCRIPT_EXPORTS
#define MVVSCRIPT_API __declspec(dllexport)
#else
#define MVVSCRIPT_API __declspec(dllimport)
#pragma comment(lib, "mvvScript.lib")
#endif
#else
#define MVVSCRIPT_API
#endif

#endif