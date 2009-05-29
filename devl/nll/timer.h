#ifndef NLL_TIMER_H_
# define NLL_TIMER_H_

# include <time.h>
# include "types.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Define a simple Timer class.
   */
   class Timer
   {
   public:
      /**
       @brief Instanciate the timer and start it
       */
      Timer()
      {
         start();
         _cend = 0;
      }

      /**
       @brief restart the timer
       */
      void start()
      {
         _cstart = clock();
      }

      /**
       @brief end the timer, return the time in seconds spent
       */
      f32 end()
      {
         _cend = clock();
         return getTime();
      }

      /**
       @brief end the timer, return the time in seconds spent. Can be called again, in this case starting time is still the same.
       */
      f32 getCurrentTime()
      {
         end();
         return static_cast<f32>( _cend - _cstart ) / CLOCKS_PER_SEC;
      }

      /**
       @brief return the time in seconds spent since between starting and ending the timer. The timer needs to be ended before calling it.
       */
      f32 getTime() const
      {
         return static_cast<f32>( _cend - _cstart ) / CLOCKS_PER_SEC;
      }

   private:
      time_t   _cstart;
      time_t   _cend;
   };
}
}

#endif
