#ifndef MVV_PARSER_TYPE_RUNTIME_H_
# define MVV_PARSER_TYPE_RUNTIME_H_

# include "mvvScript.h"
# include <mvvPlatform/refcounted.h>
# include <stack>

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
         REF,        /// reference type
         NIL         /// empty pointer type
      };

   public:
      RuntimeValue()
      {
         type = EMPTY;
         ref = 0;
      }

      RuntimeValue( TypeEnum t ) : type( t )
      {
         ref = 0;
      }

      void setType( TypeEnum t )
      {
         type = t;
      }
  
      
      // values...
      float       floatval;      /// hold the value of the runtime value is of this type
      int         intval;        /// hold the value of the runtime value is of this type
      platform::RefcountedTyped<RuntimeValues> vals; /// hold a list of values (i.e. named value or array)
      std::string stringval;     // hold a string
      RuntimeValue* ref;         // hold a reference

      TypeEnum    type;          /// shortcut for the type of value    
   };

   struct RuntimeEnvironment
   {
      RuntimeEnvironment()
      {
         framePointer = 0;
      }

      RuntimeValues     stack;            // stack, 1 object = 1 entry in the stack
      RuntimeValue      resultRegister;   // when a function returns, var is evaluated... this register is updated
      ui32              framePointer;     // the active frame
      std::stack<ui32>  stackFrame;       // a list of frame pointers
   };

   /**
    @brief unref a variable (in case of a variable, we must create an alias using reference to transmit its value)
    */
   inline RuntimeValue& unref( RuntimeValue& r )
   {
      RuntimeValue* rr = &r;
      while ( rr->type == RuntimeValue::REF )
      {
         rr = rr->ref;
      }
      return *rr;
   }
}
}

#endif