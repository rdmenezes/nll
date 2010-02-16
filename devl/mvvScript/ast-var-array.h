#ifndef MVV_PARSER_AST_EXP_VAR_ARRAY_H_
# define MVV_PARSER_AST_EXP_VAR_ARRAY_H_

# include "ast-var.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstVarArray : public AstVar
   {
   public:
      AstVarArray( const YYLTYPE& location, AstVar* name, bool isDeduced, AstExp* index ) : AstVar( location ), _name( name ), _isDeduced ( isDeduced ), _index( index )
      {
         ensure( name && index, "can't be null" );
      }

      const AstVar& getName() const
      {
         return *_name;
      }

      AstVar& getName()
      {
         return *_name;
      }

      const AstExp& getIndex() const
      {
         return *_index;
      }

      AstExp& getIndex()
      {
         return *_index;
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
      AstVar*       _name;
      bool          _isDeduced;
      AstExp*       _index;
   };
}
}

#endif