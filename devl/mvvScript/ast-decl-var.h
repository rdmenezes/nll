#ifndef MVV_PARSER_AST_DECL_VAR_H_
# define MVV_PARSER_AST_DECL_VAR_H_

# include "typable.h"
# include "ast-exp.h"
# include "ast-decl.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstDeclVar : public AstDecl
   {
   public:
      AstDeclVar( const YYLTYPE& location, AstType* type, const mvv::Symbol& name, AstExp* init = 0 ) : AstDecl( location ), _type( type ), _name( name ), _init( init )
      {
         ensure( type, "can't be null" );
      }

      mvv::Symbol getName() const
      {
         return _name;
      }

      const AstType* getType() const
      {
         return _type;
      }

      const AstExp* getInit() const
      {
         return _init;
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
      bool                 _isArray;
      mvv::Symbol          _name;
      AstType*             _type;
      AstExp*              _init;
   };
}
}

#endif