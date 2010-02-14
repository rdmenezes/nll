#ifndef MVV_PARSER_AST_STATEMENTS_H_
# define MVV_PARSER_AST_STATEMENTS_H_

# include "ast.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstStatements : public Ast
   {
   public:
      AstStatements( const YYLTYPE& location ) : Ast( location )
      {
      }

      const std::vector<Ast*>& getStatements() const
      {
         return _statements;
      }

      void push_back( Ast* ast )
      {
         _statements.push_back( ast );
      }

      /// Accept a const visitor \a v.
      virtual void accept( ConstVisitor& v ) const
      {
         v( *this );
      }

      /// Accept a non-const visitor \a v.
      virtual void accept( Visitor& v )
      {
         v( *this );
      }

   private:
      std::vector<Ast*> _statements;
   };
}
}

#endif