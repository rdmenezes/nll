#ifndef NLL_STATIC_PLUG
# define NLL_STATIC_PLUG

# include "database-benchmark.h"
         
namespace nll
{
namespace benchmark
{
   /**
    @brief define a static pluggin: execute a function at startup

    Use a global variable to run the function. There is no guarantee about the order of execution
    */
   class StaticPlug
   {
   public:
      typedef void ( * pFunc )();

   public:
      StaticPlug( pFunc func )
      {
         func();
      }
   };
}
}

# define MAKE_UNIQUE( symb )   symb##_FILE_##_LINE_

# define STATIC_PLUG( func )                    \
   nll::benchmark::StaticPlug MAKE_UNIQUE( func ) ( func );

#endif
