#ifndef STDAFX_H
#define STDAFX_H

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4511 4512 4702)
#endif
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// TODO: reference precompiled headers your program requires here

#else
#error This file can only be included once.
#endif
