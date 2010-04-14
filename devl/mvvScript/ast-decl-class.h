#ifndef MVV_PARSER_AST_DECL_CLASS_H_
# define MVV_PARSER_AST_DECL_CLASS_H_

# include "typable.h"
# include "ast-exp.h"
# include "ast-decl.h"
# include "ast-type.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstDeclClass : public AstDecl
   {
   public:
      AstDeclClass( const YYLTYPE& location, const mvv::Symbol& name, AstDecls* decls ) : AstDecl( location, name ), _decls( decls )
      {
         ensure( decls, "can't be null" );
      }

      ~AstDeclClass()
      {
         delete _decls;
      }

      const AstDecls& getDeclarations() const
      {
         return *_decls;
      }

      AstDecls& getDeclarations()
      {
         return *_decls;
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

      RuntimeValue& getRuntimeObjectSource()
      {
         return _runtimeObjectSource;
      }

   private:
      AstDecls*            _decls;

      RuntimeValue         _runtimeObjectSource; // this hold the current object context (used for example for 'this')
   };
}
}

#endif