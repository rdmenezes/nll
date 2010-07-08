#include "symbol-table.h"

namespace mvv
{
namespace parser
{

   const AstTypeT* SymbolTableTypedef::find_in_scope( mvv::Symbol v ) const
   {
      for ( ui32 n = 0; n < _current->typedefs.size(); ++n )
      {
         if ( _current->typedefs[ n ]->getName() == v )
         {
            return &_current->typedefs[ n ]->getType();
         }
      }
      return 0;
   }

   AstTypedef* SymbolTableTypedef::find_typedef_in_scope( mvv::Symbol v )
   {
      for ( ui32 n = 0; n < _current->typedefs.size(); ++n )
      {
         if ( _current->typedefs[ n ]->getName() == v )
         {
            return _current->typedefs[ n ];
         }
      }
      return 0;
   }

   AstTypedef* SymbolTableTypedef::find( mvv::Symbol v )
   {
      Scope* c = _current;
      while ( c )
      {
         AstTypedef* t = find_typedef_in_scope( v );
         if ( t )
         {
            _current = c;
            return t;
         }
         _current = _current->pred;
      }
      _current = c;
      return 0;
   }
}
}