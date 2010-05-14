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
      AstTypeT( const YYLTYPE& location, std::vector<AstExp*>* defaultSize = 0 ) : Ast( location ), _isArray( false ), _defaultSize( defaultSize ), _symbolClass( 0 ), _typeIsReference( false )
      {
      }

      virtual ~AstTypeT()
      {
         if ( _defaultSize )
         {
            for ( ui32 n = 0; n < _defaultSize->size(); ++n )
            {
               delete ( *_defaultSize )[ n ];
            }
         }
         delete _defaultSize;
      }

      void setIsAReference( bool isRef )
      {
         _typeIsReference = isRef;
      }

      bool isAReference() const
      {
         return _typeIsReference;
      }

      // if this is an array, set the size
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

      void setReference( AstDeclClass* decl )
      {
         _symbolClass = decl;
      }

      AstDeclClass* getReference() const
      {
         return _symbolClass;
      }

   private:
      bool                  _isArray;
      std::vector<AstExp*>* _defaultSize;
      AstDeclClass*         _symbolClass;
      bool                  _typeIsReference;
   };

   class MVVSCRIPT_API AstType : public AstTypeT
   {
   public:
      enum Type
      {
         CMP_INT,
         CMP_FLOAT,
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
   };
}
}

#endif