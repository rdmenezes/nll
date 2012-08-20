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

// define this flag to disable the regression time comparison
//#define NLL_TESTER_NO_REGRESSION

// define this if you don't want to run unit tests on several threads
#define NO_MULTITHREADED_UNIT_TESTS

# define NLL_TESTER_LOG_PATH   "../../tester/log/"
# define TESTER_STREAM std::cout

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

      _tolerance = 0.020;        // 20% tolerance
      _regressionMinTime = 0.035;  // in second
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
#ifndef NO_MULTITHREADED_UNIT_TESTS
      #pragma omp critical
#endif
      {
         ++_successful;
      }
   }

   void failed( const char* func, const char* file, std::exception& msg )
   {
      // add a crtical section: it must not be accessed concurently
#ifndef NO_MULTITHREADED_UNIT_TESTS
      #pragma omp critical
#endif
      {
         _faileds.push_back( Failed( func, file, msg.what() ) );
      }
   }

   void regressionExport() const
   {
      // just export the raw config
      const std::string name = NLL_TESTER_LOG_PATH + std::string( "nll." ) + nll::core::val2str( time( 0 ) ) + ".log";
      _config.write( name );

      // reload and compare with previous runs
      const std::string regressionLog = NLL_TESTER_LOG_PATH "regression.log";
      nll::tester::Config rconfig( regressionLog );
      for ( nll::tester::Config::Storage::const_iterator directory = _config._storage.begin();
            directory != _config._storage.end();
            ++directory )
      {
         rconfig.setDirectory( directory->first );

         // check if it is the metadata directory
         bool isMetadataDirectory = false;
         std::string dir = directory->first;
         const std::vector<const char*> splits = nll::core::split( dir, '.' );
         if ( splits.size() >= 3 && splits[ 0 ] == std::string( "nll" ) && splits[ 1 ] == std::string( "metadata" ) )
            isMetadataDirectory = true;

         for ( nll::tester::Config::Directory::const_iterator item = directory->second.begin();
               item != directory->second.end();
               ++item )
         {
            if ( !isMetadataDirectory && rconfig[ item->first ] != "" )
            {
               // check the timings
               const double vref = nll::core::str2val<double>( rconfig[ item->first ] );
               const double v = nll::core::str2val<double>( item->second );
               bool displayedWarning = false;
               if ( v > vref * ( 1 + _tolerance ) )
               {
                  if ( v > _regressionMinTime )
                  {
                     displayedWarning = true;
                     TESTER_STREAM << "warning performance:" << directory->first << ":" << item->first << " ref="
                                   << vref << " current=" << v << std::endl;
                  }
               }
               if ( !displayedWarning )
               {
                  TESTER_STREAM << "time:" << item->first << " ref="
                                << vref << " current=" << v << std::endl;
               }

               // if better value, just copy it
               if ( v < vref )
               {
                  rconfig[ item->first ] = item->second;
               }
            } else {
               // just copy the item
               rconfig[ item->first ] = item->second;
            }
         }
      }
      rconfig.write( regressionLog );
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
   double               _tolerance; /// in %
   double               _regressionMinTime; /// in second
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

# define MAKE_UNIQUE( symb )   symb##_FILE_##_LINE_

# define TESTER_TEST_SUITE( testSuite )               \
            static void testSuite##_suite();          \
            static TestSuite MAKE_UNIQUE(testSuite) (testSuite##_suite);\
            static void testSuite##_suite()           \
            {                                         \
               testSuite instance;                    \
               const char* name = #testSuite;         \
               TESTER_STREAM << "# Runing test suite: " << name << std::endl; \
               

# define TESTER_TEST( func )                          \
               try                                    \
               {                                      \
                  TESTER_STREAM << "#  Runing test: " << #func << std::endl; \
                  nll::core::Timer startTaskTimer_;   \
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
               TESTER_STREAM << "# #F#" << std::endl; \
               throw FailedAssertion( ( std::string("assert failed \"" ) + #exp + std::string( "\"" ) ).c_str() );   \
            }

# define TESTER_UNREACHABLE throw std::exception( ( std::string("assert failed \"" ) + #exp + std::string( "\"" ) ).c_str() );

#pragma warning( pop )

#endif
