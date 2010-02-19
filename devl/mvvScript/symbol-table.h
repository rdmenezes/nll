#ifndef MVV_PARSER_SYMBOL_TABLE_H_
# define MVV_PARSER_SYMBOL_TABLE_H_

# include "ast-files.h"

namespace mvv
{
namespace parser
{
   struct Function
   {
      Function( mvv::Symbol& n, AstArgs* a ) : name( n ), args( a )
      {}

      mvv::Symbol name;
      AstArgs*    args;
   };

   /**
    @brief Customizable symbol table: search range can be limited, possible to not deconstruct the scopes.
           Store symbols as LIFO
    */
   template <class T, class Symbol = mvv::Symbol, bool RemoveScope = true>
   class SymbolTable
   {
   public:
      typedef std::map<Symbol, T*>        Symbols;
      typedef std::vector<Symbols>        Scopes;

   public:
      SymbolTable() : _currentScope( 0 )
      {
         // global scope
         _scopes.push_back( Symbols() );
         _barrier.push_back( false );
      }

      void beginScope( bool barrier = false )
      {
         if ( _currentScope == _scopes.size() )
         {
            _barrier.push_back( barrier );
            _scopes.push_back( Symbols() );
            ++_currentScope;
         }
      }

      void endScope()
      {
         if ( _currentScope == _scopes.size() && RemoveScope )
         {
            _barrier.push_back();
            _scopes.push_back();
            --_currentScope;
         }
      }

      void insert( const Symbol& name, T* value )
      {
         (*_scopes.rbegin())[ name ] = value;
      }

      /**
       @brief Find the symbols from the latest to the first scope. Returns 0 if not found.
       */
      const T* find( const Symbol& s ) const
      {
         for ( i32 n = static_cast<int>( _scopes.size() ) - 1; i >= 0; --i )
         {
            Symbols::const_iterator ii = _scopes[ n ]->find( s );
            if ( ii != _scopes[ n ]->end() )
               return ii->second;
            if ( _barrier[ n ] )
               return 0;
         }
         return 0;
      }

      T* find( const Symbol& s )
      {
         for ( i32 n = static_cast<int>( _scopes.size() ) - 1; i >= 0; --i )
         {
            Symbols::iterator ii = _scopes[ n ]->find( s );
            if ( ii != _scopes[ n ]->end() )
               return ii->second;
            if ( _barrier[ n ] )
               return 0;
         }
         return 0;
      }

   private:
      Scopes               _scopes;
      std::vector<bool>    _barrier;
      ui32                 _currentScope;
   };


   template <class T>
   struct SymbolTableDictionaryNode
   {
      SymbolTableDictionaryNode( const mvv::Symbol& n ) : name( n ), next( 0 ), previous( 0 )
      {}

      mvv::Symbol                name;
      SymbolTableDictionaryNode* next;
      SymbolTableDictionaryNode* previous;
      std::set<T*>               declarations;
   };

   template <class T>
   class SymbolTableDictionary
   {
      typedef SymbolTableDictionaryNode<T*>  Node;

   public:
      SymbolTableDictionary() : _node( 0 ), _current( 0 )
      {
      }

      void begin_scope( mvv::Symbol s )
      {
         Node* node = new Node( s );
         if ( _current )
         {
            _node
         } else {
         }
      }

   private:
      Node* _node;
      Node* _current;
   };

   typedef SymbolTable<AstDeclVar>              SymbolTableVars;
   typedef SymbolTable<AstDeclFun, Function>    SymbolTableFuncs;

   typedef SymbolTable<AstDeclClass>   SymbolTableClasses;

}
}

#endif