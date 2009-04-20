#ifndef STDAFX_H
#define STDAFX_H

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

// TODO: reference precompiled headers your program requires here
# include "database-builder.h"
# include <tester/register.h>

// only for speed purpose...
//#define DONT_RUN_SLOW_TEST
#define DONT_RUN_VERY_SLOW_TEST
//#define DONT_RUN_TEST

#else
#error This file can only be included once.
#endif
