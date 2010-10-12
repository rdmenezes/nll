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

   inline void createFields( RuntimeValue& val, ui32 nbFields, RuntimeValue::TypeEnum t )
   {
      assert( val.type == RuntimeValue::TYPE ); // it must be a TYPE value
      RuntimeValues* values = new RuntimeValues( nbFields );
      for ( ui32 n = 0; n < nbFields; ++n )
      {
         (*values)[ n ].type = t;
      }
      val.vals = RuntimeValue::RefcountedValues( 0, 0, values );
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

      (*(*val.vals)[ 0 ].vals)[ 0 ].type = RuntimeValue::CMP_INT;
      (*(*val.vals)[ 0 ].vals)[ 1 ].type = RuntimeValue::CMP_INT;
      (*(*val.vals)[ 0 ].vals)[ 2 ].type = RuntimeValue::CMP_INT;
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

      (*(*val.vals)[ 0 ].vals)[ 0 ].type = RuntimeValue::CMP_FLOAT;
      (*(*val.vals)[ 0 ].vals)[ 1 ].type = RuntimeValue::CMP_FLOAT;
      (*(*val.vals)[ 0 ].vals)[ 2 ].type = RuntimeValue::CMP_FLOAT;
   }

   inline void createVector2f( RuntimeValue& val, float x, float y )
   {
      val.type = RuntimeValue::TYPE;

      createFields( val, 1 );
      (*val.vals)[ 0 ].type = RuntimeValue::TYPE;
      createFields( (*val.vals)[ 0 ], 2 );

      (*(*val.vals)[ 0 ].vals)[ 0 ].floatval = x;
      (*(*val.vals)[ 0 ].vals)[ 1 ].floatval = y;

      (*(*val.vals)[ 0 ].vals)[ 0 ].type = RuntimeValue::CMP_FLOAT;
      (*(*val.vals)[ 0 ].vals)[ 1 ].type = RuntimeValue::CMP_FLOAT;
   }

   inline void createMatrix3f( RuntimeValue& val, const nll::core::Matrix<float>& m )
   {
      assert( m.sizex() == 3 && m.sizey() == 3 );

      val.type = RuntimeValue::TYPE;

      createFields( val, 1 );
      (*val.vals)[ 0 ].type = RuntimeValue::TYPE;
      createFields( (*val.vals)[ 0 ], 9 );

      RuntimeValues& vals = (*(*val.vals)[ 0 ].vals);

      vals[ 0 ].floatval = m( 0, 0 );
      vals[ 1 ].floatval = m( 0, 1 );
      vals[ 2 ].floatval = m( 0, 2 );

      vals[ 3 ].floatval = m( 1, 0 );
      vals[ 4 ].floatval = m( 1, 1 );
      vals[ 5 ].floatval = m( 1, 2 );

      vals[ 6 ].floatval = m( 2, 0 );
      vals[ 7 ].floatval = m( 2, 1 );
      vals[ 8 ].floatval = m( 2, 2 );

      for ( int n = 0; n < 9; ++n )
         vals[ n ].type = RuntimeValue::CMP_FLOAT;
   }

   inline void createMatrix4f( RuntimeValue& val, const nll::core::Matrix<float>& m )
   {
      assert( m.sizex() == 4 && m.sizey() == 4 );

      val.type = RuntimeValue::TYPE;

      createFields( val, 1 );
      (*val.vals)[ 0 ].type = RuntimeValue::TYPE;
      createFields( (*val.vals)[ 0 ], 16 );

      RuntimeValues& vals = (*(*val.vals)[ 0 ].vals);

      vals[ 0 ].floatval = m( 0, 0 );
      vals[ 1 ].floatval = m( 0, 1 );
      vals[ 2 ].floatval = m( 0, 2 );
      vals[ 3 ].floatval = m( 0, 3 );

      vals[ 4 ].floatval = m( 1, 0 );
      vals[ 5 ].floatval = m( 1, 1 );
      vals[ 6 ].floatval = m( 1, 2 );
      vals[ 7 ].floatval = m( 1, 3 );

      vals[ 8  ].floatval = m( 2, 0 );
      vals[ 9  ].floatval = m( 2, 1 );
      vals[ 10 ].floatval = m( 2, 2 );
      vals[ 11 ].floatval = m( 2, 3 );

      vals[ 12 ].floatval = m( 3, 0 );
      vals[ 13 ].floatval = m( 3, 1 );
      vals[ 14 ].floatval = m( 3, 2 );
      vals[ 15 ].floatval = m( 3, 3 );

      for ( int n = 0; n < 16; ++n )
         vals[ n ].type = RuntimeValue::CMP_FLOAT;
   }

   inline void getVector3iValues( RuntimeValue& vector, nll::core::vector3i& out )
   {
      if ( vector.type != RuntimeValue::TYPE || (*vector.vals).size() != 1 ||
           (*vector.vals)[ 0 ].type != RuntimeValue::TYPE || (*(*vector.vals)[ 0 ].vals).size() != 3 )
      {
         throw std::runtime_error( "expected Vector" );
      }

      std::vector<RuntimeValue>& vec = (*(*vector.vals)[ 0 ].vals);
      out[ 0 ] = vec[ 0 ].intval;
      out[ 1 ] = vec[ 1 ].intval;
      out[ 2 ] = vec[ 2 ].intval;
   }

   inline void getVector2iValues( RuntimeValue& vector, nll::core::vector2i& out )
   {
      if ( vector.type != RuntimeValue::TYPE || (*vector.vals).size() != 1 ||
           (*vector.vals)[ 0 ].type != RuntimeValue::TYPE || (*(*vector.vals)[ 0 ].vals).size() != 2 )
      {
         throw std::runtime_error( "expected Vector" );
      }

      std::vector<RuntimeValue>& vec = (*(*vector.vals)[ 0 ].vals);
      out[ 0 ] = vec[ 0 ].intval;
      out[ 1 ] = vec[ 1 ].intval;
   }

   inline void getVector3fValues( RuntimeValue& vector, nll::core::vector3f& out )
   {
      if ( vector.type != RuntimeValue::TYPE || (*vector.vals).size() != 1 ||
           (*vector.vals)[ 0 ].type != RuntimeValue::TYPE || (*(*vector.vals)[ 0 ].vals).size() != 3 )
      {
         throw std::runtime_error( "expected Vector" );
      }

      std::vector<RuntimeValue>& vec = (*(*vector.vals)[ 0 ].vals);
      out[ 0 ] = vec[ 0 ].floatval;
      out[ 1 ] = vec[ 1 ].floatval;
      out[ 2 ] = vec[ 2 ].floatval;
   }

   inline void getMatrix4fValues( RuntimeValue& val, nll::core::Matrix<float>& m )
   {
      if ( val.type != RuntimeValue::TYPE || (*val.vals).size() != 1 ||
           (*val.vals)[ 0 ].type != RuntimeValue::TYPE || (*(*val.vals)[ 0 ].vals).size() != 16 )
      {
         throw std::runtime_error( "expected Matrix4f" );
      }

      m = nll::core::Matrix<float>( 4, 4 );
      RuntimeValues& vals = (*(*val.vals)[ 0 ].vals);

      m( 0, 0 ) = vals[ 0 ].floatval;
      m( 0, 1 ) = vals[ 1 ].floatval;
      m( 0, 2 ) = vals[ 2 ].floatval;
      m( 0, 3 ) = vals[ 3 ].floatval;

      m( 1, 0 ) = vals[ 4 ].floatval;
      m( 1, 1 ) = vals[ 5 ].floatval;
      m( 1, 2 ) = vals[ 6 ].floatval;
      m( 1, 3 ) = vals[ 7 ].floatval;

      m( 2, 0 ) = vals[ 8 ].floatval;
      m( 2, 1 ) = vals[ 9 ].floatval;
      m( 2, 2 ) = vals[ 10 ].floatval;
      m( 2, 3 ) = vals[ 11 ].floatval;

      m( 3, 0 ) = vals[ 12 ].floatval;
      m( 3, 1 ) = vals[ 13 ].floatval;
      m( 3, 2 ) = vals[ 14 ].floatval;
      m( 3, 3 ) = vals[ 15 ].floatval;
   }
}
}

#endif