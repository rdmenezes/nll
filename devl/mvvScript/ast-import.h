#ifndef MVV_PARSER_AST_IMPORT_H_
# define MVV_PARSER_AST_IMPORT_H_

# include "ast.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstImport : public Ast
   {
   public:
      AstImport( const YYLTYPE& location, const std::string& str ) : Ast( location ), _str( str )
      {
      }

      const std::string& getStr() const
      {
         return _str;
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
      std::string _str;
   };
}
}

#endif