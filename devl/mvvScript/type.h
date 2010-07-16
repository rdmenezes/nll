#ifndef MVV_PARSER_TYPE_H_
# define MVV_PARSER_TYPE_H_

# include "mvvScript.h"
# include <mvvPlatform/types.h>

namespace mvv
{
namespace parser
{
   class AstDeclClass;
   class TypeFunctionPointer;

   class MVVSCRIPT_API Type
   {
   public:
      Type( bool ref) : _ref( ref )
      {
      }

      // return true if types are compatible.
      virtual bool isCompatibleWith( const Type& t ) const = 0;

      // clone the type
      virtual Type* clone() const = 0;

      // return true if the types are exactly the same
      virtual bool isEqual( const Type& t ) const = 0;

      bool isReference() const
      {
         return _ref;
      }

      void setReference( bool ref )
      {
         _ref = ref;
      }

   protected:
      bool _ref;  // true if the type is a reference i.e. "int&"
   };

   class MVVSCRIPT_API TypeFloat : public Type
   {
   public:
      TypeFloat( bool isRef ) : Type( isRef ){}
      virtual bool isCompatibleWith( const Type& t ) const;
      virtual Type* clone() const;
      virtual bool isEqual( const Type& t ) const
      {
         return dynamic_cast<const TypeFloat*>( &t ) != 0;
      }
   };

   class MVVSCRIPT_API TypeInt : public Type
   {
   public:
      TypeInt( bool isRef ) : Type( isRef ){}
      virtual bool isCompatibleWith( const Type& t ) const;
      virtual Type* clone() const;
      virtual bool isEqual( const Type& t ) const
      {
         const TypeInt* tt = dynamic_cast<const TypeInt*>( &t );
         return tt != 0;
      }
   };

   class MVVSCRIPT_API TypeString : public Type
   {
   public:
      TypeString( bool isRef ) : Type( isRef ){}
      virtual bool isCompatibleWith( const Type& t ) const;
      virtual Type* clone() const;
      virtual bool isEqual( const Type& t ) const
      {
         return dynamic_cast<const TypeString*>( &t ) != 0;
      }
   };

   class MVVSCRIPT_API TypeVoid : public Type
   {
   public:
      TypeVoid() : Type( false )
      {}

      virtual bool isCompatibleWith( const Type& t ) const;
      virtual Type* clone() const;
      virtual bool isEqual( const Type& t ) const
      {
         return dynamic_cast<const TypeVoid*>( &t ) != 0;
      }
   };

   class MVVSCRIPT_API TypeError : public Type
   {
   public:
      TypeError() : Type( false )
      {}

      virtual bool isCompatibleWith( const Type& ) const
      {
         return true;
      }

      virtual Type* clone() const
      {
         return new TypeError();
      }

      virtual bool isEqual( const Type& t ) const
      {
         return dynamic_cast<const TypeError*>( &t ) != 0;
      }
   };

   class MVVSCRIPT_API TypeArray : public Type
   {
   public:
      // dimensionality = number of dimensions of the array!
      TypeArray( ui32 dimensionality, const Type& root, bool isRef ) : _dimensionality( dimensionality ), _root( root ), Type( isRef )
      {
      }

      virtual bool isCompatibleWith( const Type& t ) const
      {
         const TypeArray* array = dynamic_cast<const TypeArray* >( &t );
         if ( !array )
            return 0;
         bool dimOk =  true; //array->getDimentionality() == getDimentionality() || !getDimentionality() || !array->getDimentionality();
         return array && dimOk && _root.isEqual( array->getRoot() );
      }

      virtual bool isEqual( const Type& t ) const
      {
         const TypeArray* array = dynamic_cast<const TypeArray* >( &t );
         if ( !array )
            return false;
         bool dimOk = true; //array->getDimentionality() == getDimentionality() || !getDimentionality() || !array->getDimentionality();
         return array && dimOk && _root.isEqual( array->getRoot() );
      }

      virtual Type* clone() const
      {
         return new TypeArray( _dimensionality, _root, isReference() );
      }

      const Type& getRoot() const
      {
         return _root;
      }

      ui32 getDimentionality() const
      {
        return  _dimensionality;
      }

   private:
      TypeArray& operator=( const TypeArray& );
      TypeArray( const TypeArray& );

   private:
      ui32  _dimensionality;
      const Type& _root;
   };

   class MVVSCRIPT_API TypeNamed : public Type
   {
   public:
      TypeNamed( AstDeclClass* decl, bool isRef ) : _decl( decl ), Type( isRef )
      {
      }

      virtual bool isCompatibleWith( const Type& t ) const
      {
         const TypeNamed* named = dynamic_cast<const TypeNamed* >( &t );
         if ( named && named->getDecl() == getDecl() )
            return true;
         // else // TODO case where the constructor is 1 argument -> automatic promotion to handle here
         return false;
      }

      virtual bool isEqual( const Type& t ) const
      {
         const TypeNamed* tt = dynamic_cast<const TypeNamed*>( &t );
         return tt != 0 && tt->getDecl() == getDecl();
      }



      AstDeclClass* getDecl() const
      {
         return _decl;
      }

      virtual Type* clone() const
      {
         return new TypeNamed( _decl, isReference() );
      }

   private:
      AstDeclClass* _decl;
   };

   class MVVSCRIPT_API TypeNil : public Type
   {
   public:
      TypeNil() : Type( false )
      {
      }

      virtual bool isCompatibleWith( const Type& t ) const;

      virtual Type* clone() const
      {
         return new TypeNil();
      }

      virtual bool isEqual( const Type& t ) const
      {
         return dynamic_cast<const TypeNil*>( &t ) != 0;
      }
   };

   class MVVSCRIPT_API TypeFunctionPointer : public Type
   {
   public:
      TypeFunctionPointer( bool ref, Type* returnType, const std::vector<Type*>& args, TypeNamed* classDecl = 0 ) : Type( ref ), _returnType( returnType ), _args( args ), _classDecl( classDecl )
      {
         assert( returnType );
      }

      virtual bool isCompatibleWith( const Type& t ) const
      {
         const TypeNil* fn = dynamic_cast<const TypeNil*>( &t );
         if ( fn )
            return true;   // we can assign pointer = NULL
         return isEqual( t );
      }

      virtual Type* clone() const
      {
         return new TypeFunctionPointer( isReference(), _returnType, _args, _classDecl );
      }

      virtual bool isEqual( const Type& t ) const
      {
         const TypeFunctionPointer* fp = dynamic_cast<const TypeFunctionPointer*>( &t );
         if ( !fp )
            return false;
         if ( fp->_args.size() != _args.size() || ( _classDecl && !fp->_classDecl ) || ( !_classDecl && fp->_classDecl ) )
            return false;
         if ( !_returnType->isEqual( *fp->_returnType ) )
            return false;
         for ( ui32 n = 0; n < _args.size(); ++n )
            if ( !_args[ n ]->isEqual( *fp->_args[ n ] ) )
               return false;
         return true;
      }

      const std::vector<Type*>& getArgs() const
      {
         return _args;
      }

      const Type& getReturnType() const
      {
         return *_returnType;
      }

   private:
      Type*                _returnType;
      std::vector<Type*>   _args;
      TypeNamed*           _classDecl;
   };

}
}

#endif