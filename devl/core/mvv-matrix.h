#ifndef CORE_MVV_MATRIX_H_
# define CORE_MVV_MATRIX_H_

# include "core.h"
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

class FunctionRunnableMatrixfConstructor : public FunctionRunnable
{
public:
   typedef nll::core::Matrix<float> Pointee;

public:
   FunctionRunnableMatrixfConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 3 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      if ( v2.type != RuntimeValue::CMP_INT || v3.type != RuntimeValue::CMP_INT )
      {
         throw RuntimeException( "expecting (int, int)" );
      }

      Pointee* pointee = new Pointee( v2.intval, v3.intval );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionRunnableMatrixfDestructor: public FunctionRunnable
{
public:
   typedef FunctionRunnableMatrixfConstructor::Pointee Pointee;

public:
   FunctionRunnableMatrixfDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // deallocate data
      delete pointee;
      (*v1.vals)[ 0 ].ref = 0;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

#endif 