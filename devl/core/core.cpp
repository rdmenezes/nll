// core.cpp : Defines the entry point for the DLL.
//

#include "stdafx.h"
#include "core.h"
#include "mvv.h"
#include "mvv-lut.h"
#include "mvv-volume-container.h"
#include "mvv-segment-tool.h"
#include "mvv-segment.h"
#include "mvv-layout.h"
#include <mvvScript/function-runnable.h>

using namespace mvv::parser;
using namespace mvv;


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

class FunctionRunnableRound : public FunctionRunnable
{
public:
   FunctionRunnableRound( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::FLOAT   )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = static_cast<int>( std::floor( v1.floatval + 0.5 ) );
      return rt;
   }
};

class FunctionRunnableFloor : public FunctionRunnable
{
public:
   FunctionRunnableFloor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::FLOAT   )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::INT );
      rt.intval = static_cast<int>( std::floor( v1.floatval ) );
      return rt;
   }
};

class FunctionRunnableToFloat : public FunctionRunnable
{
public:
   FunctionRunnableToFloat( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::INT   )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::FLOAT );
      rt.floatval = static_cast<float>( v1.intval );
      return rt;
   }
};

class FunctionRunnableToStringI : public FunctionRunnable
{
public:
   FunctionRunnableToStringI( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::INT   )
      {
         throw RuntimeException( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::STRING );
      rt.stringval = nll::core::val2str( v1.intval );
      return rt;
   }
};

class FunctionRunnableToStringF : public FunctionRunnable
{
public:
   FunctionRunnableToStringF( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::INT   )
      {
         throw RuntimeException( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::STRING );
      rt.stringval = nll::core::val2str( v1.floatval );
      return rt;
   }
};



