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
      AstDeclFun( const YYLTYPE& location, AstType* type, const mvv::Symbol& name, AstDeclVars* vars, AstStatements * body = 0 ) : AstDecl( location ), _type( type ), _name( name ), _vars( vars ), _body( body )
      {
         ensure( type, "can't be null" );
      }

      mvv::Symbol getName() const
      {
         return _name;
      }

      const AstType& getType() const
      {
         return *_type;
      }

      const AstStatements* getBody() const
      {
         return _body;
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
      AstType*             _type;
      AstDeclVars*         _vars;
      AstStatements*       _body;
   };
}
}

#endif