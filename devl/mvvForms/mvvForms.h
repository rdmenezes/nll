#ifndef MVVFORMS_H
#define MVVFORMS_H

#include <mvvScript/Compiler.h>

#ifdef _MSC_VER
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MVVFORMS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MVVFORMS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MVVFORMS_EXPORTS
#define MVVFORMS_API __declspec(dllexport)
#else
#define MVVFORMS_API __declspec(dllimport)
#pragma comment(lib, "mvvForms.lib")
#endif
#else
#define MVVFORMS_API
#endif

# include <string>
# include <vector>

namespace mvv
{
   MVVFORMS_API void createMessageBoxError( const std::string& title, const std::string& msg );
   MVVFORMS_API bool createMessageBoxQuestion( const std::string& title, const std::string& msg );
   MVVFORMS_API std::vector<std::string> openFiles( const std::string& title );
   MVVFORMS_API std::string openFolder( const std::string title );
   MVVFORMS_API std::string getWorkingDirectory();
   MVVFORMS_API void setWorkingDirectory( const std::string& s );
}

extern "C" _declspec(dllexport) void importFunctions( mvv::parser::CompilerFrontEnd&, mvv::platform::Context& );

#endif