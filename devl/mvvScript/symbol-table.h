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
         /*
         _scopes.push_back( Symbols() );
         _barrier.push_back( false );
         */
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
            _barrier.pop_back();
            _scopes.pop_back();
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
         for ( i32 n = static_cast<int>( _scopes.size() ) - 1; n >= 0; --n )
         {
            Symbols::const_iterator ii = _scopes[ n ].find( s );
            if ( ii != _scopes[ n ].end() )
               return ii->second;
            if ( _barrier[ n ] )
               return 0;
         }
         return 0;
      }

      T* find( const Symbol& s )
      {
         for ( i32 n = static_cast<int>( _scopes.size() ) - 1; n >= 0; --n )
         {
            Symbols::iterator ii = _scopes[ n ].find( s );
            if ( ii != _scopes[ n ].end() )
               return ii->second;
            if ( _barrier[ n ] )
               return 0;
         }
         return 0;
      }

      const T* find_in_scope( const Symbol& s ) const
      {
         if ( !_scopes.size() )
            return 0;
         Symbols::const_iterator ii = _scopes[ _scopes.size() - 1 ].find( s );
         if ( ii != _scopes[ _scopes.size() - 1 ].end() )
               return ii->second;
         return 0;
      }

      T* find_in_scope( const Symbol& s )
      {
         if ( !_scopes.size() )
            return 0;
         Symbols::iterator ii = _scopes[ _scopes.size() - 1 ].find( s );
         if ( ii != _scopes[ _scopes.size() - 1 ].end() )
               return ii->second;
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

         AstDeclClass* find_in_class( const std::vector<mvv::Symbol>& classPath, const mvv::Symbol& s )
         {
            Node* res = _find( classPath, -1 );
            if ( !res )
               return 0;
            if ( res->name == s )
               return res->decl;
            for ( ui32 n = 0; n < res->next.size(); ++n )
            {
               if ( res->next[ n ]->name == s )
               {
                  return res->next[ n ]->decl;
               }
            }
            return 0;
         }

         AstDeclClass* find_within_scope( const std::vector<mvv::Symbol>& classPath, const mvv::Symbol& s )
         {
            Node* res = _find( classPath, -1 );
            if ( !res )
            {
               // path not found
               return 0;
            }
            if ( !res->previous )
            {
               // we are already looking in global scope
               return 0;
            }

            res = res->previous;
            while ( res->previous )
            {
               for ( ui32 n = 0; n < res->next.size(); ++n )
               {
                  if ( res->next[ n ]->name == s )
                  {
                     return res->next[ n ]->decl;
                  }
               }
               res = res->previous;
            }
            return 0;
         }

         /**
          Use the current position, find predecessors until global scope
          */
         // TODO check
         AstDeclClass* find_in_scope( const mvv::Symbol& s )
         {
            if ( s == name )
               return decl;
            if ( previous )
               return previous->find_in_scope( s );
            else
               return 0;
         }

         AstDeclClass* find( const std::vector<mvv::Symbol>& classPath, int begining )
         {
            Node* res = _find( classPath, begining );
            if ( res )
            {
               return res->decl;
            }
            return 0;
         }

         Node* _find( const std::vector<mvv::Symbol>& classPath, int begining )
         {
            if ( name == mvv::Symbol::create( "" ) || classPath[ begining ] == name )
            {
               if ( static_cast<int>( classPath.size() ) == begining + 1 )
                  return this;
               for ( ui32 n = 0; n < next.size(); ++n )
               {
                  Node* res = next[ n ]->_find( classPath, begining + 1 );
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

         Node* clone() const
         {
            Node* r = new Node( name, decl );
            for ( ui32 n = 0; n < next.size(); ++n )
            {
               Node* cp = next[ n ]->clone();
               cp->previous = r;
               r->next.push_back( cp );
            }
            return r;
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
         if ( !_root )
            return 0;
         return _root->find( s, -1 );
      }

      AstDeclClass* find_in_class( const std::vector<mvv::Symbol>& path, const mvv::Symbol& s )
      {
         if ( !_root )
            return 0;
         return _root->find_in_class( path, s );
      }


      AstDeclClass* find( const std::vector<mvv::Symbol>& s )
      {
         if ( !_root )
            return 0;
         return _root->find( s, -1 );
      }

      // find the class with full path, then go up to global scope to find the declaration
      AstDeclClass* find_within_scope( const std::vector<mvv::Symbol>& path, const mvv::Symbol& s )
      {
         if ( !_root )
            return 0;
         return _root->find_within_scope( path, s );
      }

      const AstDeclClass* find_in_scope( const mvv::Symbol& s ) const
      {
         if ( !_root )
            return 0;
         return _current->find_in_scope( s );
      }

      SymbolTableDictionary& operator=( const SymbolTableDictionary& t )
      {
         ensure( _current == _root, "you can't clone a table with a build in progress" );
         if ( _root )
         {
            _root->destroy();
         }

         if ( t._root )
         {
            _root = t._root->clone();
            _current = _root;
         } else {
            _root = 0;
            _current = 0;
         }
         return *this;
      }

      SymbolTableDictionary( const SymbolTableDictionary& t ) : _root( 0 ), _current( 0 )
      {
         operator=( t );
      }

   private:
      Node* _root;
      Node* _current;
   };

   typedef SymbolTable<AstDeclVar>                            SymbolTableVars;     /// Scoped symbol table
   typedef std::map<mvv::Symbol, std::vector<AstDeclFun*> >   SymbolTableFuncs;    /// We only need to store functions in global scope
   typedef SymbolTableDictionary                              SymbolTableClasses;  /// tree-like storage for classes
}
}

#endif