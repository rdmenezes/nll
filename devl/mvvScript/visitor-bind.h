#ifndef MVV_PARSE_BIND_VISITOR_H_
# define MVV_PARSE_BIND_VISITOR_H_

# include "visitor-default.h"
# include "parser-context.h"
# include "symbol-table.h"

namespace mvv
{
namespace parser
{
   /**
    @brief Defines a visitor that will reference all variable/function/class declaration, and
           link them when used. It will also report all binding errors.
    */
   class VisitorBind : public VisitorDefault
   {
   public:
      typedef std::set<mvv::Symbol> Symbols;

   public:
      VisitorBind( ParserContext& context ) : _context( context )
      {}

      virtual void operator()( AstExpAssign& e )
      {
         // TODO
         operator()( e.getLValue() );
         operator()( e.getValue() );
      }

      virtual void operator()( AstVarSimple& )
      {
         // TODO
      }

      virtual void operator()( AstVarArray& e )
      {
         // TODO
         operator()( e.getName() );
         operator()( e.getIndex() );
      }

      virtual void operator()( AstVarField& e )
      {
         // TODO
         operator()( e.getField() );
      }

      virtual void operator()( AstType& )
      {
         // TODO
      }

      virtual void operator()( AstDeclVar& e )
      {
         // TODO
         operator()( e.getType() );
         if ( e.getInit() )
         {
            operator()( *e.getInit() );
         }
      }

      virtual void operator()( AstDeclClass& e )
      {
         // TODO
         operator()( e.getDeclarations() );
      }

      virtual void operator()( AstDeclFun& e ) 
      {
         // TODO
         operator()( e.getType() );
         if ( e.getBody() )
         {
            operator()( *e.getBody() );
         }
      }

      virtual void operator()( AstExpCall& e )
      {
         // TODO
         operator()( e.getName() );
      }

      virtual void operator()( Ast& e )
      {
         e.accept( *this );
      }

      const Symbols& getFilesToInclude() const
      {
         return _filesToInclude;
      }

      const Symbols& getFilesToImport() const
      {
         return _filesToImport;
      }

   private:
      // disabled
      VisitorBind& operator=( const VisitorBind& );
      VisitorBind( const VisitorBind& );

   private:
      ParserContext&    _context;
      Symbols           _filesToInclude;
      Symbols           _filesToImport;

      SymbolTableVars      _vars;
      SymbolTableFuncs     _funcs;
      SymbolTableClasses   _classes;
   };
}
}

#endif