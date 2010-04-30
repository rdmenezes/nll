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
      // TO remove
      TypeNamed* tt = dynamic_cast<TypeNamed*>( t );
      //if ( tt )
      //   std::cout << " object construction=" << tt->getDecl()->getDestructor() << " data=" << getDataPtr() << std::endl;
      //
      std::cout << "construction=" << _data << " " << getDataPtr() << std::endl;

      ensure( eval, "evaluator can't be null" );   
      _data->own = own;
      _data->data = data;
      _data->extension = new Extension( eval, t );   // we are using the extension param to store the type of this object
   }

   void RefcountedTypedDestructor::destroy()
   {
      //_data->ref = 10000;
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
                     if ( ext->evaluator->_env.resultRegister.vals._data == i )
                  {
                     ext->evaluator->_env.resultRegister.vals._data = 0;      // we need to unref the result
                  }
                  //std::cout << " destructor call dat=" << _data << " fn=" <<  fun << " data=" << getDataPtr() << std::endl;
                  // call the destructor
                  RuntimeValues vals( 1 );
                  vals[ 0 ].setType( RuntimeValue::TYPE );
                  vals[ 0 ].vals = RefcountedTypedDestructor( ext->evaluator, 0, (RuntimeValues*)i->data, false );
                  //std::cout << _data->ref << std::endl;
                  ext->evaluator->_callFunction( *fun, vals );

                  //unref();
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