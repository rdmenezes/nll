/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_ENSURE_H_
# define NLL_ENSURE_H_

//#define NLL_NO_ENSURE
#define NLL_ENSURE_THROW

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
#   ifdef NLL_ENSURE_THROW
#    define ensure( _e, _s ) if ( ( _e ) == 0 ) { throw std::runtime_error( _s ); }
#   else
#    ifdef _MSC_VER
#     define ensure( _Expression, _String )							      \
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
#    else
#     define ensure( _Expression, _String )							      \
      if ( !( _Expression ) )										         \
      {															               \
         nll::core::LoggerNll::write( nll::core::LoggerNll::ERROR, _String );			\
	      std::cout << "------------" << std::endl;				      \
	      std::cout << "Error : " << _String << std::endl;		   \
	      std::cout << "  Location : " << __FILE__ << std::endl;	\
	      std::cout << "  Line     : " << __LINE__ << std::endl;	\
         exit( 1 );                                               \
      }
#    endif
#   endif
#  endif
# endif
#endif
