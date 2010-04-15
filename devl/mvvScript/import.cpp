#include "import.h"
#include <windows.h>

namespace mvv
{
namespace parser
{
   void* LoadLibraryWrapper( const std::string& path )
   {
      LPCWSTR pathw = nll::core::stringTowstring( path ).c_str();
      return ::LoadLibrary( pathw );
   }

   void* GetProcAdressWrapper( void* module, const std::string& name )
   {
      return ::GetProcAddress( (HMODULE)module, name.c_str() );
   }
}
}
