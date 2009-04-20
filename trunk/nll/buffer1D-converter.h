#ifndef NLL_BUFFER1D_CONVERTER_H_
# define NLL_BUFFER1D_CONVERTER_H_

# include "buffer1D.h"
# include "utility.h"

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Convert a buffer from a specific mapper and type to another one. The values are truncated if needed.
    */
   template <class Ti, class Mapperi, class To, class Mappero>
   Buffer1D<To, Mappero> convert( const Buffer1D<Ti, Mapperi>& i )
   {
      Buffer1D<To, Mappero> o( i.size() );
      for ( ui32 n = 0; n < i.size(); ++n )
         o( n ) = static_cast<To> ( NLL_BOUND( i( n ), (To)Bound<To>::min, (To)Bound<To>::max ) );
      return o;
   }

   /**
    @ingroup core
    @brief Convert a buffer from a specific mapper and type to another one. The values are truncated if needed.
    */
   template <class Ti, class Mapperi, class To, class Mappero>
   void convert( const Buffer1D<Ti, Mapperi>& i, Buffer1D<To, Mappero>& o )
   {
      o = convert<Ti, Mapperi, To, Mappero>( i );
   }
}
}
#endif
