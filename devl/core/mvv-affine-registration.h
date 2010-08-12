#ifndef CORE_MVV_AFFINE_REGISTRATION_H_
# define CORE_MVV_AFFINE_REGISTRATION_H_

# include "core.h"
# include <mvvPlatform/resource-typedef.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

using namespace mvv::parser;
using namespace mvv;

namespace nll
{
namespace core
{
   // always force notification...
   inline bool operator!= ( const Matrixf&, const Matrixf& )
   {
      return true;
   }
}
}
class FunctionAffineRegistrationConstructor : public FunctionRunnable
{
public:
   typedef platform::ResourceRegistration Pointee;

public:
   FunctionAffineRegistrationConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v2.type != RuntimeValue::TYPE )
      {
         throw RuntimeException( "wrong arguments: expecting 1 Matrix4f" );
      }

      nll::core::Matrixf matrix;
      getMatrix4fValues( v2, matrix );

      // construct the type
      Pointee* ar = new Pointee();
      ar->setValue( matrix );
      std::cout << "create affine registration=" << ar->getDataPtr() << std::endl;
      ar->getValue().print( std::cout );


      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( ar ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionAffineRegistrationDestructor : public FunctionRunnable
{
   typedef platform::ResourceRegistration Pointee;

public:
   FunctionAffineRegistrationDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* ar = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // deallocate data
      delete ar;
      (*v1.vals)[ 0 ].ref = 0;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionAffineRegistrationSetMatrix: public FunctionRunnable
{
   typedef platform::ResourceRegistration Pointee;

public:
   FunctionAffineRegistrationSetMatrix( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v2.type != RuntimeValue::TYPE )
      {
         throw RuntimeException( "wrong arguments: expecting 1 Matrix4f" );
      }

      nll::core::Matrixf matrix;
      getMatrix4fValues( v2, matrix );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* reg = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // update the value
      reg->setValue( matrix );
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionAffineRegistrationGetMatrix: public FunctionRunnable
{
   typedef platform::ResourceRegistration Pointee;

public:
   FunctionAffineRegistrationGetMatrix( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* reg = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // update the value
      nll::core::Matrixf matrix = reg->getValue();

      RuntimeValue rt( RuntimeValue::TYPE );
      createMatrix4f( rt, matrix );
      return rt;
   }
};

#endif