#ifndef MVVPLATFORM_H
#define MVVPLATFORM_H

#ifdef _MSC_VER
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MVVPLATFORM_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MVVPLATFORM_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MVVPLATFORM_EXPORTS
#define MVVPLATFORM_API __declspec(dllexport)
#else
#define MVVPLATFORM_API __declspec(dllimport)
#pragma comment(lib, "mvvPlatform.lib")
#endif
#else
#define MVVPLATFORM_API
#endif

#endif

# pragma warning( disable:4251 ) // we are assuming client & library are using exactly the same version of the stdlib
                                 // and same compilation options