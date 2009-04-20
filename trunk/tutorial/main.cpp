#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include <tester/register.h>
#include "database-benchmark.h"
#include "load-database.h"


int main()
{
   return Register::instance().run();
}
