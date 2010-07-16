#ifndef MVV_PARSER_AST_EXP_FIELD_VAR_H_
# define MVV_PARSER_AST_EXP_FIELD_VAR_H_

# include "ast-var.h"

namespace mvv
{
namespace parser
{
   /**
    type test1.test2.val stored as: varfield_test1( varfield_test2( varsimple_test1 ) )
    */
   class MVVSCRIPT_API AstVarField : public AstVar
   {
   public:
      AstVarField( const YYLTYPE& location, AstVar* field, mvv::Symbol name ) : AstVar( location ), _field( field ), _name( name ), _pointee( 0 )
      {
         ensure( field, "can't be null" );
         _memberClass = 0;
         _memberFun = 0;
      }

      ~AstVarField()
      {
         delete _field;
      }

      const Symbol& getName() const
      {
         return _name;
      }

      const AstVar& getField() const
      {
         return *_field;
      }

      AstVar& getField()
      {
         return *_field;
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

      // when a.b, set the type b in the var field
      void setPointee( AstDecl* pointee )
      {
         _pointee = pointee;
      }

      AstDecl* getPointee() const
      {
         return _pointee;
      }

      void setMemberFunction( AstDeclClass* memberClass, AstDeclFun* memberFun )
      {
          _memberClass = memberClass;
          _memberFun = memberFun;
      }

      AstDeclClass*  getMemberClass()
      {
         return _memberClass;
      }

      AstDeclFun*    getMemberFun()
      {
         return _memberFun;
      }

   private:
      AstVar*       _field;
      AstDecl*      _pointee;    // don't deallocate!
      mvv::Symbol   _name;

      // member function pointer
      AstDeclClass*  _memberClass;
      AstDeclFun*    _memberFun;
   };
}
}

#endif