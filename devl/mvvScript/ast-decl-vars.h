#ifndef MVV_PARSER_AST_DECL_VARS_H_
# define MVV_PARSER_AST_DECL_VARS_H_

# include "ast.h"
# include <deque>


namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstDeclVars : public Ast
   {
   public:
      typedef std::deque<AstDeclVar*>   Decls;

      AstDeclVars( const YYLTYPE& location ) : Ast( location )
      {
      }

      ~AstDeclVars()
      {
         for ( Decls::iterator i = _decls.begin(); i != _decls.end(); ++i )
            delete *i;
      }


      void insert( AstDeclVar* ast )
      {
         _decls.push_front( ast );
      }

      const Decls& getVars() const
      {
         return _decls;
      }

      Decls& getVars()
      {
         return _decls;
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
      Decls    _decls;
   };
}
}

#endif