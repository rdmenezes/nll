#ifndef MVV_NOTIFIABLE_H_
# define MVV_NOTIFIABLE_H_

namespace mvv
{
   /**
    @ingroup mvv
    @brief The class implementing this will be notifiable (i.e. if run on another thread and waiting, with method
           will run again)
    */
   class Notifiable
   {
   public:
      virtual void notify() = 0;
   };
}

#endif
