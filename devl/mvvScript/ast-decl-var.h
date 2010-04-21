#ifndef MVV_PARSER_AST_DECL_VAR_H_
# define MVV_PARSER_AST_DECL_VAR_H_

# include "typable.h"
# include "ast-exp.h"
# include "ast-decl.h"
# include "ast-args.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstDeclVar : public AstDecl
   {
   public:
      AstDeclVar( const YYLTYPE& location, AstTypeT* type, const mvv::Symbol& name, AstExp* init = 0, AstArgs* declarationList = 0, AstArgs* objectInit = 0 ) : AstDecl( location, name ), _type( type ), _init( init ), _declarationList( declarationList ), _class( 0 ), _objectInit( objectInit ), _constructor( 0 )
      {
         ensure( type, "can't be null" );
  //       _index = -1;
         _indexRuntime = static_cast<ui32>( -1 );
      }

      ~AstDeclVar()
      {
         delete _type;
         delete _init;
         delete _declarationList;
         delete _objectInit;
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

      AstArgs* getObjectInitialization() const
      {
         return _objectInit;
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

      // if not null, this type is nested in another type
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

      // in case a variable is a class, it needs to call the constructor
      void setConstructorCall( AstDeclFun* c )
      {
         _constructor = c;
      }

      AstDeclFun* getConstructorCall() const
      {
         return _constructor;
      }
/*
      void setIndex( ui32 index )
      {
         _index = 0;
      }

      ui32 getIndex() const
      {
         return _index;
      }
*/
      void setRuntimeIndex( ui32 index )
      {
         _indexRuntime = index;
      }

      ui32 getRuntimeIndex() const
      {
         return _indexRuntime;
      }



   private:
      AstTypeT*            _type;
      AstExp*              _init;
      AstArgs*             _declarationList; // like "int a[3] = {0, 21, 42};"
      AstDeclClass*        _class;
      AstArgs*             _objectInit;      // like "Test a(1, "sdfsf", Test3() )"
      AstDeclFun*          _constructor;
      ui32                 _indexRuntime;    // the index of the declaration in the runtime stack relative to the current frame pointer
   };
}
}

#endif