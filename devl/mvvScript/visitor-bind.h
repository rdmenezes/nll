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