#ifndef MVV_PARSER_AST_VAR_SIMPLE_H_
# define MVV_PARSER_AST_VAR_SIMPLE_H_

# include "ast-var.h"
# include "ast-decl-var.h"

namespace mvv
{
namespace parser
{
   // if we have a reference for var, it means it is a simple variable (and not used in a function call)
   class MVVSCRIPT_API AstVarSimple : public AstVar
   {
   public:
      AstVarSimple( const YYLTYPE& location, mvv::Symbol name, bool isDeduced ) : AstVar( location ), _name( name ), _isDeduced ( isDeduced )
      {
         _isFunctionAddress = false;
         _functionAddress = 0;
      }

      const Symbol& getName() const
      {
         return _name;
      }


      void setIsFunctionAddress( bool val )
      {
         _isFunctionAddress = val;
      }

      bool getIsFunctionAddress() const
      {
         return _isFunctionAddress;
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

      // we do this if we use the var simple to get the pointer of the function: void f(){} => FUNCTION_TYPEDEF pf = f;
      void setFunctionAddress( AstDeclFun* f )
      {
         _functionAddress = f;
      }

      AstDeclFun* getFunctionAddress() const
      {
         return _functionAddress;
      }

      virtual ui32 getRuntimeIndex()
      {
         AstDeclVar* decl = dynamic_cast<AstDeclVar*>( getReference() );
         if ( decl )
            return decl->getRuntimeIndex();
         return 0;
      }

   private:
      mvv::Symbol   _name;
      bool          _isDeduced;  // deprecated?.
      bool          _isFunctionAddress; // true if this must returns the adress of a function
      AstDeclFun*   _functionAddress;   // the function to return the address
   };
}
}

#endif