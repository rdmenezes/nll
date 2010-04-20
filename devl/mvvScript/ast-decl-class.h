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
      AstDeclClass( const YYLTYPE& location, const mvv::Symbol& name, AstDecls* decls ) : AstDecl( location, name ), _decls( decls ), _memberVariableSize( 0 )
      {
         ensure( decls, "can't be null" );

         for ( AstDecls::Decls::iterator it = decls->getDecls().begin(); it != decls->getDecls().end(); ++it )
         {
            if ( dynamic_cast<AstDeclVar*>( *it ) )
               ++_memberVariableSize;
         }
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

      ui32 getMemberVariableSize() const
      {
         return _memberVariableSize;
      }

   private:
      AstDecls*            _decls;
      ui32                 _memberVariableSize; // hold the number of field an object contains
      RuntimeValue         _runtimeObjectSource; // this hold the current object context (used for example for 'this')
   };
}
}

#endif