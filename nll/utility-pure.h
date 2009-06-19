#ifndef NLL_UTILITY_PURE_H_
# define NLL_UTILITY_PURE_H_

# include <limits>
# include <string>
# include <vector>
# include <sstream>

# ifdef NLL_BOUND
#  undef NLL_BOUND
# endif

/**
 @brief bound a value between 2 bounds. Special warning: min and max type have to match val type!!
 */
# define NLL_BOUND(val, min, max) ( ( ( val ) > ( max ) ) ? ( max ) : ( ( ( val ) < ( min ) ) ? ( min ) : ( val ) ) )


namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief calculate an absolute value. Work around for Visual and ambiguous case...
    */
   inline double absolute( double val )
   {
      return val >= 0 ? val : - val;
   }

   /**
    @ingroup core
    @brief test if 2 values are equal with a certain tolerance
   */
   template <class T>
   bool equal( const T var, const T val, const T accuracy = std::numeric_limits<T>::epsilon() )
   {
      return absolute( var - val ) <= accuracy;
   }

   /**
    @ingroup core
    @brief compute statically the size of an array defined like this: T array[] = { .... }
   */
	template <std::size_t N, class T> ui32 getStaticBufferSize(const T (& /*Elements*/)[N])
	{
		return N;
	}

   /**
    @ingroup core
    @brief compute a string representing this value
   */
   template <class T>
   std::string val2str( const T& val )
   {
      std::stringstream s;
      s << val;
      std::string res;
      s >> res;
      return res;
   }

   /**
    @ingroup core
    @brief compute a value representing a string
   */
   template <class T>
   T str2val( const std::string& val )
   {
      std::stringstream s;
      s << val;
      T res;
      s >> res;
      return res;
   }

   /**
    @ingroup core
    @brief split a string according to a specific separator.
    
     The input string is altered: each time the separator is found, it is replaced by a null character.
     Each entry of the returned vector point to a part of the string. If a separator is found at the beginning
     or at the end of the string, an empty string representing this entry will be added in the result.
    */
   inline std::vector<const char*> split( std::string& str, char separator = ' ' )
   {
      std::vector<const char*> s;
      unsigned last = 0;
      for ( unsigned n = 0; n < str.size(); ++n )
         if ( str[ n ] == separator )
         {
            str[ n ] = 0;
            s.push_back( &str[ last ] );
            last = n + 1;
         }
      s.push_back( &str[ last ] );
      return s;
   }
}
}

#endif
