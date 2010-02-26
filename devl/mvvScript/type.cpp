#include "type.h"

namespace mvv
{
namespace parser
{
   bool TypeFloat::isCompatibleWith( const Type& t ) const
   {
      return dynamic_cast<const TypeInt*>( & t ) ||
             dynamic_cast<const TypeFloat*>( & t );
   }

   Type* TypeFloat::clone() const
   {
      return new TypeFloat();
   }

   bool TypeInt::isCompatibleWith( const Type& t ) const
   {
      return dynamic_cast<const TypeInt*>( & t ) ||
             dynamic_cast<const TypeFloat*>( & t );
   }

   Type* TypeInt::clone() const
   {
      return new TypeInt();
   }

   bool TypeString::isCompatibleWith( const Type& t ) const
   {
      return dynamic_cast<const TypeString*>( & t ) != 0;
   }

   Type* TypeString::clone() const
   {
      return new TypeString();
   }

   bool TypeVoid::isCompatibleWith( const Type& t ) const
   {
      return dynamic_cast<const TypeVoid*>( & t ) != 0;
   }

   Type* TypeVoid::clone() const
   {
      return new TypeVoid();
   }
}
}