void importFunctions( CompilerFrontEnd& e, mvv::platform::Context& context )
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

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "toFloat" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableToFloat( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "round" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableRound( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "floor" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableFloor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "toString" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableToStringF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "toString" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableToStringI( fn ) ) );
   }

   //
   // --------------------------
   // MVV bindings
   //

   //
   // volume loading
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "loadVolumeMF2" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLoadVolumeMF2( fn, context, e ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "getVolume" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionGetVolumeId( fn, context, &e.getEvaluator(), ty ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "loadVolumeAsynchronous" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLoadVolumeAsynchronous( fn, context, e ) ) );
   }

   //
   // Volume
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "getSize" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeGetSize( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "getValue" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeGetValue( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "setValue" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ), new TypeInt( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeSetValue( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "getSpacing" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeGetSpacing( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "getOrigin" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeGetOrigin( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "getRotation" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeGetRotation( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "getPst" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeGetPst( fn ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "setOrigin" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeSetOrigin( fn ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "setSpacing" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeSetSpacing( fn ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Matrix3f" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "setRotation" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeSetRotation( fn ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Matrix4f" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "setPst" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeSetPst( fn ) ) );
   }

   //
   // VolumeID
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "VolumeID"), platform::Symbol::create( "~VolumeID" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionVolumeIDDestructor( fn, context ) ) );
   }

   //
   // Lut
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "Lut" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutConstructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "~Lut" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "setColorIndex" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ), new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutSetColorIndex( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "getColorIndex" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutGetColorIndex( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "transform" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutTransform( fn ) ) );
   }

   //
   // VolumeContainer
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "VolumeContainer"), platform::Symbol::create( "VolumeContainer" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionVolumeContainerConstructor( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "VolumeContainer"), platform::Symbol::create( "~VolumeContainer" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionVolumeContainerDestructor( fn ) ) );
   }

   {
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      Type* lut = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Lut" ) ) ) );
      assert( lut && volumeId );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "VolumeContainer"), platform::Symbol::create( "add" ) ), nll::core::make_vector<const Type*>( volumeId, lut, new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionVolumeContainerAdd( fn ) ) );
   }

   {
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( volumeId );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "VolumeContainer"), platform::Symbol::create( "erase" ) ), nll::core::make_vector<const Type*>( volumeId ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionVolumeContainerErase( fn ) ) );
   }

   {
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      Type* lut = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Lut" ) ) ) );
      assert( volumeId && lut );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "VolumeContainer"), platform::Symbol::create( "setLut" ) ), nll::core::make_vector<const Type*>( volumeId, lut ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionVolumeContainerSetLut( fn ) ) );
   }

   {
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( volumeId );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "VolumeContainer"), platform::Symbol::create( "setIntensity" ) ), nll::core::make_vector<const Type*>( volumeId, new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionVolumeContainerSetIntensity( fn ) ) );
   }

   //
   // SegmentToolCentering
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolCentering"), platform::Symbol::create( "SegmentToolCentering" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolCenteringConstructor( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolCentering"), platform::Symbol::create( "~SegmentToolCentering" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolCenteringDestructor( fn ) ) );
   }

   //
   // SegmentToolPointer
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolPointer"), platform::Symbol::create( "SegmentToolPointer" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolPointerConstructor( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolPointer"), platform::Symbol::create( "~SegmentToolPointer" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolPointerDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolPointer"), platform::Symbol::create( "setPosition" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolPointerSetPosition( fn ) ) );
   }

   //
   // SegmentToolAnnotations
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolAnnotations"), platform::Symbol::create( "SegmentToolAnnotations" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolAnnotationsConstructor( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolAnnotations"), platform::Symbol::create( "~SegmentToolAnnotations" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolAnnotationsDestructor( fn ) ) );
   }

   {
      Type* vector3f = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      Type* vector3i = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( vector3f && vector3i );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolAnnotations"), platform::Symbol::create( "add" ) ), nll::core::make_vector<const Type*>( vector3f, new TypeString( false ), vector3i ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolAnnotationsAdd( fn, context ) ) );
   }

   {
      Type* annotationId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "SegmentToolAnnotations" ), mvv::Symbol::create( "AnnotationID" ) ) ) );
      assert( annotationId );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolAnnotations"), platform::Symbol::create( "erase" ) ), nll::core::make_vector<const Type*>( annotationId ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolAnnotationsErase( fn ) ) );
   }

   //
   // SegmentToolCamera
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolCamera"), platform::Symbol::create( "SegmentToolCamera" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolCameraConstructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "SegmentToolCamera"), platform::Symbol::create( "~SegmentToolCamera" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentToolCameraDestructor( fn ) ) );
   }

   //
   // Segment
   //
   {
      Type* container = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeContainer" ) ) ) );
      Type* vector3f = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "Segment" ) ), nll::core::make_vector<const Type*>( container, vector3f, vector3f, vector3f ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentConstructor( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "~Segment" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentDestructor( fn ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "SegmentToolPointer" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentSetToolPointer( fn ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "SegmentToolCamera" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentSetToolCamera( fn ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "SegmentToolCentering" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentSetToolAutocenter( fn ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "SegmentToolAnnotations" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentSetToolAnnotations( fn ) ) );
   }

   //
   // Layout
   //
   {
      Type* direction = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Layout" ), mvv::Symbol::create( "Vertical" ) ) ) );
      Type* layout = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Layout" ) ) ) );
      assert( layout && direction );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Layout"), platform::Symbol::create( "Layout" ) ), nll::core::make_vector<const Type*>( direction, layout, layout, new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLayoutConstructorVertical( fn ) ) );
   }

   {
      Type* direction = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Layout" ), mvv::Symbol::create( "Horizontal" ) ) ) );
      Type* layout = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Layout" ) ) ) );
      assert( layout && direction );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Layout"), platform::Symbol::create( "Layout" ) ), nll::core::make_vector<const Type*>( direction, layout, layout, new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLayoutConstructorHorizontal( fn ) ) );
   }

   {
      Type* segment = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Segment" ) ) ) );
      assert( segment );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Layout"), platform::Symbol::create( "Layout" ) ), nll::core::make_vector<const Type*>( segment ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLayoutConstructorSegment( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Layout"), platform::Symbol::create( "~Layout" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLayoutDestructor( fn ) ) );
   }
}