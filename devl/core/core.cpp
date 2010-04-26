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
class FunctionRunnablePlusFF : public FunctionRunnable
{
public:
   FunctionRunnablePlusFF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.floatval + v2.floatval;
      return rt;
   }
};

class FunctionRunnableMinusFF : public FunctionRunnable
{
public:
   FunctionRunnableMinusFF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.floatval - v2.floatval;
      return rt;
   }
};

class FunctionRunnableMulFF : public FunctionRunnable
{
public:
   FunctionRunnableMulFF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.floatval * v2.floatval;
      return rt;
   }
};

class FunctionRunnableDivFF : public FunctionRunnable
{
public:
   FunctionRunnableDivFF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      if ( v2.floatval == 0 )
      {
         throw RuntimeException( "division by zero" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.floatval / v2.floatval;
      return rt;
   }
};

class FunctionRunnablePlusIF : public FunctionRunnable
{
public:
   FunctionRunnablePlusIF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.intval + v2.floatval;
      return rt;
   }
};

class FunctionRunnableMinusIF : public FunctionRunnable
{
public:
   FunctionRunnableMinusIF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.intval - v2.floatval;
      return rt;
   }
};

class FunctionRunnableMulIF : public FunctionRunnable
{
public:
   FunctionRunnableMulIF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.intval * v2.floatval;
      return rt;
   }
};

class FunctionRunnableDivIF : public FunctionRunnable
{
public:
   FunctionRunnableDivIF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      if ( v2.floatval == 0 )
      {
         throw RuntimeException( "division by zero" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.intval / v2.floatval;
      return rt;
   }
};

class FunctionRunnablePlusFI : public FunctionRunnable
{
public:
   FunctionRunnablePlusFI( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.floatval + v2.intval;
      return rt;
   }
};

class FunctionRunnableMinusFI : public FunctionRunnable
{
public:
   FunctionRunnableMinusFI( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.floatval - v2.intval;
      return rt;
   }
};

class FunctionRunnableMulFI : public FunctionRunnable
{
public:
   FunctionRunnableMulFI( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.floatval * v2.intval;
      return rt;
   }
};

class FunctionRunnableDivFI : public FunctionRunnable
{
public:
   FunctionRunnableDivFI( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }

      if ( v2.intval == 0 )
      {
         throw RuntimeException( "division by zero" );
      }

      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = v1.floatval / v2.intval;
      return rt;
   }
};

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

class FunctionRunnableEqFI : public FunctionRunnable
{
public:
   FunctionRunnableEqFI( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = fabs( v1.floatval - static_cast<float>( v2.intval ) ) < 1e-6;
      return rt;
   }
};

class FunctionRunnableEqIF : public FunctionRunnable
{
public:
   FunctionRunnableEqIF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = fabs( v2.floatval - static_cast<float>( v1.intval ) ) < 1e-6;
      return rt;
   }
};

class FunctionRunnableEqFF : public FunctionRunnable
{
public:
   FunctionRunnableEqFF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = fabs( v1.floatval - v2.floatval ) < 1e-6;
      return rt;
   }
};

class FunctionRunnableNEqII : public FunctionRunnable
{
public:
   FunctionRunnableNEqII( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      rt.intval = v1.intval != v2.intval;
      return rt;
   }
};

class FunctionRunnableNEqFI : public FunctionRunnable
{
public:
   FunctionRunnableNEqFI( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = fabs( v1.floatval - static_cast<float>( v2.intval ) ) > 1e-6;
      return rt;
   }
};

class FunctionRunnableNEqIF : public FunctionRunnable
{
public:
   FunctionRunnableNEqIF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = fabs( v2.floatval - static_cast<float>( v1.intval ) ) > 1e-6;
      return rt;
   }
};

class FunctionRunnableNEqFF : public FunctionRunnable
{
public:
   FunctionRunnableNEqFF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = fabs( v1.floatval - v2.floatval ) > 1e-6;
      return rt;
   }
};

class FunctionRunnableLEqII : public FunctionRunnable
{
public:
   FunctionRunnableLEqII( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      rt.intval = v1.intval <= v2.intval;
      return rt;
   }
};

class FunctionRunnableLEqFI : public FunctionRunnable
{
public:
   FunctionRunnableLEqFI( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.floatval <= static_cast<float>( v2.intval );
      return rt;
   }
};

