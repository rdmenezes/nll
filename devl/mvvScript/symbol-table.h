#ifndef MVV_PARSER_SYMBOL_TABLE_H_
# define MVV_PARSER_SYMBOL_TABLE_H_

# include "ast-files.h"
# include "utils2.h"

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

      std::set<T*> findMatch( const std::string& s ) const
      {
         std::set<T*> match;
         if ( _scopes.size() == 0 )
            return match;
         for ( ui32 n = 0; n < (ui32)_scopes.size(); ++n )
         {
            for ( Symbols::const_iterator it = _scopes[ n ].begin(); it != _scopes[ n ].end(); ++it )
            {
               if ( parser::isMatch( it->first, s ) )
                  match.insert( it->second );
            }
         }

         return match;
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
         typedef std::vector<AstTypedef*> Typedefs;

         Scope() : pred( 0 )
         {
         }

         Scope( Scope* p ) : pred( p )
         {
         }

         Typedefs                         typedefs;
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
         _updatePredecessors( _scopes, 0 );
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

      std::set<AstTypedef*> findMatch( const std::string& s ) const;

   private:
      // because we need to copy the data structures, we need to recursively run through all the scopes
      // and update the predecessor with the copied data pointers (and not the source!)
      void _updatePredecessors( Scope& current, Scope* pred )
      {
         current.pred = pred;
         for ( Scope::Iter it = current.scopes.begin(); it != current.scopes.end(); ++it )
         {
            _updatePredecessors( it->second, &current );
         }
      }

   private:
      Scope    _scopes;
      Scope*   _current;
   };

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

      std::set<AstDeclClass*> findMatch( const std::string& s ) const
      {
         std::set<AstDeclClass*> match;
         for ( ui32 n = 0; n < _scopes.scopes.size(); ++n )
         {
            if ( parser::isMatch( _scopes.scopes[ n ].name, s ) )
               match.insert( _scopes.scopes[ n ].decl );
         }

         return match;
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

   inline static std::set<AstDeclFun*> findMatch( const SymbolTableFuncs& funs, const std::string& s )
   {
      std::set<AstDeclFun*> match;
      for ( std::map<mvv::Symbol, FunctionTable>::const_iterator it = funs.begin(); it != funs.end(); ++it )
      {
         if ( parser::isMatch( it->first, s ) )
         {
            for ( ui32 n = 0; n < it->second.list.size(); ++n )
            {
               match.insert( it->second.list[ n ] );
            }
         }
      }

      return match;
   }
}
}

#endif