#include "import.h"
#include <windows.h>

#include <vector>
#include <nll/types.h>
#include <nll/utility-pure.h>
#include <sstream>


namespace mvv
{
namespace parser
{
   void* LoadLibraryWrapper( const std::string& path, std::string& reason_out )
   {
      std::wstring conv = nll::core::stringTowstring( path );
      LPCWSTR pathw = conv.c_str();

      void* ptr = ::LoadLibrary( pathw );
      if ( ptr )
      {
         reason_out = "";
      } else {
         reason_out = "loadlibrary failed with exit code=" + nll::core::val2str( GetLastError() );
      }
      return ptr;
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
