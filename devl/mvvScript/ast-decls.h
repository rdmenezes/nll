#ifndef MVV_PARSER_AST_DECLS_H_
# define MVV_PARSER_AST_DECLS_H_

# include "ast.h"


namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstDecls : public Ast
   {
   public:
      typedef std::list<AstDecl*>   Decls;

      AstDecls( const YYLTYPE& location ) : Ast( location )
      {
      }

      ~AstDecls()
      {
         for ( Decls::iterator i = _decls.begin(); i != _decls.end(); ++i )
            delete *i;
      }

      void insert( AstDecl* ast )
      {
         _decls.push_front( ast );
      }

      const Decls& getDecls() const
      {
         return _decls;
      }

      Decls& getDecls()
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