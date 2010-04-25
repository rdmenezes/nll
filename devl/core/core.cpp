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

      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.intval + v2.intval;
      return rt;
   }
};

class FunctionRunnableMinusII : public FunctionRunnable
{
public:
   FunctionRunnableMinusII( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      rt.intval = v1.intval - v2.intval;
      return rt;
   }
};

class FunctionRunnableMulII : public FunctionRunnable
{
public:
   FunctionRunnableMulII( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      rt.intval = v1.intval * v2.intval;
      return rt;
   }
};

class FunctionRunnableDivII : public FunctionRunnable
{
public:
   FunctionRunnableDivII( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v2.intval == 0 )
      {
         throw RuntimeException( "division by zero" );
      }

      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.intval / v2.intval;
      return rt;
   }
};

class FunctionRunnablePlusSS : public FunctionRunnable
{
public:
   FunctionRunnablePlusSS( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::STRING || v2.type != RuntimeValue::STRING  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 strings as arguments" );
      }

      RuntimeValue rt( RuntimeValue::STRING );
      rt.stringval = v1.stringval + v2.stringval;
      return rt;
   }
};

class FunctionRunnablePrintlnS : public FunctionRunnable
{
public:
   FunctionRunnablePrintlnS( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      if ( v1.type != RuntimeValue::STRING )
      {
         throw RuntimeException( "wrong arguments: expecting 1 string as arguments" );
      }

      std::cout << v1.stringval << std::endl;

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionRunnablePrintS : public FunctionRunnable
{
public:
   FunctionRunnablePrintS( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      if ( v1.type != RuntimeValue::STRING )
      {
         throw RuntimeException( "wrong arguments: expecting 1 string as arguments" );
      }

      std::cout << v1.stringval;

      RuntimeValue rt( RuntimeValue::EMPTY );
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
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator-" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMinusII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator*" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMulII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator/" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableDivII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator==" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableEqII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "println" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePrintlnS( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "print" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePrintS( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator+" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), new TypeString( false ) ) );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePlusSS( fn ) ) );
   }
}