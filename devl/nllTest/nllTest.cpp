// nllTest.cpp : Defines the test runner entry point.
//

#include <fstream>
#include <tester/register.h>

# include "problem-builder-gmm.h"

namespace nll
{
   const ui8 colors[ 7 ][ 3 ] =
   {
      {255, 0, 0},
      {255, 255, 0},
      {255, 0, 255},
      {0, 255, 0},
      {0, 0, 255},
      {0, 255, 255},
      {255, 255, 255}
   };
}

int main()
{
   #ifdef NLL_FIND_MEMORY_LEAK
   _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
   _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
   _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
   _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
   #endif

   // erase logger
   std::ofstream f("nll.log");
   f.close();

   Register::instance().run();
}
