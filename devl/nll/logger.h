#ifndef NLL_LOGGER_H_
# define NLL_LOGGER_H_

# include <string>
# include <fstream>
# include <iostream>
# include <assert.h>

# pragma warning( push )
# pragma warning( disable:4251 ) // dll-interface


namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Define a logger base class
    */
   class NLL_API LoggerBase
   {
   public:
      virtual ~LoggerBase(){}

      /**
       @brief write a string to the log
       */
      virtual void write( const std::string& str ) = 0;

      /**
       @brief return the internal stream object
       */
      virtual std::ostream& getStream() = 0;
   };

   /**
    @ingroup core
    @brief Specific implemenatation of the logger using text files
    @sa LoggerBase
    */
   class NLL_API LoggerFile : public LoggerBase
   {
   public:
      LoggerFile( const std::string& file ) : _o( file.c_str(), std::ios_base::app ){ assert( _o.is_open() ); }
      virtual void write( const std::string& str )
      {
         _o << str << std::endl;
      }
      virtual std::ostream& getStream()
      {
         return _o;
      }

   private:
      std::ofstream _o;
   };


   /**
    @ingroup core
    @brief Singleton that holds all logger instances.
    */
   class _LoggerHandler
   {
      typedef std::vector<LoggerBase*> Container;

   public:
      /**
       @brief destroy all the loggers
       */
      ~_LoggerHandler()
      {
         for ( ui32 n = 0; n < _loggers.size(); ++n )
            destroyLogger( n );
      }

      /**
       @brief destroy a specific logger
       */
      void destroyLogger( const ui32 id )
      {
# ifndef NLL_NOT_MULTITHREADED
         #pragma omp critical
# endif
         {
            assert( id < _loggers.size() );
            delete _loggers[ id ];
            _loggers[ id ] = 0;
         }
      }

      /**
       @brief retireve the instance of a specific logger
       */
      LoggerBase& getLogger( const ui32 id )
      {
         assert( id < _loggers.size() );
         return *_loggers[ id ];
      }

      /**
       @brief create a new logger
       @todo use a factory
       @return the id of the logger
       */
      ui32 createFileLogger( const std::string& file )
      {
         ui32 id = 0;
# ifndef NLL_NOT_MULTITHREADED
         #pragma omp critical
# endif
         {
            LoggerBase* l = new LoggerFile( file );
            _loggers.push_back( l );
            id = static_cast<ui32>( _loggers.size() - 1 );
         }
         return id;
      }

   private:
      Container   _loggers;
   };

   typedef Singleton<_LoggerHandler> LoggerHandler;


   /**
    @ingroup core
    @brief Specific implementation for logging nll events.
    */
   class LoggerNll
   {
   public:
      enum  LogType
      {
            IMPLEMENTATION,   ///< implementation level of details logged
            INFO,             ///< info level of details logged
            WARNING,          ///< warning level of details logged
            ERROR             ///< error level of details logged
      };

      // set the logger level wanted!
      enum  LogLevel
      {
            LOGGER_LEVEL = IMPLEMENTATION ///< set the current log level, see LogType
      };

   public:
      /**
       @brief write a string to a log
       @param type the level of detail of the vent
       @param str the message to be logged
       */
      static void write( const LogType type, const std::string str )
      {
         if ( (int)LOGGER_LEVEL <= (int) type )
         {
            getLogger().write( "L" + val2str( (int)type ) + ":" + str );
         }
      }

      /**
       @brief return the logger for the nll library
       */
      static LoggerBase& getLogger()
      {
         static ui32 coreIdLogger = std::numeric_limits<ui32>::max();
         if ( coreIdLogger == std::numeric_limits<ui32>::max() )
         {
            //#pragma omp critical
            {
               if ( coreIdLogger == std::numeric_limits<ui32>::max() )
                  coreIdLogger = LoggerHandler::instance().createFileLogger("nll.log");
            }
         }
         return LoggerHandler::instance().getLogger( coreIdLogger );
      }
   private:
      // can't instanciate this class
      LoggerNll();
   };

}
}

# pragma warning ( pop )

#endif