class FunctionRunnableLEqIF : public FunctionRunnable
{
public:
   FunctionRunnableLEqIF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v2.floatval <= static_cast<float>( v1.intval );
      return rt;
   }
};

class FunctionRunnableLEqFF : public FunctionRunnable
{
public:
   FunctionRunnableLEqFF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.floatval <= v2.floatval;
      return rt;
   }
};

class FunctionRunnableGEqII : public FunctionRunnable
{
public:
   FunctionRunnableGEqII( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      rt.intval = v1.intval >= v2.intval;
      return rt;
   }
};

class FunctionRunnableGEqFI : public FunctionRunnable
{
public:
   FunctionRunnableGEqFI( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.floatval >= static_cast<float>( v2.intval );
      return rt;
   }
};

class FunctionRunnableGEqIF : public FunctionRunnable
{
public:
   FunctionRunnableGEqIF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v2.floatval >= static_cast<float>( v1.intval );
      return rt;
   }
};

class FunctionRunnableGEqFF : public FunctionRunnable
{
public:
   FunctionRunnableGEqFF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.floatval >= v2.floatval;
      return rt;
   }
};

class FunctionRunnableLTII : public FunctionRunnable
{
public:
   FunctionRunnableLTII( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      rt.intval = v1.intval < v2.intval;
      return rt;
   }
};

class FunctionRunnableLTFI : public FunctionRunnable
{
public:
   FunctionRunnableLTFI( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.floatval < static_cast<float>( v2.intval );
      return rt;
   }
};

class FunctionRunnableLTIF : public FunctionRunnable
{
public:
   FunctionRunnableLTIF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v2.floatval < static_cast<float>( v1.intval );
      return rt;
   }
};

class FunctionRunnableLTFF : public FunctionRunnable
{
public:
   FunctionRunnableLTFF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.floatval < v2.floatval;
      return rt;
   }
};

class FunctionRunnableGTII : public FunctionRunnable
{
public:
   FunctionRunnableGTII( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      rt.intval = v1.intval > v2.intval;
      return rt;
   }
};

class FunctionRunnableGTFI : public FunctionRunnable
{
public:
   FunctionRunnableGTFI( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.floatval > static_cast<float>( v2.intval );
      return rt;
   }
};

class FunctionRunnableGTIF : public FunctionRunnable
{
public:
   FunctionRunnableGTIF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::INT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v2.floatval > static_cast<float>( v1.intval );
      return rt;
   }
};

class FunctionRunnableGTFF : public FunctionRunnable
{
public:
   FunctionRunnableGTFF( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v1.type != RuntimeValue::FLOAT || v2.type != RuntimeValue::FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = v1.floatval > v2.floatval;
      return rt;
   }
};

class FunctionRunnableAnd : public FunctionRunnable
{
public:
   FunctionRunnableAnd( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      rt.intval = v1.intval && v2.intval;
      return rt;
   }
};

class FunctionRunnableOr : public FunctionRunnable
{
public:
   FunctionRunnableOr( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      rt.intval = v1.intval || v2.intval;
      return rt;
   }
};



void importFunctions( CompilerFrontEnd& e)
{
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator+" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePlusFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator-" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMinusFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator*" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMulFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator/" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableDivFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator+" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePlusIF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator-" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMinusIF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator*" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMulIF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator/" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableDivIF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator+" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePlusFI( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator-" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMinusFI( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator*" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMulFI( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator/" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableDivFI( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator+" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePlusII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator-" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMinusII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator*" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMulII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator/" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableDivII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator==" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableEqII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator==" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableEqIF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator==" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableEqFI( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator==" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableEqFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator!=" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableNEqII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator!=" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableNEqIF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator!=" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableNEqFI( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator!=" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableNEqFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator<=" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableLEqII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator<=" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableLEqIF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator<=" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableLEqFI( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator<=" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableLEqFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator>=" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableGEqII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator>=" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableGEqIF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator>=" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableGEqFI( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator>=" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableGEqFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator<" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableLTII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator<" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableLTIF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator<" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableLTFI( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator<" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableLTFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator>" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableGTII( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator>" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableGTIF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator>" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableGTFI( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator>" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableGTFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "println" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePrintlnS( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "print" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePrintS( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator+" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePlusSS( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator&&" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableAnd( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator||" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableOr( fn ) ) );
   }
}