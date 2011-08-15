#ifndef FORMS_H
#define FORMS_H

#ifdef _MSC_VER
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FORMS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FORMS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef FORMS_EXPORTS
#define FORMS_API __declspec(dllexport)
#else
#define FORMS_API __declspec(dllimport)
#pragma comment(lib, "forms.lib")
#endif
#else
#define FORMS_API
#endif

# include <string>
# include <vector>

/**
 @brief Defines the set of controls which is OS dependent...
 @note in the windows implementation, a CWinApp object is created!
 */
namespace mvv
{
   FORMS_API std::vector<unsigned> createMessageBoxTextSelection( const std::string& title, const std::vector<std::string>& texts );
   FORMS_API std::string createMessageBoxText( const std::string& title );
   FORMS_API void createMessageBoxError( const std::string& title, const std::string& msg );
   FORMS_API bool createMessageBoxQuestion( const std::string& title, const std::string& msg );
   FORMS_API std::vector<std::string> openFiles( const std::string& title );
   FORMS_API std::string openFolder( const std::string title );
   FORMS_API std::string getWorkingDirectory();
   FORMS_API void setWorkingDirectory( const std::string& s );
}

#endif