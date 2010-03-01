#ifndef MVV_PARSER_TYPE_H_
# define MVV_PARSER_TYPE_H_

# include "mvvScript.h"
# include <mvvPlatform/types.h>

namespace mvv
{
namespace parser
{
   class AstDeclClass;

   class MVVSCRIPT_API Type
   {
   public:
      // return true if types are compatible.
      virtual bool isCompatibleWith( const Type& t ) const = 0;

      // clone the type
      virtual Type* clone() const = 0;

      // return true if the types are exactly the same
      virtual bool isEqual( const Type& t ) const = 0;
   };

   class MVVSCRIPT_API TypeFloat : public Type
   {
   public:
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
      virtual bool isCompatibleWith( const Type& t ) const;
      virtual Type* clone() const;
      virtual bool isEqual( const Type& t ) const
      {
         return dynamic_cast<const TypeInt*>( &t ) != 0;
      }
   };

   class MVVSCRIPT_API TypeString : public Type
   {
   public:
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
      TypeArray( ui32 dimensionality, const Type& root ) : _dimensionality( dimensionality ), _root( root )
      {
      }

      virtual bool isCompatibleWith( const Type& t ) const
      {
         const TypeArray* array = reinterpret_cast<const TypeArray* >( &t );
         if ( !array )
            return 0;
         return array && array->getDimentionality() == getDimentionality() && _root.isCompatibleWith( array->getRoot() );
      }

      virtual bool isEqual( const Type& t ) const
      {
         const TypeArray* array = reinterpret_cast<const TypeArray* >( &t );
         if ( !array )
            return false;
         return array && array->getDimentionality() == getDimentionality() && _root.isEqual( array->getRoot() );
      }

      virtual Type* clone() const
      {
         return new TypeArray( _dimensionality, _root );
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
      TypeNamed( AstDeclClass* decl ) : _decl( decl )
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
         return new TypeNamed( _decl );
      }

   private:
      AstDeclClass* _decl;
   };

   class MVVSCRIPT_API TypeNil : public Type
   {
   public:
      virtual bool isCompatibleWith( const Type& t ) const
      {
         return dynamic_cast<const TypeNamed*>( &t ) != 0;
      }

      virtual Type* clone() const
      {
         return new TypeNil();
      }

      virtual bool isEqual( const Type& ) const
      {
         // we never want to check nil == nil
         return 0;
      }
   };

}
}

#endif