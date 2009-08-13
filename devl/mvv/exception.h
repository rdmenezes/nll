#ifndef MVV_EXCEPTION_H_
# define MVV_EXCEPTION_H_

# include <string>
# include "mvv.h"

namespace mvv
{
   class MVV_API Exception
   {
   public:
      Exception( const std::string& msg ) : _msg( msg )
      {}

      const std::string& what() const
      {
         return _msg;
      }

   protected:
      std::string _msg;
   };

   class MVV_API ExceptionBadOrder : public Exception
   {
   public:
      ExceptionBadOrder( const std::string& msg ) : Exception( msg )
      {}
   };

   class MVV_API ExceptionOrderNotConsumed : public Exception
   {
   public:
      ExceptionOrderNotConsumed( const std::string& msg ) : Exception( msg )
      {}
   };
}

#endif