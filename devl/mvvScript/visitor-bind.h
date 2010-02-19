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
    @brief Defines will link all variable/function/class declaration to where they are used.
           It will also report all binding errors.

           However, The bind visitor will need to register the simple variable & function decl in scopes level > 1 (not global),
           as well as arguments in function declarations
           Additionally, untyped variable declaration ( i.e. n = 3.0; ), the bind visitor must register them if they are not declared, but issue a
           warning if it has been declared in another file

           Note: unnamed varaible can't be created in expresions, if, ... Just by var = exp

           TODO: check for function/class if they are in LUT, else they have been created in a scope...
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
   };
}
}

#endif