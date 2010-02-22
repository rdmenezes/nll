#ifndef MVV_PARSER_AST_TYPE2_H_
# define MVV_PARSER_AST_TYPE2_H_

# include "typable.h"
# include "ast-exp.h"

namespace mvv
{
namespace parser
{
   class MVVSCRIPT_API AstTypeT : public Ast, public Typable
   {
   public:
      AstTypeT( const YYLTYPE& location, std::vector<AstExp*>* defaultSize = 0 ) : Ast( location ), _isArray( false ), _defaultSize( defaultSize )
      {
      }

      void setSize( std::vector<AstExp*>* defaultSize )
      {
         _defaultSize = defaultSize;
      }

      const std::vector<AstExp*>* getSize() const
      {
         return _defaultSize;
      }

      std::vector<AstExp*>* getSize()
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

   private:
      bool                  _isArray;
      std::vector<AstExp*>* _defaultSize;
   };

   class MVVSCRIPT_API AstType : public AstTypeT
   {
   public:
      enum Type
      {
         INT,
         FLOAT,
         STRING,
         VOID,
         VAR,
         SYMBOL,
         EMPTY
      };

      AstType( const YYLTYPE& location, Type type, const mvv::Symbol* symbol = 0, std::vector<AstExp*>* defaultSize = 0 ) : AstTypeT( location, defaultSize ), _type( type )
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

      

      const mvv::Symbol* getSymbol() const
      {
         return _symbol;
      }

      Type getType() const
      {
         return _type;
      }

      void setReference( AstDeclClass* decl )
      {
         ensure( _type == SYMBOL, "only for symbol type" );
         _symbolClass = decl;
      }

      AstDeclClass* getReference() const
      {
         return _symbolClass;
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
      mvv::Symbol*         _symbol;
      Type                 _type;
      AstDeclClass*        _symbolClass;
   };
}
}

#endif