#ifndef MVV_PARSER_AST_DECL_FUN_H_
# define MVV_PARSER_AST_DECL_FUN_H_

# include "typable.h"
# include "ast-exp.h"
# include "ast-decl.h"
# include "ast-type.h"
# include "ast-decl-vars.h"
# include "function-runnable.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstDeclFun : public AstDecl
   {
   public:
      AstDeclFun( const YYLTYPE& location, AstTypeT* type, const mvv::Symbol& name, AstDeclVars* vars, AstStatements * body = 0, AstDeclClass* memberOfClass = 0 ) : AstDecl( location, name ), _type( type ), _vars( vars ), _body( body ), _memberOfClass( memberOfClass ), _expectedFunctionType( 0 )
      {
         ensure( vars, "can't be null" );
      }

      ~AstDeclFun()
      {
         delete _type;
         delete _vars;
         delete _body;
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

      void setImportedFunction( platform::RefcountedTyped<FunctionRunnable> f )
      {
         _functionImported = f;
      }

      FunctionRunnable* getImportedFunction()
      {
         return _functionImported.getDataPtr();
      }

   private:
      AstTypeT*            _type;
      AstDeclVars*         _vars;
      AstStatements*       _body;
      AstDeclClass*        _memberOfClass;
      Type*                _expectedFunctionType;

      platform::RefcountedTyped<FunctionRunnable> _functionImported; // when a function must be imported, at runtime, this is what will be run
   };
}
}

#endif