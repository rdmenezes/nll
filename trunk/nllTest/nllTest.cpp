// nllTest.cpp : Defines the test runner entry point.
//

#include "stdafx.h"
#include <fstream>


int main()
{
   // erase logger
   std::ofstream f("nll.log");
   f.close();

   Register::instance().run();
}
