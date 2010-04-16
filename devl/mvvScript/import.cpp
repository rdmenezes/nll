#include "import.h"
#include <windows.h>

#include <nll/utility-pure.h>


namespace mvv
{
namespace parser
{
   void* LoadLibraryWrapper( const std::string& path )
   {
      std::wstring conv = nll::core::stringTowstring( path );
      LPCWSTR pathw = conv.c_str();
      return ::LoadLibrary( pathw );
   }

   void* GetProcAdressWrapper( void* module, const std::string& name )
   {
      return ::GetProcAddress( (HMODULE)module, name.c_str() );
   }

   void FreeLibraryWrapper( void* module )
   {
      ::FreeLibrary( (HMODULE)module );
   }
}
}
