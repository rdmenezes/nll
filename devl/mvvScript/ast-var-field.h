#ifndef MVV_PARSER_AST_EXP_FIELD_VAR_H_
# define MVV_PARSER_AST_EXP_FIELD_VAR_H_

# include "ast-var.h"

namespace mvv
{
namespace parser
{
   /**
    type Struct1.Struct2.ID
    */
   class MVVSCRIPT_API AstVarField : public AstVar
   {
   public:
      AstVarField( const YYLTYPE& location, AstVar* field, mvv::Symbol name ) : AstVar( location ), _field( field ), _name( name )
      {
         ensure( field, "can't be null" );
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

   private:
      AstVar*       _field;
      mvv::Symbol   _name;
   };
}
}

#endif