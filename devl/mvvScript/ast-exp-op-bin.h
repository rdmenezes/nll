#ifndef MVV_PARSER_AST_BIN_OP_H_
# define MVV_PARSER_AST_BIN_OP_H_

# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstOpBin : public AstExp
   {
   public:
      enum Op
      {
         GE,
         LE,
         NE,
         MINUS,
         PLUS,
         TIMES,
         DIVIDE,
         EQ,
         GT,
         LT,
         AND,
         OR
      };

   public:
      AstOpBin( const YYLTYPE& location, AstExp* left, AstExp* right, Op op ) : AstExp( location ), _left( left ), _right( right ), _op( op )
      {
         ensure( left && right, "must not be null" );
      }

      const AstExp& getLeft() const
      {
         return *_left;
      }

      AstExp& getLeft()
      {
         return *_left;
      }

      const AstExp& getRight() const
      {
         return *_right;
      }

      AstExp& getRight()
      {
         return *_right;
      }


      Op getOp() const
      {
         return _op;
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
      AstExp*  _left;
      AstExp*  _right;
      Op       _op;
   };
}
}

#endif