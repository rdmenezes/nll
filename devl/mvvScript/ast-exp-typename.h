#ifndef MVV_PARSER_AST_EXP_TYPENAME_H_
# define MVV_PARSER_AST_EXP_TYPENAME_H_

# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstExpTypename : public AstExp
   {
   public:
      typedef int    value_type;

      AstExpTypename( const YYLTYPE& location, AstTypeT* type, AstArgs* args ) : AstExp( location ), _type( type ), _args( args )
      {
         ensure( type && args, "must not be null" );
      }

      const AstTypeT& getType() const
      {
         return *_type;
      }

      AstTypeT& getType()
      {
         return *_type;
      }

      AstArgs& getArgs()
      {
         return *_args;
      }

      const AstArgs& getArgs() const
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
      AstTypeT*  _type;
      AstArgs*   _args;
   };
}
}

#endif