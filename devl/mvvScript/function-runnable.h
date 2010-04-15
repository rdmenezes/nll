#ifndef MVV_PARSER_FUNCTION_RUNNABLE_H_
# define MVV_PARSER_FUNCTION_RUNNABLE_H_

# include "mvvScript.h"
# include "symbol-table.h"
# include "parser-context.h"

namespace mvv
{
namespace parser
{
   // forward declaration
   class AstDeclFun;

   /**
    @brief An abstraction representing a function call.
           
           When importing a function from a DLL, the DLL will be dynamically linked. The DLL
           must then register all functions/member functions it is declaring.
           At runtime, when a function address will match a <code>FunctionRunnable</code>, it
           will be called.
    */
   class MVVSCRIPT_API FunctionRunnable
   {
   public:
      FunctionRunnable( AstDeclFun* fun ) : _fun( fun )
      {
      }

      virtual ~FunctionRunnable()
      {}

      const AstDeclFun* getFunctionPointer() const
      {
         return _fun;
      }

      /**
       @brief This method is called at runtime when the _fun address match an imported function
       @return a runtime value representing the return value of the function
       @note types and return values must be compatible!
       */
      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args ) = 0;

   private:
      // disabled copy
      FunctionRunnable& operator=( const FunctionRunnable& );
      FunctionRunnable( const FunctionRunnable& );

   protected:
      AstDeclFun* _fun;
   };
}
}

#endif