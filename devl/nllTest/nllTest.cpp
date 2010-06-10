// nllTest.cpp : Defines the test runner entry point.
//

#include <fstream>
#include <tester/register.h>


int main()
{
   // erase logger
   std::ofstream f("nll.log");
   f.close();

   Register::instance().run();
}
