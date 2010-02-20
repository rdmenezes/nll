#ifndef MVV_PARSER_AST_TYPEFIELD_VAR_H_
# define MVV_PARSER_AST_TYPEFIELD_VAR_H_

# include "ast-type.h"

namespace mvv
{
namespace parser
{
   /**
    type Struct1.Struct2.ID
    */
   class MVVSCRIPT_API AstTypeField : public AstTypeT
   {
   public:
      AstTypeField( const YYLTYPE& location, AstTypeT* field, mvv::Symbol name, std::vector<AstExp*>* defaultSize = 0 ) : AstTypeT( location, defaultSize ), _field( field ), _name( name )
      {
         ensure( field, "can't be null" );
      }

      const Symbol& getName() const
      {
         return _name;
      }

      const AstTypeT& getField() const
      {
         return *_field;
      }

      AstTypeT& getField()
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

   private:
      AstTypeT*     _field;
      mvv::Symbol   _name;
   };
}
}

#endif