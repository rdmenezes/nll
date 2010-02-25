#ifndef MVV_PARSE_REGISTER_DECLARATIONS_VISITOR_H_
# define MVV_PARSE_REGISTER_DECLARATIONS_VISITOR_H_

# include "visitor-default.h"
# include "parser-context.h"
# include "symbol-table.h"
# include "utils.h"

namespace mvv
{
namespace parser
{
   /**
    @brief Defines a visitor that will reference all global definitions:
           _global_ variable/ _global_ function/ all class declaration.

           It will report any redefinition errors.
    
           We need this as we need to parse first other files that might declare global variables,
           functions and classes. Also the class life's scope is different from functions and variables as they
           need to remain in the look up table after traversal....
    */
   class VisitorRegisterDeclarations : public VisitorDefault
   {
   public:
      typedef std::set<mvv::Symbol> Symbols;
      using VisitorDefault::operator();

   public:
      VisitorRegisterDeclarations( ParserContext& context ) : _context( context ), _scopeDepth( 0 )
      {
         // global scope
         _vars.beginScope();
      }

      const Symbols& getFilesToInclude() const
      {
         return _filesToInclude;
      }

      const Symbols& getFilesToImport() const
      {
         return _filesToImport;
      }

      virtual void operator()( AstStatements& e )
      {
         // visit only global scope
         if ( _scopeDepth == 0 )
         {
            ++_scopeDepth;
             for ( AstStatements::Statements::iterator it = e.getStatements().begin();
               it != e.getStatements().end();
               ++it )
             {
                operator()( **it );
             }
            --_scopeDepth;
         }
      }

      virtual void operator()( AstDeclClass& e )
      {
         if ( _vars.find( e.getName() ) )
         {
            // we know we have at least 1 function declared...
            impl::reportAlreadyDeclaredType( _vars.find( e.getName() )->getLocation(), e.getLocation(), _context, "a variable has already been declared with this name" );
            operator()( e.getDeclarations() );
            return;
         }

         SymbolTableFuncs::iterator it = _funcs.find( e.getName() );
         if ( it != _funcs.end() )
         {
            // we know we have at least 1 function declared...
            impl::reportAlreadyDeclaredType( it->second[ 0 ]->getLocation(), e.getLocation(), _context, "a function has already been declared with this name" );
            operator()( e.getDeclarations() );
            return;
         }

         const AstDeclClass* decl = _classes.find_in_scope( e.getName() );
         if ( decl )
         {
            impl::reportAlreadyDeclaredType( decl->getLocation(), e.getLocation(), _context, "a class has already been declared with this name" );
            operator()( e.getDeclarations() );
            return;
         } 

         // no error
         ++_scopeDepth;
         _classes.begin_scope( e.getName(), &e );
         operator()( e.getDeclarations() );
         --_scopeDepth;
         _classes.end_scope();
      }

      /**
       TODO for node
       */


      virtual void operator()( AstIf& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }

      virtual void operator()( const AstVarArray& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }

      virtual void operator()( const AstVarField& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }

      virtual void operator()( AstArgs& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }

      virtual void operator()( AstReturn& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }

      virtual void operator()( AstExpCall& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }

      virtual void operator()( AstExpTypename& )
      {
         // nothing to do: just don't go through these nodes, we don't have to!
      }


      virtual void operator()( AstDeclFun& e ) 
      {
         // discard type and body
         // if body is undeclared, it can't be multiple times declared...

         if ( _scopeDepth == 1 )
         {
            const AstDeclVar* var = _vars.find( e.getName() );
            if ( var )
            {
               impl::reportAlreadyDeclaredType( var->getLocation(), e.getLocation(), _context, "a variable has already been declared with this name" );
               return;
            }
            const AstDeclClass* decl = _classes.find( nll::core::make_vector<mvv::Symbol>( e.getName() ) );
            if ( decl )
            {
               impl::reportAlreadyDeclaredType( decl->getLocation(), e.getLocation(), _context, "a class has already been declared with this name" );
               return;
            }

            // it means it is a global function. Else it is a member function.
            // in the case where it is declared outside of a class/global scope, the declaration
            // will not be created (nodes are not visited) and a binding error will be issued
            SymbolTableFuncs::iterator it = _funcs.find( e.getName() );
            if ( it == _funcs.end() )
            {
               _funcs[ e.getName() ].push_back( &e );
            } else {
               for ( std::vector<AstDeclFun*>::iterator ii = it->second.begin();
                     ii != it->second.end();
                     ++ii )
               {
                  if ( areDeclVarsEqual( &(*ii)->getVars(), &e.getVars() ) )
                  {
                     impl::reportAlreadyDeclaredType( (*ii)->getLocation(), e.getLocation(), _context, "a function with the same prototype has already been declared" );
                  }
               }

               it->second.push_back( &e );
            }
         }
      }

      virtual void operator()( AstImport& e )
      {
         _filesToImport.insert( e.getStr() );
      }

      virtual void operator()( AstInclude& e )
      {
         _filesToInclude.insert( e.getStr() );
      }

      virtual void operator()( AstDeclVar& e )
      {
         // if global scope, add it to the symbol table
         if ( _scopeDepth == 1 )
         {
            const AstDeclClass* decl = _classes.find_in_scope( e.getName() );
            if ( decl )
            {
               impl::reportAlreadyDeclaredType( decl->getLocation(), e.getLocation(), _context, "a class has already been declared with this name" );
               return;
            }

            const AstDeclClass* decl2 = _classes.find( nll::core::make_vector<mvv::Symbol>( e.getName() ) );
            if ( decl2 )
            {
               impl::reportAlreadyDeclaredType( decl2->getLocation(), e.getLocation(), _context, "a class has already been declared with this name" );
               return;
            }

            SymbolTableFuncs::iterator it = _funcs.find( e.getName() );
            if ( it != _funcs.end() )
            {
               // we know we have at lest 1 function of this name declared
               impl::reportAlreadyDeclaredType( it->second[ 0 ]->getLocation(), e.getLocation(), _context, "a function has already been declared with this name" );
               return;
            }

            if ( _vars.find( e.getName() ) )
            {
               impl::reportAlreadyDeclaredType( _vars.find( e.getName() )->getLocation(), e.getLocation(), _context, "a variable has already been declared with this name" );
            } else {
               _vars.insert( e.getName(), &e );
            }
         }
      }

      const SymbolTableVars& getVars() const
      {
         return _vars;
      }

      const SymbolTableFuncs& getFuncs() const
      {
         return _funcs;
      }

      const SymbolTableClasses& getClasses() const
      {
         return _classes;
      }

   private:

      // disabled
      VisitorRegisterDeclarations& operator=( const VisitorRegisterDeclarations& );
      VisitorRegisterDeclarations( const VisitorRegisterDeclarations& );

   private:
      ParserContext&    _context;
      Symbols           _filesToInclude;
      Symbols           _filesToImport;
      int               _scopeDepth;

      SymbolTableVars                  _vars;
      SymbolTableFuncs                 _funcs;
      SymbolTableClasses               _classes;
   };
}
}

#endif