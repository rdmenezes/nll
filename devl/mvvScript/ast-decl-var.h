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
      AstDeclVar( const YYLTYPE& location, AstTypeT* type, const mvv::Symbol& name, AstExp* init = 0, AstArgs* declarationList = 0 ) : AstDecl( location ), _type( type ), _name( name ), _init( init ), _declarationList( declarationList ), _class( 0 )
      {
         ensure( type, "can't be null" );
      }

      const mvv::Symbol& getName() const
      {
         return _name;
      }

      const AstTypeT& getType() const
      {
         return *_type;
      }

      const AstArgs* getDeclarationList() const
      {
         return _declarationList;
      }

      AstArgs* getDeclarationList()
      {
         return _declarationList;
      }

      AstTypeT& getType()
      {
         return *_type;
      }

      const AstExp* getInit() const
      {
         return _init;
      }

      AstExp* getInit()
      {
         return _init;
      }

      AstDeclClass* isClassMember() const
      {
         return _class;
      }

      void setClassMember( AstDeclClass* m )
      {
         _class = m;
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
      AstExp*              _init;
      AstArgs*             _declarationList;
      AstDeclClass*        _class;
   };
}
}

#endif