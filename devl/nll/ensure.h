#ifndef NLL_ENSURE_H_
# define NLL_ENSURE_H_

//#define NLL_NO_ENSURE

# ifndef nllWarning
#  define nllWarning(str)   std::cerr << "warning: " << str << std::endl;
# endif

/**
 * @brief Ensure that the expression is true, else log the error and stop the program
          it can be turned off by setting the flag <code>NLL_NO_ENSURE</code>

   It is typically used to ensure that the program stays in a correct state for a developer version.
   However it should be disabled for a released version. It is different from <code>assert</code> since
   it won't really slow down the components as it is not used in internal loops/critical code.
 */
#ifndef ensure
#  ifdef NLL_NO_ENSURE
#   define ensure( _Expression, _String )                 (_Expression)                  
#  else
#   ifdef _MSC_VER
#    define ensure( _Expression, _String )							      \
      if ( !( _Expression ) )										         \
      {															               \
         nll::core::LoggerNll::write( nll::core::LoggerNll::ERROR, _String );			\
	      std::cout << "------------" << std::endl;				      \
	      std::cout << "Error : " << _String << std::endl;		   \
	      std::cout << "  Location : " << __FILE__ << std::endl;	\
	      std::cout << "  Line     : " << __LINE__ << std::endl;	\
         _CrtDbgBreak();                                          \
         exit( 1 );                                               \
      }
#   else
#    define ensure( _Expression, _String )							      \
      if ( !( _Expression ) )										         \
      {															               \
         nll::core::LoggerNll::write( nll::core::LoggerNll::ERROR, _String );			\
	      std::cout << "------------" << std::endl;				      \
	      std::cout << "Error : " << _String << std::endl;		   \
	      std::cout << "  Location : " << __FILE__ << std::endl;	\
	      std::cout << "  Line     : " << __LINE__ << std::endl;	\
         exit( 1 );                                               \
      }
#   endif
#  endif
# endif
#endif
