#ifndef MVV_PARSER_AST_EXP_CALL_H_
# define MVV_PARSER_AST_EXP_CALL_H_

# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstExpCall : public AstExp
   {
   public:
      typedef float    value_type;

      AstExpCall( const YYLTYPE& location, AstVar* name, AstArgs* args ) : AstExp( location ), _name( name ), _args( args )
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

   private:
      AstVar*   _name;
      AstArgs*  _args;
   };
}
}

#endif