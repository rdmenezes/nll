// nllTest.cpp : Defines the test runner entry point.
//
#include <fstream>
#include <tester/register.h>
#include <regionDetection/globals.h>


int main()
{
   Register::instance().run();
}

const char* datasets[] =
{
   DATA_PATH "case1.mf2",
   DATA_PATH "case2.mf2",
   DATA_PATH "case3.mf2",
   DATA_PATH "case5.mf2",
   DATA_PATH "case8.mf2",
   DATA_PATH "case10.mf2",
   DATA_PATH "case13.mf2",
   DATA_PATH "case14.mf2",
   DATA_PATH "case15.mf2",
   DATA_PATH "case16.mf2",
   DATA_PATH "case17.mf2",
   DATA_PATH "case18.mf2",
   DATA_PATH "case19.mf2",
   DATA_PATH "case20.mf2",
   DATA_PATH "case21.mf2",
   DATA_PATH "case22.mf2",
   DATA_PATH "case23.mf2",
   DATA_PATH "case24.mf2",
   DATA_PATH "case25.mf2",
   DATA_PATH "case26.mf2",
   DATA_PATH "case27.mf2",
   DATA_PATH "case28.mf2",
   DATA_PATH "case29.mf2",
   DATA_PATH "case30.mf2",
   DATA_PATH "case31.mf2",
   DATA_PATH "case32.mf2",
   DATA_PATH "case33.mf2",
   DATA_PATH "case35.mf2",
//   DATA_PATH "case36.mf2",
   DATA_PATH "case37.mf2",
   DATA_PATH "case38.mf2",
   DATA_PATH "case39.mf2",
   DATA_PATH "case40.mf2",
   DATA_PATH "case41.mf2",
   DATA_PATH "case42.mf2",
   DATA_PATH "case44.mf2",
   DATA_PATH "case45.mf2",
   DATA_PATH "case46.mf2",
   DATA_PATH "case47.mf2",
   DATA_PATH "case48.mf2",
   DATA_PATH "case49.mf2",
   DATA_PATH "case50.mf2",
   DATA_PATH "case51.mf2",
   DATA_PATH "case52.mf2",
   DATA_PATH "case53.mf2",
   DATA_PATH "case54.mf2",
   DATA_PATH "case55.mf2",
   DATA_PATH "case56.mf2",
   DATA_PATH "case57.mf2",
   DATA_PATH "case58.mf2",
   DATA_PATH "case59.mf2",
   DATA_PATH "case60.mf2",
   DATA_PATH "case61.mf2",
   DATA_PATH "case62.mf2",
   DATA_PATH "case63.mf2",
   DATA_PATH "case64.mf2",
   /*DATA_PATH "case65.mf2",
   DATA_PATH "case66.mf2",
   DATA_PATH "case67.mf2",*/
   DATA_PATH "case68.mf2",
   DATA_PATH "case69.mf2",
   DATA_PATH "case70.mf2",
};