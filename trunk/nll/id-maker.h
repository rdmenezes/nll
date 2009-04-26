#ifndef NLL_ID_H_
# define NLL_ID_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Singleton generating uniq IDs
    */
   class _IdMaker
   {
   public:
      /**
       @brief generate a unique ID
       */
      ui32 generateId()
      {
         #pragma omp atomic
         ++_id;

         return _id;
      }

      _IdMaker() : _id( 0 ){}

   private:
      ui32     _id;
   };

   typedef Singleton<_IdMaker>   IdMaker;
}
}

#endif
