#ifndef MVV_PARSER_AST_BREAK_H_
# define MVV_PARSER_AST_BREAK_H_

namespace mvv
{
namespace parser
{
   class AstBreak : public Ast
   {
   public:
      AstBreak( const YYLTYPE& location ) : Ast( location ), _loop( 0 )
      {
      }

      void setLoop( AstWhile* loop )
      {
         _loop = loop;
      }

      AstWhile* getLoop() const
      {
         return _loop;
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
      AstWhile*   _loop;   // the loop that needs to be stoped
   };
}
}

#endif