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

      void clear()
      {
         _scopes.clear();
         _barrier.clear();
         _currentScope = 0;
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
       @brief Find the symbols from the latest to the first scope/or barrier. Returns 0 if not found.
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

   class MVVSCRIPT_API SymbolTableTypedef
   {
      struct Scope
      {
         typedef std::map<mvv::Symbol, Scope>::iterator  Iter;

         Scope() : pred( 0 )
         {
         }

         Scope( Scope* p ) : pred( p )
         {
         }

         std::vector<AstTypedef*>         typedefs;
         std::map<mvv::Symbol, Scope>     scopes;
         Scope*                           pred;
      };

   public:
      SymbolTableTypedef() : _scopes( Scope( 0 ) )
      {
         _current = &_scopes;
      }

      void begin_scope( mvv::Symbol s )
      {
         Scope::Iter iter = _current->scopes.find( s );
         if ( iter == _current->scopes.end() )
         {
            _current->scopes[ s ] = Scope( _current );
         }
         
         _current = &_current->scopes[ s ];
      }

      void end_scope()
      {
         ensure( _current, "error" );
         ensure( _current->pred, "no predecessor!!!" );

         _current = _current->pred;
      }

      void insert( AstTypedef* val )
      {
         _current->typedefs.push_back( val );
      }

      const AstTypeT* find_in_scope( mvv::Symbol v ) const;

      AstTypedef* find_typedef_in_scope( mvv::Symbol v );

      AstTypedef* find( mvv::Symbol v );

      // find the exact match of the symbols, assuming v0.. v(n-1) is a class and v(n) is a typedef, returns typedef v(n)
      AstTypedef* findExact( const std::vector<mvv::Symbol>& v );
      const AstTypedef* findExact( const std::vector<mvv::Symbol>& v ) const;

      SymbolTableTypedef& operator=( const SymbolTableTypedef& cpy )
      {
         _scopes = cpy._scopes;
         _current = &_scopes;
         return *this;
      }

      SymbolTableTypedef( const SymbolTableTypedef& cpy )
      {
         operator=( cpy );
      }
      
      void resetScope()
      {
         _current = &_scopes;
      }

      void clear()
      {
         _scopes = Scope();
         resetScope();
      }

   private:
      Scope    _scopes;
      Scope*   _current;
   };


   /*
   template <class T>
   class SymbolTableDictionary
   {
      struct Node
      {
         Node( const mvv::Symbol& n, T* d ) : name( n ), previous( 0 ), decl( d )
         {}

         // goto the classpath specified and find s in this scope only
         T* find_in_class( const std::vector<mvv::Symbol>& classPath, const mvv::Symbol& s )
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

         T* find_within_scope( const std::vector<mvv::Symbol>& classPath, const mvv::Symbol& s )
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

         //
         // Use the current position, find predecessors until global scope
         //
         T* find_in_scope( const mvv::Symbol& s )
         {
            if ( s == name )
               return decl;
            if ( previous )
               return previous->find_in_scope( s );
            else
               return 0;
         }

         // find the class with full path, then go up, find full fieldpath, else go up and again until global scope to find the declaration
         T* find_within_scope( const std::vector<mvv::Symbol>& path, const std::vector<mvv::Symbol>& fieldpath, const SymbolTableTypedef& typedefs )
         {
            Node* res = _find( path, -1 );
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
               Node* final = res->_find( fieldpath, 0 );
               if ( final )
                  return final->decl;
               res = res->previous;
            }
            return 0;
         }

         T* find( const std::vector<mvv::Symbol>& classPath, int begining )
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
            SymbolTableTypedef f;
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
         T*        decl;
      };

   public:
      SymbolTableDictionary() : _root( 0 ), _current( 0 )
      {
         // create the global scope
         begin_scope( mvv::Symbol::create( "" ), 0 );
      }

      ~SymbolTableDictionary()
      {
         clear();
      }

      void clear()
      {
         if ( _root )
         {
            _root->destroy();
            delete _root;
            _root = 0;
            _current = 0;
         }
      }

      void begin_scope( mvv::Symbol s, T* decl )
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

      const T* find( const std::vector<mvv::Symbol>& s ) const
      {
         if ( !_root )
            return 0;
         return _root->find( s, -1 );
      }

      T* find_in_class( const std::vector<mvv::Symbol>& path, const mvv::Symbol& s )
      {
         if ( !_root )
            return 0;
         return _root->find_in_class( path, s );
      }


      T* find( const std::vector<mvv::Symbol>& s )
      {
         if ( !_root )
            return 0;
         return _root->find( s, -1 );
      }

      // find the class with full path, then go up to global scope to find the declaration
      T* find_within_scope( const std::vector<mvv::Symbol>& path, const mvv::Symbol& s )
      {
         if ( !_root )
            return 0;
         return _root->find_within_scope( path, s );
      }

      // find the class with full path, then go up, find full fieldpath, else go up and again until global scope to find the declaration
      // the typedefs arg is used to look up field, in case one of the field is actually a typedef...
      T* find_within_scope( const std::vector<mvv::Symbol>& path, const std::vector<mvv::Symbol>& fieldpath, const SymbolTableTypedef& typedefs )
      {
         if ( !_root )
            return 0;
         return _root->find_within_scope( path, fieldpath, typedefs );
      }

      const T* find_in_scope( const mvv::Symbol& s ) const
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
   };*/

   // we handle separately the typedef and class definition
   // the typedef and class tables are created independently
   // the look up of typedef is independent
   // HOWEVER lookup of a classpath is dependent of the typedefs
   class MVVSCRIPT_API SymbolTableClasses
   {
   public:
      struct Scope
      {
         Scope() : name( mvv::Symbol::create( "" ) ), decl( 0 ), pred( 0 )
         {
         }

         Scope( mvv::Symbol s, AstDeclClass* d, Scope* p ) : name( s ), decl( d ), pred( p )
         {
         }

         mvv::Symbol                      name;
         AstDeclClass*                    decl;
         Scope*                           pred;
         std::vector<Scope>               scopes;
      };

      SymbolTableClasses()
      {
         // create the global scope with name ""
         resetScope();
      }

      // create a new scope from the current position
      void begin_scope( mvv::Symbol s, AstDeclClass* d )
      {
         assert( _current );
         _current->scopes.push_back( Scope( s, d, _current ) );
         _current = &_current->scopes[ _current->scopes.size() - 1 ];
      }

      void end_scope()
      {
         assert( _current && _current->pred );
         //if ( _current->pred )
         {
            _current = _current->pred;
         }
      }

      void clear()
      {
         _scopes = Scope();
         resetScope();
      }

      // find a match for, with the full list of symbols
      AstDecl* find( const std::vector<mvv::Symbol>& s, const SymbolTableTypedef& typedefs )
      {
         return const_cast<AstDecl*>( _find( s, typedefs ) );
      }
      

      const AstDecl* find( const std::vector<mvv::Symbol>& s, const SymbolTableTypedef& typedefs ) const
      {
         return _find( s, typedefs );
      }

      // find a match for, with the full list of symbols
      const AstDecl* _find( const std::vector<mvv::Symbol>& s, const SymbolTableTypedef& typedefs ) const;
      static const AstDeclClass* _findClassFromTypedef( const AstTypedef* t );


      // returns the path of a class, there must be _NO_ typedef in the path
      const Scope* _findNoTypedef( const std::vector<mvv::Symbol>& s ) const
      {
         Scope* current = const_cast<Scope*>( &_scopes );   // we guarantee there will be no change!

         for ( ui32 n = 0; n < s.size(); ++n )
         {
            bool found = false;
            for ( ui32 nn = 0; nn < current->scopes.size(); ++nn )
            {
               if ( current->scopes[ nn ].name == s[ n ] )
               {
                  // found it!
                  current = &current->scopes[ nn ];
                  found = true;
                  break;
               }
            }
            if ( !found )
               return 0;
         }
         return current;
      }

      Scope* _findNoTypedef( const std::vector<mvv::Symbol>& s )
      {
         Scope* current = &_scopes;

         for ( ui32 n = 0; n < s.size(); ++n )
         {
            bool found = false;
            for ( ui32 nn = 0; nn < current->scopes.size(); ++nn )
            {
               if ( current->scopes[ nn ].name == s[ n ] )
               {
                  // found it!
                  current = &current->scopes[ nn ];
                  found = true;
                  break;
               }
            }
            if ( !found )
               return 0;
         }
         return current;
      }


      // find a declaration in the class: it will look in the class for the full field, then the superclass and so on until global scope included
      AstDecl* find_within_scope( const std::vector<mvv::Symbol>& path, const std::vector<mvv::Symbol>& field, const SymbolTableTypedef& typedefs )
      {
         for ( ui32 nn = 0; nn < path.size(); ++nn )
         {
            std::vector<mvv::Symbol> path2( path.size() - nn );
            for ( ui32 n = 0; n < path2.size(); ++n )
               path2[ n ] = path[ n ];
            for ( ui32 n = 0; n < field.size(); ++n )
               path2.push_back( field[ n ] );
            AstDecl* c = find( path2, typedefs );
            if ( c )
               return c;
         }

         // check global scope
         return find( field, typedefs );
      }

      // find a symbol in the class given by the full path
      AstDecl* find_in_class( const std::vector<mvv::Symbol>& path, const mvv::Symbol& field, const SymbolTableTypedef& typedefs )
      {
         std::vector<mvv::Symbol> path2 = path;
         path2.push_back( field );
         return find( path2, typedefs );
      }

      // reinit the current scope to the root
      void resetScope()
      {
         _current = &_scopes;
      }

      SymbolTableClasses& operator=( const SymbolTableClasses& cpy )
      {
         _scopes = cpy._scopes;
         resetScope();
         return *this;
      }

      SymbolTableClasses( const SymbolTableClasses& cpy )
      {
         operator=( cpy );
      }

   private:
      Scope    _scopes;
      Scope*   _current;
   };

   struct FunctionTable
   {
      FunctionTable()
      {
         hasImplementation = false;
      }

      std::vector<AstDeclFun*>   list;
      bool hasImplementation;
   };

   typedef SymbolTable<AstDeclVar>                            SymbolTableVars;     /// Scoped symbol table
   typedef std::map<mvv::Symbol, FunctionTable>               SymbolTableFuncs;    /// We only need to store functions in global scope
}
}

#endif