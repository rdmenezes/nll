#ifndef MVV_PARSE_REGISTER_DECLARATIONS_VISITOR_H_
# define MVV_PARSE_REGISTER_DECLARATIONS_VISITOR_H_

# include "visitor-default.h"
# include "parser-context.h"
# include "symbol-table.h"

namespace mvv
{
namespace parser
{
   namespace impl
   {
      inline void reportAlreadyDeclaredType( const YYLTYPE& previous, const YYLTYPE& current, mvv::parser::ParserContext& context, const std::string& msg )
      {
         std::stringstream ss;
         ss << current << msg <<" (see " << previous << ")" << std::endl;
         context.getError() << ss.str() << mvv::parser::Error::BIND;
      }
   }

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
      {}

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
         ++_scopeDepth;
         operator()( e.getDeclarations() );
         --_scopeDepth;
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
         // if body is undeclared, multiple function definitions can be delcared

         // TODO must check it's arguments for overloading...
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
            Ast* check = checkSymbol( e.getName() );
            if ( check )
            {
               impl::reportAlreadyDeclaredType( check->getLocation(), e.getLocation(), _context, "variable already declared" );
            } else {
               _vars.insert( e.getName(), &e );
               _symbolsUsed[ e.getName() ] = &e;
            }
         }
      }


   private:
      Ast* checkSymbol( const mvv::Symbol& s ) const
      {
         std::map<mvv::Symbol, Ast*>::const_iterator it = _symbolsUsed.find( s );
         if ( it != _symbolsUsed.end() )
         {
            return it->second;
         }
         return 0;
      }

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
      std::map<mvv::Symbol, Ast*>      _symbolsUsed;
   };
}
}

#endif