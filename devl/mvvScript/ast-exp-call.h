#ifndef MVV_PARSER_AST_EXP_CALL_H_
# define MVV_PARSER_AST_EXP_CALL_H_

# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstExpCall : public AstVar
   {
   public:
      typedef float    value_type;

      AstExpCall( const YYLTYPE& location, AstVar* name, AstArgs* args ) : AstVar( location ), _name( name ), _args( args ), _simpleName( 0 ), _instanciation( 0 ), _construction( 0 ), _call( 0 )
      {
         ensure( name && args, "can't be null" );
      }

      ~AstExpCall()
      {
         delete _name;
         delete _args;
      }

      const AstVar& getName() const
      {
         return *_name;
      }

      AstVar& getName()
      {
         return *_name;
      }

      const AstArgs& getArgs() const
      {
         return *_args;
      }

      AstArgs& getArgs()
      {
         return *_args;
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

      void setSimpleName( const mvv::Symbol& name )
      {
         _simpleName = &name;
      }

      const mvv::Symbol* getSimpleName() const
      {
         return _simpleName;
      }

      void setInstanciation( AstDeclClass* i )
      {
         _instanciation = i;
      }

      AstDeclClass* getInstanciation() const
      {
         return _instanciation;
      }

      void setConstructed( AstDeclClass* i )
      {
         _construction = i;
      }

      AstDeclClass* getConstructed() const
      {
         return _construction;
      }

      void setFunctionCall( AstDeclFun* f )
      {
         _call = f;
      }

      AstDeclFun* getFunctionCall()
      {
         return _call;
      }


   private:
      AstVar*     _name;
      AstArgs*    _args;
      const mvv::Symbol*_simpleName;    // dont deallocate // if true: means it is a call to a global function/or construction of a class
      AstDeclClass*     _instanciation; // BINDING // the reference will hold a class if it is a class to be called with operator()
      AstDeclClass*     _construction;  // BINDING // refernce will hold a class def if the class need to be constructed
      AstDeclFun*       _call;          // the function that needs to be called
   };
}
}

#endif