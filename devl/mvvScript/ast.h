#ifndef MVV_PARSER_AST_H_
# define MVV_PARSER_AST_H_

# include "forward.h"
# include "visitor.h"


namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API Ast
   {
   public:
      Ast( const YYLTYPE& location ) : _location( location )
      {
      }

      virtual ~Ast()
      {
      }

      /// Accept a const visitor \a v.
      virtual void accept( ConstVisitor& v ) const = 0;

      /// Accept a non-const visitor \a v.
      virtual void accept( Visitor& v ) = 0;

      /// Returns the location of an AST
      const YYLTYPE& getLocation() const
      {
         return _location;
      }

   private:
      // disabled copy
      Ast& operator=( Ast& );
      Ast( Ast& );

   protected:
      YYLTYPE  _location;
   };
}
}

#endif