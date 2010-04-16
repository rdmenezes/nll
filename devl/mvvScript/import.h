#ifndef MVV_PARSER_IMPORT2_H_
# define MVV_PARSER_IMPORT2_H_

#include <string>

namespace mvv
{
namespace parser
{
   /**
    @brief Wrappers to isolate the corresponding functionalities ( win32 or Unix...)
    */
   void* LoadLibraryWrapper( const std::string& path );
   void* GetProcAdressWrapper( void* module, const std::string& name );
   void  FreeLibraryWrapper( void* module );
}
}

#endif