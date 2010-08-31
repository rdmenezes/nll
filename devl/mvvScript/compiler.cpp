#include "compiler.h"

/**
 @brief isolates the window header...
 */
namespace mvv
{
namespace parser
{
   typedef void (*functionImportDecl)( CompilerFrontEnd&, platform::Context& );

   void CompilerFrontEnd::importDll( const std::string& name )
   {
      std::string libpath = _findFileInPath( name + ".dll", _runtimePath );
      void* hMod = LoadLibraryWrapper( libpath );
      if ( !hMod )
      {
         throw RuntimeException( ( std::string( "LoadLibrary failed to load the library \"" ) + name + ".dll\"" ).c_str() );
      }

      functionImportDecl pfn = (functionImportDecl)GetProcAdressWrapper( hMod, "importFunctions" );
      if ( !pfn )
      {
         throw RuntimeException( "GetProcAdress failed to find the starting point in the dll" );
      }

      pfn( *this, *_contextExt );
   }
}
}