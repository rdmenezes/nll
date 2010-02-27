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

      AstExpCall( const YYLTYPE& location, AstVar* name, AstArgs* args ) : AstVar( location ), _name( name ), _args( args ), _simpleName( 0 )
      {
         ensure( name && args, "can't be null" );
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

   private:
      AstVar*     _name;
      AstArgs*    _args;
      const mvv::Symbol*_simpleName;   // dont deallocate // if true: means it is a call to a global function/or construction of a class
   };
}
}

#endif