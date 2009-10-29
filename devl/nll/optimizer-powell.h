#ifndef NLL_ALGORITM_OPTIMIZER_POWELL_H_
# define NLL_ALGORITM_OPTIMIZER_POWELL_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Powell optimizer. Efficient for problems with low dimentionality (<= 10)
    @param randomSeed the powell optimization is independantly generated a fixed number of times. The best solution is exported
    @param tolerance the tolerance of the algorithm
    */
   class NLL_API OptimizerPowell : public Optimizer
   {
   public:
      OptimizerPowell( ui32 randomSeed = 40, f64 tolerance = 1e-4 ) : _nbSeeds( randomSeed ), _tolerance( tolerance )
      {}

      /**
       @brief returns an empty set if optimization failed.
       */
      virtual std::vector<double> optimize( const OptimizerClient& client, const ParameterOptimizers& parameters )
      {
         double min = 1e20;
         core::Buffer1D<f64> best;

         std::vector< core::Buffer1D<double> > dir;
         for ( ui32 n = 0; n < parameters.size(); ++n )
         {
            dir.push_back( core::Buffer1D<f64>( parameters.size() ) );
            dir[ n ][ n ] = 1;
         }

         for ( ui32 n = 0; n < _nbSeeds; ++n )
         {
            core::Buffer1D<f64> ini( parameters.size() );
            for ( ui32 n = 0; n < parameters.size(); ++n )
               ini[ n ] = parameters[ n ].generate();
            bool error = false;
            double val = powell( ini, dir, _tolerance, client, 200, &error );
            if ( error )
               continue;

            if ( val < min )
            {
               min = val;
               best.clone( ini );
            }
         }

         if ( !best.size() )
            return std::vector<double>();

         std::vector<double> result( parameters.size() );
         for ( ui32 n = 0; n < parameters.size(); ++n )
            result[ n ] = best[ n ];
         return result;
      }

   protected:
      ui32  _nbSeeds;
      f64   _tolerance;
   };
}
}

#endif
