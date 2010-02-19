#ifndef MVV_PARSER_AST_DECL_FUN_H_
# define MVV_PARSER_AST_DECL_FUN_H_

# include "typable.h"
# include "ast-exp.h"
# include "ast-decl.h"
# include "ast-type.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstDeclFun : public AstDecl
   {
   public:
      AstDeclFun( const YYLTYPE& location, AstTypeT* type, const mvv::Symbol& name, AstDeclVars* vars, AstStatements * body = 0 ) : AstDecl( location ), _type( type ), _name( name ), _vars( vars ), _body( body )
      {
         ensure( vars, "can't be null" );
      }

      const mvv::Symbol& getName() const
      {
         return _name;
      }

      AstTypeT* getType() const
      {
         return _type;
      }

      AstTypeT* getType()
      {
         return _type;
      }

      const AstStatements* getBody() const
      {
         return _body;
      }

      AstStatements* getBody()
      {
         return _body;
      }

      const AstDeclVars& getVars() const
      {
         return *_vars;
      }

      AstDeclVars& getVars()
      {
         return *_vars;
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
      mvv::Symbol          _name;
      AstTypeT*            _type;
      AstDeclVars*         _vars;
      AstStatements*       _body;
   };
}
}

#endif