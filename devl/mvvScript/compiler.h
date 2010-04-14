#ifndef MVV_PARSER_COMPILER_H_
# define MVV_PARSER_COMPILER_H_

# include "mvvScript.h"
# include "symbol-table.h"
# include "parser-context.h"
# include "visitor-print.h"
# include "visitor-default.h"
# include "visitor-bind.h"
# include "visitor-register-declarations.h"
# include "visitor-type.h"
# include "visitor-evaluate.h"

namespace mvv
{
namespace parser
{
   /**
    @brief Front end for the compiler & interpreter
    */
   class MVVSCRIPT_API CompilerFrontEnd
   {
   public:
      typedef std::vector< platform::RefcountedTyped<Ast> > Trees;

   public:
      /**
       @brief construct the frontend
       @param parseTrace if true, the parsing trace will be displayed on std::cout
       @param scanTrace if true, the tokens will be displayed on std::cout
       */
      CompilerFrontEnd( bool parseTrace = false, bool scanTrace = false ) : _context( parseTrace, scanTrace )
      {
      }

      /**
       @brief Clear the front end as if it just has been created
       */
      void clear()
      {
         _context.clear();
         _vars.clear();
         _funcs.clear();
         _classes.clear();
         _executionTrees.clear();
      }

      /**
       @brief Parse the string.
       @throw RuntimeException when the compiler fails execute incorrectly (i.e. out of bounds, dangling reference...)
       @note variables, functions & classes declared in this string are saved in the current context and available
             for later usage.
       */
      Error::ErrorType run( const std::string& s )
      {
         Ast* exp = _context.parseString( s );
         _executionTrees.push_back( platform::RefcountedTyped<Ast>( exp ) );

         if ( exp )
         {
            VisitorRegisterDeclarations visitor( _context, _vars, _funcs, _classes );
            visitor( *exp );
            if ( !_context.getError().getStatus() )
            {
               VisitorBind visitorBind( _context, _vars, _funcs, _classes );
               visitorBind( *exp );
               if ( !_context.getError().getStatus() )
               {
                  VisitorType visitorType( _context, _vars, _funcs, _classes );
                  visitorType( *exp );
                  if ( !_context.getError().getStatus() )
                  {
                     VisitorEvaluate visitorEvaluate( _context, _vars, _funcs, _classes );
                     visitorEvaluate( *exp );
                  }
               }
            }
         }

         return _context.getError().getStatus();
      }

      /**
       @brief find a global variable in the execution context
       @throw std::exception if the varaible can't be found
       @return it's runtime value & type
       */
      const RuntimeValue& getVariable( const mvv::Symbol& name ) const
      {
         const AstDeclVar* val = _vars.find( name );
         if ( !val )
         {
            throw std::exception("can't find this variable in the current execution context" );
         }
         return val->getRuntimeValue();
      }


   private:
      ParserContext       _context; // parsing context
      SymbolTableVars     _vars;    // current list of variable definition
      SymbolTableFuncs    _funcs;   // current list of functions definition
      SymbolTableClasses  _classes; // current list of class definition

      Trees               _executionTrees;   // the trees that have been parsed
   };
}
}

#endif