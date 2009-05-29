#ifndef NLL_SINGLETON_H_
# define NLL_SINGLETON_H_

namespace nll
{
namespace core
{
   /**
    @brief Generic thread-safe singleton
    */
   template <class T>
   class Singleton
   {
   public:
      static T& instance()
      {
         if ( !_instance )
         {
            // use a double lock so that we only lock this when it is created
            #pragma omp critical
            {
               if ( !_instance )
               {
                  _instance = new T();
               }
            }
         }
         return *_instance;
      }

   protected:
      // disable copy and new instanciations
      Singleton();
      ~Singleton();

   private:
      Singleton& operator=( const Singleton& );

      Singleton( const Singleton& );

      static T* _instance;
   };

   template <class T> T* Singleton<T>::_instance = 0;
}
}

#endif
