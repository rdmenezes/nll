#include "stdafx.h"
#include "system.h"
#include <windows.h>

namespace mvv
{
   int systemCall( const std::string& cmd )
   {
      return system( cmd.c_str() );
   }
}