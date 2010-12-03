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
   //nll::colors = nll::ccolors;
   // erase logger
   std::ofstream f("nll.log");
   f.close();

   Register::instance().run();
}
