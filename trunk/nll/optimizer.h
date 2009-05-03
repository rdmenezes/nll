#ifndef NLL_OPTIMIZER_H_
# define NLL_OPTIMIZER_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief interface for a generic optimizer implementation
    */
   class NLL_API Optimizer
   {
   public:
      /**
       @brief Optimize a function by finding the minimum. How well the problem is solved is expressed by a function fitness
              provided by the <code>client</code> We are trying to find the best parameters minimizing this cost function.
       @param client the client to be optimized. It is used to give a score of a particular solution
       @param parameters defines the properties of the parameters. Properties like min, max, distribution, search operators...
              These properties are <b>very</b> important. If knowledge of the problem can be added, it would narrow a lot
              the search space, making the optimizer successful.
       */
      virtual std::vector<double> optimize( const OptimizerClient& client, const ParameterOptimizers& parameters ) = 0;

      virtual ~Optimizer()
      {}
   };
}
}

#endif
