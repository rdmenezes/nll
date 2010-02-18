#ifndef MVV_PARSER_AST_TYPE2_H_
# define MVV_PARSER_AST_TYPE2_H_

# include "typable.h"
# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstType : public Ast, public Typable
   {
   public:
      enum Type
      {
         INT,
         FLOAT,
         STRING,
         VOID,
         VAR,
         SYMBOL
      };

      AstType( const YYLTYPE& location, Type type, const mvv::Symbol* symbol = 0, AstExp* defaultSize = 0 ) : Ast( location ), _type( type ), _isArray( false ), _defaultSize( defaultSize )
      {
         if ( symbol )
         {
            _symbol = new mvv::Symbol( *symbol );
         } else {
            _symbol = 0;
         }
      }

      ~AstType()
      {
         delete _symbol;
      }

      void setSize( AstExp* defaultSize )
      {
         _defaultSize = defaultSize;
      }

      const AstExp* getSize() const
      {
         return _defaultSize;
      }

      AstExp* getSize()
      {
         return _defaultSize;
      }

      void setArray( bool val )
      {
         _isArray = val;
      }

      bool isArray() const
      {
         return _isArray;
      }

      const mvv::Symbol* getSymbol() const
      {
         return _symbol;
      }

      Type getType() const
      {
         return _type;
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
      bool                 _isArray;
      mvv::Symbol*         _symbol;
      Type                 _type;
      AstExp*              _defaultSize;
   };
}
}

#endif