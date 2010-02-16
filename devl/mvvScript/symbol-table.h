#ifndef MVV_PARSER_SYMBOL_TABLE_H_
# define MVV_PARSER_SYMBOL_TABLE_H_

# include "ast-files.h"

namespace mvv
{
namespace parser
{
   template <class T>
   class SymbolTable
   {
   public:
      typedef std::map<mvv::Symbol, T*>   Symbols;
      typedef std::list<Symbols>          Scopes;

   public:
      SymbolTable()
      {
         // global scope
         _scopes.push_back( Symbols() );
      }

      void beginScope()
      {
         _scopes.push_back( Symbols() );
      }

      void endScope()
      {
         _scopes.pop_back();
      }

      void insert( const mvv::Symbol& name, T* value )
      {
         (*_scopes.rbegin())[ name ] = value;
      }

      /**
       @brief Find the symbols from the latest to the first scope. Returns 0 if not found.
       */
      const T* find( const mvv::Symbol& s ) const
      {
         for ( Scopes::const_reverse_iterator it = _scopes.rbegin(); it != _scopes.rend(); ++it )
         {
            Symbols::const_iterator ii = it->find( s );
            if ( ii != it->end() )
               return ii->second;
         }
         return 0;
      }

      T* find( const mvv::Symbol& s )
      {
         for ( Scopes::reverse_iterator it = _scopes.rbegin(); it != _scopes.rend(); ++it )
         {
            Symbols::iterator ii = it->find( s );
            if ( ii != it->end() )
               return ii->second;
         }
         return 0;
      }

   private:
      Scopes   _scopes;
   };

   typedef SymbolTable<AstDeclVar>     SymbolTableVars;
   typedef SymbolTable<AstDeclFun>     SymbolTableFuncs;
   typedef SymbolTable<AstDeclClass>   SymbolTableClasses;

}
}

#endif