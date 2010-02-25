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
      AstDeclFun( const YYLTYPE& location, AstTypeT* type, const mvv::Symbol& name, AstDeclVars* vars, AstStatements * body = 0, AstDeclClass* memberOfClass = 0 ) : AstDecl( location ), _type( type ), _name( name ), _vars( vars ), _body( body ), _memberOfClass( memberOfClass ), _expectedFunctionType( 0 )
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

      void setMemberOfClass( AstDeclClass* memberOfClass )
      {
         _memberOfClass = memberOfClass;
      }

      AstDeclClass* getMemberOfClass() const
      {
         return _memberOfClass;
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

      // returns the type of the "return" statement of a function. Can be null if no return, or no body visited
      Type* getExpectedFunctionType() const
      {
         return _expectedFunctionType;
      }

      void setExpectedFunctionType( Type* t )
      {
         _expectedFunctionType = t;
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
      AstDeclClass*        _memberOfClass;
      Type*                _expectedFunctionType;
   };
}
}

#endif