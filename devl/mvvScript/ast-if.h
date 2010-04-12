#ifndef MVV_PARSER_AST_IF_H_
# define MVV_PARSER_AST_IF_H_

# include "ast.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstIf : public Ast
   {
   public:
      AstIf( const YYLTYPE& location, AstExp* condition, Ast* thenexp, Ast* elseexp ) : Ast( location ), _condition( condition ), _then( thenexp ), _else( elseexp )
      {
         ensure( condition && thenexp, "can't be null" );
      }

      ~AstIf()
      {
         delete _condition;
         delete _then;
         delete _else;
      }

      const AstExp& getCondition() const
      {
         return *_condition;
      }

      AstExp& getCondition()
      {
         return *_condition;
      }

      const Ast& getThen() const
      {
         return *_then;
      }

      Ast& getThen()
      {
         return *_then;
      }

      const Ast* getElse() const
      {
         return _else;
      }

      Ast* getElse()
      {
         return _else;
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
      AstExp*  _condition;
      Ast*     _then;
      Ast*     _else;
   };
}
}

#endif