#ifndef NLL_OPTIMIZER_CLIENT_H_
# define NLL_OPTIMIZER_CLIENT_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Base class for a client to be optimized. The optimizer has to find the best parameters minimizing the cost
           funtion.
           
           A key point for optimization algorithms is how quickly, a solution can be evaluated. It is too slow,
           the number of solutions tested will be too slow, and the underlying algorithm may not converge to an
           acceptable solution.
    */
   class NLL_API OptimizerClient
   {
   public:
      /**
       @brief return the score of the optimization for a specific set of parameters.

       The lowest is the score, the better.
       @sa Classifier
       */
      virtual double evaluate( const nll::core::Buffer1D<nll::f64>& parameters ) const = 0;

      /**
       @todo deprecate...
       */
      double operator()( const nll::core::Buffer1D<nll::f64>& parameters ) const { return evaluate( parameters ); }

      virtual ~OptimizerClient()
      {}
   };
}
}

#endif
