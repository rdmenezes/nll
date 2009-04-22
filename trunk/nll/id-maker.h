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
   class IdMaker
   {
   public:
      /**
       @brief get an instance
       */
      static IdMaker& instance()
      {
         static IdMaker id;

         return id;
      }

      /**
       @brief generate a uniq ID
       */
      ui32 generateId(){
         return ++_id;
      }

   private:
      IdMaker() : _id( 0 ){}

   private:
      ui32     _id;
   };
}
}

#endif
