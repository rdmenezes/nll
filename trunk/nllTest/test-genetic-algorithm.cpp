#include "stdafx.h"
#include <nll/nll.h>
#include <time.h>

struct GA_Eval
{
   double operator()(const nll::core::Buffer1D<nll::f64>& g) const
   {
      assert(g.size() == 5);
      nll::f64 tmp =	g[0] * sin(g[0]) +
			      1.7 * g[1] * sin(g[0]) -
			      1.5 * g[2] - 0.1 * g[3] * cos(g[3] + g[4] - g[0]) + (0.2 * g[4] * g[4] - g[1]) - 1;
      return nll::core::absolute(tmp);
   }
};

struct GA_GenerateGene
{
   nll::core::Buffer1D<nll::f64> operator()() const
   {
      nll::core::Buffer1D<nll::f64> g(5);
      for (nll::ui32 n = 0; n < 5; ++n)
	      g[n] = (nll::f32)(rand() % 200) - 100;
      return g;
   }
};

struct GA_Mutate
{
   void operator()(nll::core::Buffer1D<nll::f64>& g) const
   {
      assert(g.size() == 5);
      nll::ui32 n = rand() % 5;
      nll::f32 d = static_cast<nll::f32>((rand() % 10000) - 5000) / 20000;
      g[n] += d;
   }
};

class TestGA
{
public:
   void testGA()
   {
      srand(1);
      typedef nll::core::Buffer1D<nll::f64> Gene;
      typedef std::vector<Gene> Genes;

      GA_GenerateGene   generate;
      GA_Eval           evaluate;
      GA_Mutate         mutate;
      Gene              seed = Gene(5);

      srand((nll::ui32)time(0));
      Genes genes = nll::algorithm::launchGeneticAlgorithm(seed, generate, evaluate, mutate, 50, 100, 0.3, 0.1, 20);
      double val = evaluate( genes[ 0 ] );
      std::cout << "min=" << evaluate(genes[0]) << std::endl;
      for (int n = 0; n < 5; ++n)
         std::cout << genes[0][n] << " ";
      std::cout << std::endl;
      TESTER_ASSERT( val < 0.0001 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGA);
# ifndef DONT_RUN_SLOW_TEST
TESTER_TEST(testGA);
# endif
TESTER_TEST_SUITE_END();
#endif
