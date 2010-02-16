#ifndef MVV_PARSER_AST_ARGS_H_
# define MVV_PARSER_AST_ARGS_H_

# include "ast-args.h"


namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstArgs : public Ast
   {
   public:
      typedef std::list<AstExp*>   Args;

      AstArgs( const YYLTYPE& location ) : Ast( location )
      {
      }

      void insert( AstExp* ast )
      {
         ensure( ast, "can't be null" );
         _args.push_front( ast );
      }

      const Args& getArgs() const
      {
         return _args;
      }

      Args& getArgs()
      {
         return _args;
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
      Args    _args;
   };
}
}

#endif