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
      AstVarArray( const YYLTYPE& location, AstVar* name, AstExp* index ) : AstVar( location ), _name( name ), _index( index ), _fun( 0 )
      {
         ensure( name && index, "can't be null" );
      }

      ~AstVarArray()
      {
         delete _name;
         delete _index;
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

      void setFunction( AstDeclFun* f )
      {
         _fun = f;
      }

      AstDeclFun* getFunction() const
      {
         return _fun;
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
      AstExp*       _index;
      AstDeclFun*   _fun;     // in case we have defined a custom operator[]
   };
}
}

#endif