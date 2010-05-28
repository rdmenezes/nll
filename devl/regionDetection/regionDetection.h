#ifndef REGIONDETECTION_H
#define REGIONDETECTION_H

#ifdef _MSC_VER
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the REGIONDETECTION_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// REGIONDETECTION_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef REGIONDETECTION_EXPORTS
#define REGIONDETECTION_API __declspec(dllexport)
#else
#define REGIONDETECTION_API __declspec(dllimport)
#pragma comment(lib, "regionDetection.lib")
#endif
#else
#define REGIONDETECTION_API
#endif

#endif