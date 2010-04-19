#ifndef MVV_PARSER_AST_BIN_OP_H_
# define MVV_PARSER_AST_BIN_OP_H_

# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   // TODO: we need to annotate with what function to use!
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
      AstOpBin( const YYLTYPE& location, AstExp* left, AstExp* right, Op op ) : AstExp( location ), _left( left ), _right( right ), _op( op ), _fn( 0 ), _isBinOpInClass( false )
      {
         ensure( left && right, "must not be null" );
      }

      ~AstOpBin()
      {
         delete _left;
         delete _right;
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

      /**
       the operators are in fact all shortcuts to global functions/member functions, except operator== and operator!= which are a specific case
       */
      void setFunctionCall( AstDeclFun* fn )
      {
         _fn = fn;
      }

      AstDeclFun* getFunctionCall() const
      {
         return _fn;
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

      void setIsBinOpInClass( bool val )
      {
         _isBinOpInClass = val;
      }

      bool isBinOpInClass() const
      {
         return _isBinOpInClass;
      }


   private:
      AstExp*     _left;
      AstExp*     _right;
      Op          _op;
      AstDeclFun* _fn;
      bool        _isBinOpInClass;     // true if the Binary operator is declared in the class

   };
}
}

#endif