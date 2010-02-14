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
      typedef std::list<Ast*> Statements;
      AstStatements( const YYLTYPE& location ) : Ast( location )
      {
      }

      const Statements& getStatements() const
      {
         return _statements;
      }

      void insert( Ast* ast )
      {
         _statements.push_front( ast );
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
      Statements _statements;
   };
}
}

#endif