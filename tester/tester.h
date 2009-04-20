#ifndef TESTER_H_
# define TESTER_H_

#ifdef _MSC_VER
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef TESTER_EXPORTS
#define TESTER_API __declspec(dllexport)
#else
#define TESTER_API __declspec(dllimport)
#pragma comment(lib, "tester.lib")
#endif
#else
#define TESTER_API
#endif

#endif
