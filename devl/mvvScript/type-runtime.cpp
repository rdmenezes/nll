# include "type-runtime.h"
# include "visitor-evaluate.h"

using namespace mvv::parser;

namespace mvv
{
namespace parser
{
namespace impl
{
   void RefcountedTypedDestructor::destroy()
   {
      if ( _data )
      {
         if ( _data->extension && _data->own )
         {
            
            Extension* ext = reinterpret_cast<Extension*>( _data->extension );
            Type* type = ext->type;
            TypeNamed* named = dynamic_cast<TypeNamed*>( type );
            if ( named )
            {
               // if we have a named type && this type has a destructor, run it!
               AstDeclFun* fun = named->getDecl()->getDestructor();
               if ( fun )
               {
                  // call the destructor
                  RuntimeValues vals( 1 );
                  vals[ 0 ].setType( RuntimeValue::TYPE );
                  vals[ 0 ].vals = RefcountedTypedDestructor( ext->evaluator, 0, getDataPtr(), false );
                  ext->evaluator->_callFunction( *fun, vals );
               }
            } else {
               //ensure( 0, "only objects & array of objects can be destructed..." ); // ARRAY
            }
         }

         if ( _data->own )
         {
            delete reinterpret_cast<RuntimeValues*>( _data->data );
            delete reinterpret_cast<Extension*>( _data->extension );
         }
         delete _data;
         _data = 0;
      }
   }
}
}
}