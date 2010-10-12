#ifndef MVV_PARSER_AST_TYPE2_H_
# define MVV_PARSER_AST_TYPE2_H_

# include "typable.h"
# include "ast-exp.h"
# include "ast-decl-vars.h"

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

      void setReference( AstDecl* decl )
      {
         _symbolClass = decl;
      }

      AstDecl* getReference() const
      {
         return _symbolClass;
      }

   private:
      bool                  _isArray;
      std::vector<AstExp*>* _defaultSize;
      AstDecl*              _symbolClass;          // holds a class or typedef
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

   class MVVSCRIPT_API AstFunctionType : public AstTypeT
   {
   public:
      AstFunctionType( const YYLTYPE& location, AstTypeT* returnType, AstDeclVars* args ) : AstTypeT( location, 0 ), _returnType( returnType ), _args( args )
      {
         ensure( returnType && args, "can't be null" );
         for ( AstDeclVars::Decls::iterator it = args->getVars().begin(); it != args->getVars().end(); ++it )
         {
            (*it)->setIsInFunctionPrototype();
         }
      }

      ~AstFunctionType();

      const AstTypeT& getType() const
      {
         return *_returnType;
      }

      AstTypeT& getType()
      {
         return *_returnType;
      }

      const AstDeclVars& getArgs() const
      {
         return *_args;
      }

      AstDeclVars& getArgs()
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
      AstTypeT*      _returnType;
      AstDeclVars*   _args;
   };
}
}

#endif