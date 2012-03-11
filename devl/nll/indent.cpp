#include <iomanip>
#include <ostream>
#include "nll.h"

namespace nll
{
namespace core
{
   /// The slot to store the current indentation level.
   static long int indent_index = std::ios::xalloc();

   std::ostream& incindent( std::ostream& o )
   {
      o.iword(indent_index) += 2;
      return o;
   }

   std::ostream& decindent( std::ostream& o )
   {
      ensure(o.iword( indent_index ), "indentation error" );
      o.iword( indent_index ) -= 2;
      return o;
   }

   std::ostream& resetindent( std::ostream& o )
   {
      o.iword( indent_index ) = 0;
      return o;
   }

   std::ostream& iendl( std::ostream& o )
   {
      o << std::endl;
      // Be sure to be able to restore the stream flags.
      char fill = o.fill(' ');
      return o << std::setw( o.iword( indent_index ) ) << "" << std::setfill( fill );
   }

   std::ostream& incendl( std::ostream& o )
   {
      return o << incindent << iendl;
   }

   std::ostream& decendl( std::ostream& o )
   {
      return o << decindent << iendl;
   }
}
}
