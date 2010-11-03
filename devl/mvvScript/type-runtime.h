#ifndef MVV_PARSER_TYPE_RUNTIME_H_
# define MVV_PARSER_TYPE_RUNTIME_H_

# include "mvvScript.h"
# include <mvvPlatform/refcounted.h>
# include <stack>
# include "type.h"

namespace mvv
{
namespace parser
{
   class VisitorEvaluate;

   class RuntimeValue;

   class AstDeclFun;

   /// Represents a set of value, typically used for named type
   typedef std::vector<RuntimeValue>   RuntimeValues;

   namespace impl
   {
      /**
       @brief This is only used to refcount an "class" or an array
       @note _data->extension stores the internal type of the object we are representing
       */
      class MVVSCRIPT_API RefcountedTypedDestructor : public platform::Refcounted
      {
      public:
         /**
          @brief we need to be able to call the constructor through the current evaluation context!
          */
         struct Extension
         {
            Extension( VisitorEvaluate* e, Type* t )
            {
               type = t;
               evaluator = e;
            }

            Type*             type;
            VisitorEvaluate*  evaluator;
         };

      public:
         typedef RuntimeValues   value_type;

         virtual ~RefcountedTypedDestructor()
         {
            if ( _data )
            {
               // we unref here to avoid virtual call to destroy! a second unref() might be run
               // in the base destructor but this doesn't do anything.
               unref();
            }
         }

         RefcountedTypedDestructor()
         {}

         /**
          @brief a tailored refcountedtyped calling a destructor just before the object is deallocated
          @param t expected the TypeNamed of the class where the destructor will be called
          @param eval the evaluator used to evaluate the destructor
          @note if t && eval are null, no destructor is called
          */
         RefcountedTypedDestructor( VisitorEvaluate* eval, Type* t, RuntimeValues* data, bool own = true );

         virtual void destroy();

         RuntimeValues& getData()
         {
            return *reinterpret_cast<RuntimeValues*>( _data->data );
         }

         RuntimeValues* getDataPtr()
         {
            return reinterpret_cast<RuntimeValues*>( _data->data );
         }

         const RuntimeValues* getDataPtr() const
         {
            return reinterpret_cast<RuntimeValues*>( _data->data );
         }

         const RuntimeValues& getData() const
         {
            return *reinterpret_cast<RuntimeValues*>( _data->data );
         }

         RuntimeValues& operator*()
         {
            ensure( getNumberOfReference(), "invalid operation" );
            return *reinterpret_cast<RuntimeValues*>( _data->data );
         }

         RuntimeValues* operator&()
         {
            return reinterpret_cast<RuntimeValues*>( _data->data );
         }

         const RuntimeValues& operator*() const
         {
            ensure( getNumberOfReference(), "invalid operation" );
            return *reinterpret_cast<RuntimeValues*>( _data->data );
         }

         const RuntimeValues* operator&() const
         {
            return reinterpret_cast<RuntimeValues*>( _data->data );
         }

         const Extension* getExtension() const
         {
            return reinterpret_cast<Extension*>( _data->extension );
         }
      };
   }

   /**
    @brief Represents a runtime value with its runtime type

    All data are stored in this structure (int, float..) however only one is valid, dependending on the
    type.
    */
   class RuntimeValue
   {
   public:
      typedef impl::RefcountedTypedDestructor   RefcountedValues;

      /**
       @brief Basically classify types in these categories
       */
      enum TypeEnum
      {
         EMPTY,      /// empty type
         CMP_INT,        /// int type
         CMP_FLOAT,      /// float type
         STRING,     /// string
         TYPE,       /// class type
         REF,        /// reference type
         NIL,        /// empty pointer type  // TODO test!
         PTR,         /// a pointer from external resource stores in the ref field
         FUN_PTR     /// a function pointer
      };

   public:
      RuntimeValue() : type( EMPTY ), ref( 0 ), functionPointer( 0 )
      {
      }

      RuntimeValue( TypeEnum t ) : type( t ), ref( 0 ), functionPointer( 0 )
      {
      }

      void setType( TypeEnum t )
      {
         type = t;
      }
  
      
      // values...
      float       floatval;      /// hold the value of the runtime value is of this type
      int         intval;        /// hold the value of the runtime value is of this type
      RefcountedValues vals;     /// hold a list of values (i.e. named value or array)
      std::string stringval;     /// hold a string
      RuntimeValue* ref;         /// hold a reference
      AstDeclFun*   functionPointer;/// a funtion pointer 

      TypeEnum    type;          /// shortcut for the type of value    
   };

   struct RuntimeEnvironment
   {
      RuntimeEnvironment()
      {
         framePointer = 0;
      }

      void clear()
      {
         // we need to clear these values as they can save a smart pointer...
         resultRegister.vals.unref();

         // we need to unref all share pointers: in case an object is calling a destructor, it will push on the stack which is being destroyed...
         for ( int n = 0; n < static_cast<int>( stack.size() ); ++n )
         //for ( int n = static_cast<int>( stack.size() ) - 1; n >= 0; --n )
         {
            stack[ n ].vals.unref();
         }

         stack.clear();
      }

      RuntimeValues     stack;            // stack, 1 object = 1 entry in the stack
      RuntimeValue      resultRegister;   // when a function returns, var is evaluated... this register is updated
      ui32              framePointer;     // the active frame
      std::stack<ui32>  stackFrame;       // a list of frame pointers
      std::stack<ui32>  stackUnstack;     // a list of end of frame pointers
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