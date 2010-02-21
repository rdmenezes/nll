#ifndef MVV_PARSER_SYMBOL_TABLE_H_
# define MVV_PARSER_SYMBOL_TABLE_H_

# include "ast-files.h"

namespace mvv
{
namespace parser
{
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


   

   class SymbolTableDictionary
   {
      struct Node
      {
         Node( const mvv::Symbol& n, AstDeclClass* d ) : name( n ), previous( 0 ), decl( d )
         {}

         AstDeclClass* find( const std::vector<mvv::Symbol>& classPath, int begining )
         {
            if ( name == mvv::Symbol::create( "" ) || classPath[ begining ] == name )
            {
               if ( static_cast<int>( classPath.size() ) == begining + 1 )
                  return decl;
               for ( ui32 n = 0; n < next.size(); ++n )
               {
                  AstDeclClass* res = next[ n ]->find( classPath, begining + 1 );
                  if ( res )
                  {
                     return res;
                  }
               }
            }
            return 0;
         }

         void destroy()
         {
            for ( ui32 n = 0; n < next.size(); ++n )
            {
               next[ n ]->destroy();
               delete next[ n ];
            }
         }

         mvv::Symbol          name;
         std::vector<Node*>   next;
         Node*                previous;
         AstDeclClass*        decl;
      };

   public:
      SymbolTableDictionary() : _root( 0 ), _current( 0 )
      {
         // create the global scope
         begin_scope( mvv::Symbol::create( "" ), 0 );
      }

      ~SymbolTableDictionary()
      {
         if ( _root )
         {
            _root->destroy();
            delete _root;
            _root = 0;
            _current = 0;
         }
      }

      void begin_scope( mvv::Symbol s, AstDeclClass* decl )
      {
         ensure( decl || _current == 0, "can't insert a null decl except the first one" );

         Node* node = new Node( s, decl );
         if ( _current )
         {
            _current->next.push_back( node );
            node->previous = _current;
            _current = node;
         } else {
            _current = node;
            _root = node;
         }
      }

      void end_scope()
      {
         ensure( _current, "can't be null: poped too much..." );
         _current = _current->previous;
      }

      const AstDeclClass* find( const std::vector<mvv::Symbol>& s ) const
      {
         if ( !_current )
            return 0;
         return _current->find( s, -1 );
      }

   private:
      // copy disabled
      SymbolTableDictionary( const SymbolTableDictionary& );
      SymbolTableDictionary& operator=( const SymbolTableDictionary& );

   private:
      Node* _root;
      Node* _current;
   };

   typedef SymbolTable<AstDeclVar>              SymbolTableVars;     /// Scoped symbol table
   typedef std::map<mvv::Symbol, AstDeclFun*>   SymbolTableFuncs;    /// We only need to store functions in global scope
   typedef SymbolTable<AstDeclClass>            SymbolTableClasses;  /// tree-like storage for classes
}
}

#endif