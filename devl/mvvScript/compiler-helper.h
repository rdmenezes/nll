#ifndef MVV_PARSER_COMPILER_HELPER_H_
# define MVV_PARSER_COMPILER_HELPER_H_

# include "compiler.h"

namespace mvv
{
namespace parser
{
   /**
    @brief Create fields for a runtime value
    @note if e == 0 || t == 0, destructor cannot be called for this object! (so this is invalid if the type has a destructor)
    */
   inline void createFields( RuntimeValue& val, ui32 nbFields, VisitorEvaluate* e = 0, Type* t = 0 )
   {
      assert( val.type == RuntimeValue::TYPE ); // it must be a TYPE value
      val.vals = RuntimeValue::RefcountedValues( e, t, new RuntimeValues( nbFields ) );
   }

   inline void createVector3i( RuntimeValue& val, int x, int y, int z )
   {
      val.type = RuntimeValue::TYPE;

      createFields( val, 1 );
      (*val.vals)[ 0 ].type = RuntimeValue::TYPE;
      createFields( (*val.vals)[ 0 ], 3 );

      (*(*val.vals)[ 0 ].vals)[ 0 ].intval = x;
      (*(*val.vals)[ 0 ].vals)[ 1 ].intval = y;
      (*(*val.vals)[ 0 ].vals)[ 2 ].intval = z;

      (*(*val.vals)[ 0 ].vals)[ 0 ].type = RuntimeValue::INT;
      (*(*val.vals)[ 0 ].vals)[ 1 ].type = RuntimeValue::INT;
      (*(*val.vals)[ 0 ].vals)[ 2 ].type = RuntimeValue::INT;
   }

   inline void createVector3f( RuntimeValue& val, float x, float y, float z )
   {
      val.type = RuntimeValue::TYPE;

      createFields( val, 1 );
      (*val.vals)[ 0 ].type = RuntimeValue::TYPE;
      createFields( (*val.vals)[ 0 ], 3 );

      (*(*val.vals)[ 0 ].vals)[ 0 ].floatval = x;
      (*(*val.vals)[ 0 ].vals)[ 1 ].floatval = y;
      (*(*val.vals)[ 0 ].vals)[ 2 ].floatval = z;

      (*(*val.vals)[ 0 ].vals)[ 0 ].type = RuntimeValue::FLOAT;
      (*(*val.vals)[ 0 ].vals)[ 1 ].type = RuntimeValue::FLOAT;
      (*(*val.vals)[ 0 ].vals)[ 2 ].type = RuntimeValue::FLOAT;
   }
}
}

#endif