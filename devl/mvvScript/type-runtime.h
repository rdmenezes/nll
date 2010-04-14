#ifndef MVV_PARSER_TYPE_RUNTIME_H_
# define MVV_PARSER_TYPE_RUNTIME_H_

# include "mvvScript.h"
# include <mvvPlatform/refcounted.h>

namespace mvv
{
namespace parser
{
   class RuntimeValue;

   /// Represents a set of value, typically used for named type
   typedef std::vector<RuntimeValue>   RuntimeValues;

   /**
    @brief Represents a runtime value with its runtime type

    All data are stored in this structure (int, float..) however only one is valid, dependending on the
    type.
    */
   class MVVSCRIPT_API RuntimeValue
   {
   public:
      /**
       @brief Basically classify types in these categories
       */
      enum TypeEnum
      {
         EMPTY,      /// empty type
         INT,        /// int type
         FLOAT,      /// float type
         STRING,     /// string
         TYPE,       /// class type
         ARRAY,      /// array type
         NIL         /// empty pointer type
      };
   public:
      RuntimeValue()
      {
         type = EMPTY;
         typeval = 0;
      }

      RuntimeValue( TypeEnum t, Type* v ) : type( t ), typeval( v )
      {
      }

      void setType( TypeEnum t, Type* tv )
      {
         ensure( tv, "can't be null" );
         type = t;
         typeval = tv;
      }

      TypeEnum    type;          /// shortcut for the type of value
      Type*       typeval;       /// the underlying type

      float       floatval;      /// hold the value of the runtime value is of this type
      int         intval;        /// hold the value of the runtime value is of this type
      std::string stringval;     /// hold the value of the runtime value is of this type

      platform::RefcountedTyped<RuntimeValues> vals; /// hold a list of values (i.e. named value or array)
   };
}
}

#endif