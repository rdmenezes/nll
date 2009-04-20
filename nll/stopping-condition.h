#ifndef NLL_STOPPING_CONDITION_H_
# define NLL_STOPPING_CONDITION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief define a stopping condition criteria
    */
   class NLL_API StopCondition
   {
   public:
      /**
       @brief return true if algorithm needs to be stopped else return false
       */
      virtual bool stop( f64 fitness ) const = 0;

      /**
       @brief reitinialize the stop condition if necessary
       */
      virtual void reinit(){}
   };

   /**
    @ingroup algorithm
    @brief define a stopping condition only based on the number of iterations
    */
   class NLL_API StopConditionIteration : public StopCondition
   {
   public:
      StopConditionIteration( ui32 nbIterMax ) : _nbIterMax( nbIterMax ), _iter( 0 )
      {}

      /**
       @brief increment the iteration counter each this method is called
       */
      virtual bool stop( f64 /*fitness*/ ) const
      {
         if ( _iter >= _nbIterMax )
            return true;
         ++_iter;
         return false;
      }

      virtual void reinit()
      {
         _iter = 0;
      }

   private:
      mutable ui32 _iter;
      ui32 _nbIterMax;
   };

   /**
    @ingroup algorithm
    @brief define a stopping condition only based on the fitness
    */
   class NLL_API StopConditionFitness : public StopCondition
   {
   public:
      StopConditionFitness( f64 minFitness ) : _minFitness( minFitness )
      {}

      /**
       @brief increment the iteration counter each this method is called
       */
      virtual bool stop( f64 fitness ) const
      {
         return _minFitness <= fitness;
      }

   private:
      f64 _minFitness;
   };
}
}

#endif
