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
      while ( _current )
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

   AstTypedef* SymbolTableTypedef::findExact( const std::vector<mvv::Symbol>& v )
   {
      Scope* c = &_scopes;
      for ( ui32 n = 0; n < v.size(); ++n )
      {
         if ( n + 1 == v.size() )
         {
            for ( ui32 nn = 0; nn < c->typedefs.size(); ++nn )
               if ( c->typedefs[ nn ]->getName() == v[ n ] )
                  return c->typedefs[ nn ];
            return 0;
         } else {
            // find the next scope
            Scope::Iter it = c->scopes.find( v[ n ] );
            if ( it == c->scopes.end() )
               return 0;
         }
      }
      return 0;
   }

   const AstTypedef* SymbolTableTypedef::findExact( const std::vector<mvv::Symbol>& v ) const
   {
      Scope* c = const_cast<Scope*>( &_scopes );      // we guarantee there will be no change!
      for ( ui32 n = 0; n < v.size(); ++n )
      {
         if ( n + 1 == v.size() )
         {
            for ( ui32 nn = 0; nn < c->typedefs.size(); ++nn )
               if ( c->typedefs[ nn ]->getName() == v[ n ] )
                  return c->typedefs[ nn ];
            return 0;
         } else {
            // find the next scope
            Scope::Iter it = c->scopes.find( v[ n ] );
            if ( it == c->scopes.end() )
               return 0;
         }
      }
      return 0;
   }

   const AstDeclClass* SymbolTableClasses::_find( const std::vector<mvv::Symbol>& s, const SymbolTableTypedef& typedefs ) const
   {
      Scope* current = const_cast<Scope*>( &_scopes );   // we guarantee there will be no change!
      std::vector<mvv::Symbol> symbolsMatched;
      for ( ui32 n = 0; n < s.size(); ++n )
      {
         // check if it is a real class...
         bool found = false;
         for ( ui32 nn = 0; nn < current->scopes.size(); ++nn )
         {
            if ( current->scopes[ nn ].name == s[ n ] )
            {
               // found it!
               current = &current->scopes[ nn ];
               symbolsMatched.push_back( s[ n ] );
               found = true;
               break;
            } else {
               symbolsMatched.push_back( s[ n ] );
               const AstTypedef* match = typedefs.findExact( symbolsMatched );
               if ( !match )
                  continue;
               // we found a typedef: get the actual class
               AstDecl* decl = match->getType().getReference();
               if ( !decl )
                  return 0;
               //ensure( decl, "compiler error: there must be a reference!" );
               AstDeclClass* declc = dynamic_cast<AstDeclClass*>( decl );
               ensure( declc, "compiler error: it must be a class declaration" );
               current = const_cast<Scope*>( _findNoTypedef( declc->getAccessPath() ) );
               ensure( declc, "compiler error: we must be able to find the typedef declaration!" );
               found = true;
               break;
            }
         }

         if ( !found )
            return 0;
      }

      if ( current )
      {
         return current->decl;
      }

      return 0;
   }
}
}