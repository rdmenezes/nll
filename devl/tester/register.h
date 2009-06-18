#ifndef REGISTER_H_
# define REGISTER_H_

# include <vector>
# include <string>
# include <iostream>
# include "tester.h"
# include "config.h"


# pragma warning( push )
# pragma warning( disable:4251 ) // std::vector should have dll-interface
# pragma warning( disable:4996 ) // deprecation

// define this if you don't want to run unit tests on several threads
//#define NO_MULTITHREADED_UNIT_TESTS

# define NLL_TESTER_LOG_PATH   "../../tester/log/"

/**
 @defgroup core

 It defines procedures to automatically run and report unit tests. It is based on the cppunit interface.
 Unit tests <b>must not</b> have side effects between themselves as unit tests are run in parrallel.
 */

namespace nll
{
namespace impl
{
   inline std::string ftoa( double val )
   {
      std::string res;
      std::stringstream f;
      f << val;
      f >> res;
      return res;
   }
}
}

class TestSuite;

class TESTER_API FailedAssertion : public std::exception
{
public:
   FailedAssertion( const char* msg ) throw() : _msg( msg )
   {
   }
   ~FailedAssertion() throw()
   {
   }
   virtual const char* what() const throw()
   {
      return _msg.c_str();
   }
private:
   std::string _msg;
};

class TESTER_API Register
{
   struct Failed
   {
      Failed( const char* fu, const char* fi, const char* ms ) :
         funcName( fu ),
         file( fi ),
         msg( ms )
      {}
      const char* funcName;
      const char* file;
      std::string msg;
   };

private:
   typedef std::vector<TestSuite*>     Suites;
   typedef std::vector<Failed>         Faileds;

public:
   Register() : _successful( 0 )
   {
      // configure the regression log
      std::string mode;
# ifdef NDEBUG
      mode = "release";
# else
      mode = "debug";
# endif
      const std::string name = getenv( "NLL_MACHINE_ID" ) ? std::string( getenv( "NLL_MACHINE_ID" ) ) : "UNKNOWN";
      const std::string dirMetadata = "nll.metadata." + mode + "." + name;
      const std::string dirTestdata = "nll.testdata." + mode + "." + name;

      _config.setDirectory( dirMetadata );
      _config[ "nll.version" ] = NLL_VERSION;
      _config[ "nll.machine" ] = name;

      _config.setDirectory( dirTestdata );
   }

   static Register& instance()
   {
      static Register reg;
      return reg;
   }

   void add( TestSuite* suite )
   {
      _suites.push_back( suite );
   }

   void regression( const std::string& key, const std::string& val )
   {
      _config[ key ] = val;
   }

   void successful()
   {
      // add a crtical section: it must not be accessed concurently
      #pragma omp critical
      {
         ++_successful;
      }
   }

   void failed( const char* func, const char* file, std::exception& msg )
   {
      // add a crtical section: it must not be accessed concurently
      #pragma omp critical
      {
         _faileds.push_back( Failed( func, file, msg.what() ) );
      }
   }

   void regressionExport() const
   {
      const std::string name = NLL_TESTER_LOG_PATH + std::string( "nll." ) + nll::core::val2str( clock() ) + ".log";
      std::cout << "name=" << name << std::endl;
      _config.write( name );
      nll::tester::Config rconfig( NLL_TESTER_LOG_PATH "regression.log" );

   }

   unsigned run();

private:
   Register( const Register& );
   Register& operator=( const Register& );

private:
   Suites               _suites;
   unsigned             _successful;
   Faileds              _faileds;
   nll::tester::Config  _config;
};

class TESTER_API TestSuite
{
public:
   typedef void (* pFunc)();

public:
   TestSuite( pFunc f ) : _f( f )
   {
      Register::instance().add( this );
   }

   void run()
   {
      _f();
   }

private:
   pFunc    _f;
};

# define TESTER_STREAM std::cout

# define MAKE_UNIQUE( symb )   symb##_FILE_##_LINE_

# define TESTER_TEST_SUITE( testSuite )               \
            static void testSuite##_suite();          \
            static TestSuite MAKE_UNIQUE(testSuite) (testSuite##_suite);\
            static void testSuite##_suite()           \
            {                                         \
               testSuite instance;                    \
               const char* name = #testSuite;
               

# define TESTER_TEST( func )                          \
               try                                    \
               {                                      \
                  nll::core::Timer startTaskTimer_;   \
                  TESTER_STREAM << ".";               \
                  instance.func();                    \
                  Register::instance().regression( "nll." + std::string( name ) + "." + #func, nll::impl::ftoa( startTaskTimer_.getCurrentTime() ) ); \
                  Register::instance().successful();  \
               } catch ( std::exception& e )          \
               {                                      \
                  Register::instance().failed( #func, name, e );      \
               }                                      \

# define TESTER_TEST_SUITE_END()                      \
            }                                         \

# define TESTER_ASSERT( exp )                         \
            if ( !( exp ) )                           \
            {                                         \
               TESTER_STREAM << "F";                  \
               throw FailedAssertion( ( std::string("assert failed \"" ) + #exp + std::string( "\"" ) ).c_str() );   \
            }

# define TESTER_UNREACHABLE throw std::exception( ( std::string("assert failed \"" ) + #exp + std::string( "\"" ) ).c_str() );

#pragma warning( pop )

#endif
