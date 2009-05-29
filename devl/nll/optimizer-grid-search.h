#ifndef OPTIMIZER_GRID_SEARCH_H_
# define OPTIMIZER_GRID_SEARCH_H_

#pragma warning( push )
#pragma warning( disable:4251 ) // std::vector need dll-interface
#pragma warning( disable:4512 ) // std::vector need dll-interface
#pragma warning( disable:4127 ) // conditional expression is constant

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Find the optimal parameters using an exhaustif search. The search step for each parameter is defined
           by <code>params[n].next(val)</code> The whole range for each parameter is searched, this method is
           NOT suitable for a complex search space (lots of parameters and search range).

           Grid search could be extremly expensive in time, but it is guaranteed that it will find the minimum
           value on this grid. If the evaluation function is quite fast, and the range search is small, the algorithm
           is suitable.
    */
   class OptimizerGridSearch : public Optimizer
   {
   public:
      /**
       @brief Optimizer. Use the parameter granularities to increment the search.
      */
      virtual std::vector<double> optimize( const OptimizerClient& client, const ParameterOptimizers& params )
      {
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "grid search started" );
         core::Buffer1D<double> pos( params.size() );
         for ( ui32 n = 0; n < params.size(); ++n )
            pos[ n ] = params[ n ].getMin();

         core::Buffer1D<double> bestSolution;
         double min = INT_MAX;
         while ( 1 )
         {
            std::stringstream sstr;
            sstr << "test point:";
            pos.print( sstr );
            sstr << "best=";
            bestSolution.print( sstr );
            sstr << "val=" << min;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sstr.str() );

            double val = client.evaluate( pos );
            if ( val < min )
            {
               min = val;
               bestSolution.clone( pos );
            }

            pos[ 0 ] = params[ 0 ].next( pos[ 0 ] );
            for ( ui32 n = 0; n < params.size() - 1; ++n )
               if ( pos[ n ] >= params[ n ].getMax() )
               {
                  pos[ n ] = params[ n ].getMin();
                  pos[ n + 1 ] = params[ n + 1 ].next( pos[ n + 1 ] );
               } else break;
            if ( pos[ params.size() - 1 ] >= params[ params.size() - 1 ].getMax() )
               break;
         }

         std::stringstream sstr;
         sstr << "grid search best solution=" << min << std::endl;
         bestSolution.print( sstr );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sstr.str() );

         std::vector<double> sol( params.size() );
         for ( ui32 n = 0; n < params.size(); ++n )
            sol[ n ] = bestSolution[ n ];
         return sol;
      }
   };
}
}

#pragma warning( pop )

#endif
