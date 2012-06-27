#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include <tester/register.h>
#include "database-benchmark.h"
#include "load-database.h"


int main()
{
   std::ofstream f("nll.log");
   f.close();

   return Register::instance().run();
}
