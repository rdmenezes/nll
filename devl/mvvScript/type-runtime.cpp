# include "type-runtime.h"
# include "visitor-evaluate.h"

using namespace mvv::parser;

namespace mvv
{
namespace parser
{
namespace impl
{
   RefcountedTypedDestructor::RefcountedTypedDestructor( VisitorEvaluate* eval, Type* t, RuntimeValues* data, bool own )
   {  
      _data->own = own;
      _data->data = data;
      _data->extension = new Extension( eval, t );   // we are using the extension param to store the type of this object
   }

   void RefcountedTypedDestructor::destroy()
   {
      mvv::platform::Refcounted::Internals* i = _data;
      if ( i )
      {
         if ( i->extension && i->own )
         {
            
            Extension* ext = reinterpret_cast<Extension*>( i->extension );
            Type* type = ext->type;
            TypeNamed* named = dynamic_cast<TypeNamed*>( type );
            if ( named )
            {

               // if we have a named type && this type has a destructor, run it!
               AstDeclFun* fun = named->getDecl()->getDestructor();
               if ( fun )
               {
                  if ( ext->evaluator )
                  {
                     // save the result register
                     // i.e. in case a destructor is called but a function returned
                     // a value: the result register will be erased, losing the function return value
                     // that's why we need to save and restore the result register
                     RuntimeValue resultCopy = ext->evaluator->_env.resultRegister;

                     assert( ext->evaluator ); // we need an evaluator to run the destructor
                     if ( ext->evaluator->_env.resultRegister.vals._data == i )
                     {
                        // if the result is stored in the result register, we need to unref
                        // else, as we still have a ref in the result, this will be ref & unref (even though it is not a type)
                        ext->evaluator->_env.resultRegister.vals._data = 0;
                     }

                     // call the destructor
                     RuntimeValues vals( 1 );
                     vals[ 0 ].setType( RuntimeValue::TYPE );
                     vals[ 0 ].vals = RefcountedTypedDestructor( ext->evaluator, 0, (RuntimeValues*)i->data, false );
                     ext->evaluator->_callFunction( *fun, vals );

                     // restore the result register
                     ext->evaluator->_env.resultRegister = resultCopy;
                  } else {
                     std::cout << "non critical error: destructor cannot be called as no evaluator was attached!" << std::endl;
                  }
               }
            } else {
               //ensure( 0, "only objects & array of objects can be destructed..." ); // ARRAY
            }
         }

         if ( i->own )
         {
            delete reinterpret_cast<RuntimeValues*>( i->data );
            delete reinterpret_cast<Extension*>( i->extension );

            i->data = 0;
            i->extension = 0;
         }
         delete i;
         i = 0;
      }
   }
}
}
}