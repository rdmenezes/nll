#ifndef NLL_CORE_EXCEPTION_H_
# define NLL_CORE_EXCEPTION_H_

namespace nll
{
namespace core
{
   class Exception : public std::exception
   {
   public:
      Exception( const char* msg ) throw() : _msg( msg )
      {}

      virtual ~Exception() throw()
      {
      }

      virtual const char* what() const throw()
      {
         return _msg;
      }

   private:
      const char* _msg;
   };
}
}

#endif
