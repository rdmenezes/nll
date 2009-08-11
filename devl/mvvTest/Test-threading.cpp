#include "stdafx.h"

// Copyright (C) 2001-2003
// William E. Kempf
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/thread/thread.hpp>
#include <boost/thread/once.hpp>
#include <cassert>

int value=0;
boost::once_flag once = BOOST_ONCE_INIT;

void init()
{
    ++value;
}

void thread_proc()
{
    boost::call_once(&init, once);
}

class TestThreading
{
public:
   void test1()
   {
      boost::thread_group threads;
      for (int i=0; i<5; ++i)
         threads.create_thread(&thread_proc);
      threads.join_all();
      assert(value == 1);
      std::cout << "ok" << std::endl;
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestThreading);
TESTER_TEST(test1);
TESTER_TEST_SUITE_END();
#endif