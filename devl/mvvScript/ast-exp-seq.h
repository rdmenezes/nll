#ifndef MVV_PARSER_AST_EXP_SEQ_H_
# define MVV_PARSER_AST_EXP_SEQ_H_

# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstExpSeq : public AstExp
   {
   public:
      AstExpSeq( const YYLTYPE& location, AstExp* exp ) : AstExp( location ), _exp( exp )
      {
         ensure( exp, "must not be null" );
      }

      const Ast& getExp() const
      {
         return *_exp;
      }

      Ast& getExp()
      {
         return *_exp;
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
      AstExp*  _exp;
   };
}
}

#endif