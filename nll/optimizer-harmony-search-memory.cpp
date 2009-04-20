#include "stdafx.h"
#include "nll.h"

namespace nll
{
namespace algorithm
{
   void OptimizerHarmonySearchMemory::_initializeMemory( const OptimizerClient& client, const ParameterOptimizers& parameters )
   {
      assert( parameters.size() );
      assert( _hms );

      _memory = HarmonyMemory( _hms );
      _evaluatedSolutions.clear();
      for ( ui32 n = 0; n < _hms; ++n )
      {
         SolutionVector sol( static_cast<ui32>( parameters.size() ) );
         for ( ui32 ni = 0; ni < parameters.size(); ++ni )
         {
            sol[ ni ] = parameters[ ni ].generate();
         }
         _memory[ n ] = SolutionStorage( sol, client.evaluate( sol ) );
      }
      std::sort( _memory.rbegin(), _memory.rend() );
   }
}
}
