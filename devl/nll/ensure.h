#ifndef NLL_ENSURE_H_
# define NLL_ENSURE_H_

/**
 * @brief Ensure that the expression is true, else log the error and stop the program
 */
# ifdef _MSC_VER
#  define ensure( _Expression, _String )							      \
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
# else
#  define ensure( _Expression, _String )							      \
      if ( !( _Expression ) )										         \
      {															               \
         nll::core::LoggerNll::write( nll::core::LoggerNll::ERROR, _String );			\
	      std::cout << "------------" << std::endl;				      \
	      std::cout << "Error : " << _String << std::endl;		   \
	      std::cout << "  Location : " << __FILE__ << std::endl;	\
	      std::cout << "  Line     : " << __LINE__ << std::endl;	\
         exit( 1 );                                               \
      }
# endif
#endif
