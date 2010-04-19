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
      if ( args.size() != 2 || args[ 0 ]->type != RuntimeValue::INT || args[ 1 ]->type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT, 0 );
      rt.intval = args[ 0 ]->intval + args[ 1 ]->intval;
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
      if ( args.size() != 2 || args[ 0 ]->type != RuntimeValue::INT || args[ 1 ]->type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT, 0 );
      rt.intval = args[ 0 ]->intval == args[ 1 ]->intval;
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