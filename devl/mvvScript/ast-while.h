#ifndef MVV_PARSER_AST_WHILE_H_
# define MVV_PARSER_AST_WHILE_H_

namespace mvv
{
namespace parser
{
   class AstWhile : public Ast
   {
   public:
      AstWhile( const YYLTYPE& location, AstExp* cond, AstStatements* statements ) : Ast( location ), _cond( cond ), _statements( statements )
      {
         assert( cond && statements ); // can't be null
      }

      virtual ~AstWhile()
      {
         delete _cond;
         delete _statements;
      }

      AstExp& getCondition()
      {
         return *_cond;
      }

      AstStatements& getStatements()
      {
         return *_statements;
      }

      const AstExp& getCondition() const
      {
         return *_cond;
      }

      const AstStatements& getStatements() const
      {
         return *_statements;
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
      AstExp*        _cond;
      AstStatements* _statements;
   };
}
}

#endif