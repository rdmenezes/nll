#ifndef MVV_PARSER_IMPORT2_H_
# define MVV_PARSER_IMPORT2_H_

#include <string>

namespace mvv
{
namespace parser
{
   void* LoadLibraryWrapper( const std::string& path );
   void* GetProcAdressWrapper( void* module, const std::string& name );
}
}

#endif