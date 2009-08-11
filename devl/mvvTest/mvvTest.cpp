// mvvTest.cpp : Defines the test runner entry point.
//

#include "stdafx.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#pragma comment(lib, "cppunitd_dll.lib")
#else
#pragma comment(lib, "cppunit_dll.lib")
#endif
#endif

int main()
{
   CppUnit::TextUi::TestRunner runner;
   CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();

   runner.addTest(registry.makeTest());
   
   if (!runner.run("", false))
      return 1;      // nonzero if the test failed

   return 0;
}
