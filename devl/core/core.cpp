// core.cpp : Defines the entry point for the DLL.
//

#include "stdafx.h"
#include "core.h"
#include <mvvScript/function-runnable.h>

using namespace mvv::parser;
using namespace mvv;

/*
#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif
*/
/*
import int operator+( int a, int b );
import int operator-( int a, int b );
import int operator/( int a, int b );
import int operator*( int a, int b );

import float operator+( float a, int b );
import float operator-( float a, int b );
import float operator/( float a, int b );
import float operator*( float a, int b );

import float operator+( float a, float b );
import float operator-( float a, float b );
import float operator/( float a, float b );
import float operator*( float a, float b );

import float operator+( int a, float b );
import float operator-( int a, float b );
import float operator/( int a, float b );
import float operator*( int a, float b );
*/

//
// TODO: remove the checks in release mode
//

class FunctionRunnablePlusII : public FunctionRunnable
{
public:
   FunctionRunnablePlusII( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      std::cout << "operator+ for int, int" << std::endl;
      VisitorEvaluate::_debug( v1 );
      VisitorEvaluate::_debug( v2 );
      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.intval + v2.intval;
      return rt;
   }
};

class FunctionRunnableEqII : public FunctionRunnable
{
public:
   FunctionRunnableEqII( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.intval == v2.intval;
      return rt;
   }
};


void importFunctions( CompilerFrontEnd& e)
{
   std::cout << "load core functions" << std::endl;

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator+" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePlusII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator==" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableEqII( fn ) ) );
   }
}