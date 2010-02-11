#ifndef MVV_PARSER_ERROR_H_
# define MVV_PARSER_ERROR_H_

# include <sstream>

namespace mvv
{
namespace parser
{
   /**
    @brief A class centralizing all the error of the parser namespace
    */
   class Error
   {
   public:
      enum ErrorType
      {
         SUCCESS = 0,   /// job successfull
         FAILURE,       /// unspecied error
         SCAN,          /// lexical error
         PARSE,         /// parse error
         BIND,          /// binding error
         TYPE           /// type checking error
      };
   public:
      Error() : _status( SUCCESS )
      {}

      template <class T>
      Error& operator<< ( const T& err )
      {
         _stream << err;
         return *this;
      }

      Error& operator<< ( const ErrorType err )
      {
         if ( err && err < _status || !_status )
         {
            _status = err;
         }
         return *this;
      }

      Error& operator<< ( const Error& err )
      {
         *this << err.getStatus() << err.getMessage().str();
         return *this;
      }

      Error& operator<< ( const char* err )
      {
         _stream << err;
         return *this;
      }

      ErrorType getStatus() const
      {
         return _status;
      }

      const std::stringstream& getMessage() const
      {
         return _stream;
      }

      void clear()
      {
         _status = SUCCESS;
         _stream.clear();
      }


   private:
      std::stringstream    _stream; 
      ErrorType            _status;
   };

   inline std::ostream& operator<< ( std::ostream& o, const Error& e )
   {
      o << e.getMessage().str();
      return o;
   }
}
}

#endif