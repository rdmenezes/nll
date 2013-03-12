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
#include "mvv-mip-tools.h"
#include "mvv-affine-registration.h"
#include "mvv-special.h"
#include "mvv-file.h"
#include <mvvScript/function-runnable.h>
#include "system.h"
#include "mvv-resample.h"
#include "mvv-matrix.h"
#include "mvv-image.h"
#include "mvv-imagef.h"
#include "mvv-segment-tool-postprocessing.h"
#include "mvv-manipulators.h"
#include "mvv-mouse.h"
#include "mvv-geometry.h"
#include "mvv-registration.h"

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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      if ( v2.floatval == 0 )
      {
         throw std::runtime_error( "division by zero" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      if ( v2.floatval == 0 )
      {
         throw std::runtime_error( "division by zero" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      if ( v2.intval == 0 )
      {
         throw std::runtime_error( "division by zero" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }

      if ( v2.intval == 0 )
      {
         throw std::runtime_error( "division by zero" );
      }

      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::STRING || v2.type != RuntimeValue::STRING  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 strings as arguments" );
      }

      RuntimeValue rt( RuntimeValue::STRING );
      rt.stringval = v1.stringval + v2.stringval;
      return rt;
   }
};

class FunctionRunnablePrintlnS : public FunctionRunnable
{
public:
   FunctionRunnablePrintlnS( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      if ( v1.type != RuntimeValue::STRING )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 string as arguments" );
      }

      _e.getStdOut() << v1.stringval << std::endl;

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }

private:
   CompilerFrontEnd& _e;
};

class FunctionRunnablePrintS : public FunctionRunnable
{
public:
   FunctionRunnablePrintS( const AstDeclFun* fun, CompilerFrontEnd& e  ) : FunctionRunnable( fun ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      if ( v1.type != RuntimeValue::STRING )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 string as arguments" );
      }

      _e.getStdOut() << v1.stringval;

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }

private:
   CompilerFrontEnd& _e;
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = v1.intval == v2.intval;
      return rt;
   }
};

class FunctionRunnableEqSS : public FunctionRunnable
{
public:
   FunctionRunnableEqSS( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::STRING || v2.type != RuntimeValue::STRING  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 string as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = v1.stringval == v2.stringval;
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = fabs( v1.floatval - v2.floatval ) < 1e-6;
      return rt;
   }
};

class FunctionRunnableNEqSS : public FunctionRunnable
{
public:
   FunctionRunnableNEqSS( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::STRING || v2.type != RuntimeValue::STRING  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 string as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = v1.stringval != v2.stringval;
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::CMP_INT  )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_INT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = static_cast<float>( v1.intval );
      return rt;
   }
};

class FunctionRunnableToFloatS : public FunctionRunnable
{
public:
   FunctionRunnableToFloatS( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::STRING   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 string as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = atoi( v1.stringval.c_str() );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_INT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 int as arguments" );
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
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::STRING );
      rt.stringval = nll::core::val2str( v1.floatval );
      return rt;
   }
};

class FunctionRunnableCos : public FunctionRunnable
{
public:
   FunctionRunnableCos( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = cos( v1.floatval );
      return rt;
   }
};

class FunctionRunnableACos : public FunctionRunnable
{
public:
   FunctionRunnableACos( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = acos( v1.floatval );
      return rt;
   }
};

class FunctionRunnableASin : public FunctionRunnable
{
public:
   FunctionRunnableASin( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = asin( v1.floatval );
      return rt;
   }
};

class FunctionRunnableSqrt : public FunctionRunnable
{
public:
   FunctionRunnableSqrt( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = sqrt( v1.floatval );
      return rt;
   }
};

class FunctionRunnablePow : public FunctionRunnable
{
public:
   FunctionRunnablePow( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      // unref the values so that the actual RuntimeValues
      // are accessed and not a simple reference
      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT || v2.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = pow( v1.floatval, v2.floatval );
      return rt;
   }
};

class FunctionRunnableFAbs : public FunctionRunnable
{
public:
   FunctionRunnableFAbs( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = fabs( v1.floatval );
      return rt;
   }
};

class FunctionRunnableLog: public FunctionRunnable
{
public:
   FunctionRunnableLog( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = log( v1.floatval );
      return rt;
   }
};

class FunctionRunnableAbs : public FunctionRunnable
{
public:
   FunctionRunnableAbs( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_INT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = abs( v1.intval );
      return rt;
   }
};

class FunctionRunnableMod : public FunctionRunnable
{
public:
   FunctionRunnableMod( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v1.type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 int as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = v1.intval % v2.intval;
      return rt;
   }
};

class FunctionRunnableAssert : public FunctionRunnable
{
public:
   FunctionRunnableAssert( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_INT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 int as arguments" );
      }
      if ( v1.intval == 0 )
         throw std::runtime_error( "assert failed" );
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionRunnableAssert2 : public FunctionRunnable
{
public:
   FunctionRunnableAssert2( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::CMP_INT || v2.type != RuntimeValue::STRING )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 int as arguments" );
      }
      if ( v1.intval == 0 )
         throw std::runtime_error( ( "assert failed: " + v2.stringval ).c_str() );
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionRunnableExp : public FunctionRunnable
{
public:
   FunctionRunnableExp( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = exp( v1.floatval );
      return rt;
   }
};

class FunctionRunnableTan : public FunctionRunnable
{
public:
   FunctionRunnableTan( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = tan( v1.floatval );
      return rt;
   }
};

class FunctionRunnableATan : public FunctionRunnable
{
public:
   FunctionRunnableATan( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = atan( v1.floatval );
      return rt;
   }
};

class FunctionRunnableSin : public FunctionRunnable
{
public:
   FunctionRunnableSin( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = sin( v1.floatval );
      return rt;
   }
};

class FunctionRunnableSqr : public FunctionRunnable
{
public:
   FunctionRunnableSqr( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_FLOAT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float as arguments" );
      }
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = ( v1.floatval * v1.floatval );
      return rt;
   }
};

class FunctionRunnableRand : public FunctionRunnable
{
public:
   FunctionRunnableRand( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 0 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = rand() % 2147483647;
      return rt;
   }
};

class FunctionRunnableSrand : public FunctionRunnable
{
public:
   FunctionRunnableSrand( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::CMP_INT   )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 int as arguments" );
      }
      srand( v1.intval );
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionRunnableSplitString : public FunctionRunnable
{
public:
   FunctionRunnableSplitString( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      if ( v1.type != RuntimeValue::STRING || v2.type != RuntimeValue::STRING )
      {
         throw std::runtime_error( "wrong arguments: expecting 2 string as arguments" );
      }
      if ( v2.stringval.size() != 1 )
      {
         throw std::runtime_error( "second parameter must be a string of lenght one" );
      }

      std::string cpy = v1.stringval;
      std::vector<const char*> splits = nll::core::split( cpy, v2.stringval[ 0 ] );

      RuntimeValue rt( RuntimeValue::TYPE );
      createFields( rt, (unsigned)splits.size() );
      for ( int n = 0; n < splits.size(); ++n )
      {
         (*rt.vals)[ n ] = RuntimeValue( RuntimeValue::STRING );
         (*rt.vals)[ n ].stringval = splits[ n ];
      }
      return rt;
   }
};

class FunctionRunnableReplace : public FunctionRunnable
{
public:
   FunctionRunnableReplace( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 3 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      if ( v1.type != RuntimeValue::STRING || v2.type != RuntimeValue::STRING || v3.type != RuntimeValue::STRING )
      {
         throw std::runtime_error( "wrong arguments: expecting 3 string as arguments" );
      }
      if ( v2.stringval.size() != 1 )
      {
         throw std::runtime_error( "expecting one letter as argument" );
      }
      if ( v3.stringval.size() != 1 )
      {
         throw std::runtime_error( "expecting one letter as argument" );
      }
      for ( size_t n = 0; n < v1.stringval.size(); ++n )
      {
         if ( v1.stringval[ n ] == v2.stringval[ 0 ] )
         {
            v1.stringval[ n ] = v3.stringval[ 0 ];
         }
      }
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

static void redirect_stdout()
{
   static const char* tmpout = "stdout.txt";
   static const char* tmperr = "stderr.txt";
   FILE *stream ;
   stream = freopen( tmpout, "w", stdout );
   stream = freopen( tmperr, "w", stderr );
}


static std::pair<std::vector<std::string>,
                 std::vector<std::string> > restore_stdout()
{
   std::vector<std::string> strsout;
   std::vector<std::string> strserr;

   {
      static const char* tmpout = "stdout.txt";
      std::ifstream f( tmpout );
      std::string s;
      while ( !f.eof() )
      {
         getline( f, s );
         strsout.push_back( s );
      }
   }

   {
      static const char* tmperr = "stderr.txt";
      std::ifstream f( tmperr );
      std::string s;
      while ( !f.eof() )
      {
         getline( f, s );
         strserr.push_back( s );
      }
   }

#if defined( WIN32 ) || defined( WIN64 )
   freopen("CON", "w", stdout);
   freopen("CON", "w", stderr);
#else
   freopen("/dev/tty", "w", stdout);
   freopen("/dev/tty", "w", stderr);
#endif

   return std::make_pair( strsout, strserr );
}


class FunctionSize : public FunctionRunnable
{

public:
   FunctionSize( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments, expexted array" );
      }

      RuntimeValue& v0 = unref( *args[ 0 ] );

      RuntimeValue rt( RuntimeValue::CMP_INT );
      if ( v0.vals.getDataPtr() == 0 )
      {
         rt.intval = 0;
      } else {
         rt.intval = (int)(*v0.vals).size();
      }
      return rt;
   }
};


class FunctionRunnableSystem : public FunctionRunnable
{
public:
   FunctionRunnableSystem( const AstDeclFun* fun, mvv::platform::Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::STRING )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 string as arguments" );
      }
      platform::ContextGlobal* global = _context.get<platform::ContextGlobal>();
      if ( !global )
      {
         throw std::runtime_error( "mvv global context has not been initialized" );
      }
      
      redirect_stdout();
      mvv::systemCall( v1.stringval );

      std::pair<std::vector<std::string>,
                 std::vector<std::string> > result = restore_stdout();
      std::vector<std::string>& strsout = result.first;
      std::vector<std::string>& strserr = result.second;

      Pane* p = NULL;
      LayoutCommandLine* cmd = NULL;
      if ( global->layout.getDataPtr() )
      {
         p = (*global->layout).find( mvv::Symbol::create( "mvv::platform::LayoutCommandLine" ) );
         cmd = dynamic_cast<LayoutCommandLine*>( p );
      }

      if ( !cmd )
      {
         std::cout << "couldn't find a LayoutCommandLine to display system's output. Defaulted to command line" << std::endl;
      }

      if ( !p )
      {
         for ( std::vector<std::string>::reverse_iterator it = strsout.rbegin(); it != strsout.rend(); ++it )
         {
            if ( *it != "" )
               std::cout<< *it;
         }

         for ( std::vector<std::string>::reverse_iterator it = strserr.rbegin(); it != strserr.rend(); ++it )
         {
            if ( *it != "" )
               std::cout<< *it;
         }
      } else {
         for ( std::vector<std::string>::reverse_iterator it = strsout.rbegin(); it != strsout.rend(); ++it )
         {
            if ( *it != "" )
               cmd->sendMessage( *it, nll::core::vector3uc( 180, 180, 180 ) );
         }

         for ( std::vector<std::string>::reverse_iterator it = strserr.rbegin(); it != strserr.rend(); ++it )
         {
            if ( *it != "" )
               cmd->sendMessage( *it, nll::core::vector3uc( 255, 0, 0 ) );
         }
      }

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }

private:
   mvv::platform::Context&    _context;
};



void importFunctions( CompilerFrontEnd& e, mvv::platform::Context& context )
{
   std::cout << "core.dll import functions" << std::endl;
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "split" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableSplitString( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "system" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableSystem( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator+" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePlusFF( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "cos" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableCos( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "acos" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableACos( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "tan" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableTan( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "atan" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableATan( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "sin" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableSin( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "asin" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableASin( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "sqrt" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableSqrt( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "log" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableLog( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "pow" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePow( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "mod" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMod( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "abs" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableAbs( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "abs" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableFAbs( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "assert" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableAssert( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "assert" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableAssert2( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "sqr" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableSqr( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "srand" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableSrand( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "rand" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableRand( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "exp" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableExp( fn ) ) );
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
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator!=" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableNEqSS( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "operator==" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableEqSS( fn ) ) );
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
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePrintlnS( fn, e ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "print" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnablePrintS( fn, e ) ) );
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
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "toFloat" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableToFloatS( fn ) ) );
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
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "readVolumeMF2" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLoadVolumeMF2( fn, context, e ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "readVolumeTxt" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLoadVolumeTxt( fn, context, e ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "writeVolumeTxt" ) ), nll::core::make_vector<const Type*>( ty, new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionWriteTxtVolume( fn, context ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "writeVolumeBin" ) ), nll::core::make_vector<const Type*>( ty, new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionWriteBinVolume( fn, context ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "writeVolumeMF2" ) ), nll::core::make_vector<const Type*>( ty, new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionWriteMF2Volume( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "readVolumeBin" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLoadVolumeBin( fn, context, e ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "getVolume" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionGetVolumeId( fn, context, &e.getEvaluator(), ty ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "readVolumeMF2Asynchronous" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
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
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeSetOrigin( fn, context ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "indexToPosition" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeIndexToPosition( fn ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "positionToIndex" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumePositionToIndex( fn ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "setSpacing" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeSetSpacing( fn, context ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Matrix3f" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "setRotation" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeSetRotation( fn, context ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Matrix4f" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "setPst" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeSetPst( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "getBackgroundValue" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeGetBackgroundValue( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Volume"), platform::Symbol::create( "setBackgroundValue" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableVolumeSetBackgroundValue( fn, context ) ) );
   }

   {
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "findMaxVoxel") ), nll::core::make_vector<const Type*>( volumeId ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMaxVoxel( fn, context ) ) );
   }

   {
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "findMaxVoxel") ), nll::core::make_vector<const Type*>( volumeId, new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMaxVoxelSlice( fn, context ) ) );
   }

   //
   // VolumeID
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "VolumeID"), platform::Symbol::create( "~VolumeID" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionVolumeIDDestructor( fn, context ) ) );
   }

   {   
      Type* vector3i = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      Type* matrix4 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Matrix4f" ) ) ) );
      assert( vector3i && matrix4  );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "createVolume" ) ), nll::core::make_vector<const Type*>( vector3i, matrix4, new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionCreateVolume( fn, context, e ) ) );
   }

   //
   // Lut
   //
   {
      Type* vector3f = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( vector3f );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "Lut" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ), vector3f ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutConstructor( fn ) ) );
   }

   {
      Type* vector3f = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( vector3f );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "set" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeFloat( false ), vector3f ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutSet( fn ) ) );
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
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "getColor" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutGetColorIndex( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "transform" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutTransform( fn ) ) );
   }

   {
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "detectRange") ), nll::core::make_vector<const Type*>( volumeId, new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutDetectRange( fn, context ) ) );
   }

   {
      Type* color = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Lut"), platform::Symbol::create( "setVolcanoColor") ), nll::core::make_vector<const Type*>( color, new TypeFloat( false ), new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLutSetVolcanoColor( fn, context ) ) );
   }

   //
   // AffineRegistration
   //
   {
      Type* matrix = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Matrix4f" ) ) ) );
      assert( matrix );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "AffineRegistration"), platform::Symbol::create( "AffineRegistration" ) ), nll::core::make_vector<const Type*>( matrix ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionAffineRegistrationConstructor( fn ) ) );
   }

   {
      Type* matrix = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Matrix4f" ) ) ) );
      assert( matrix );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "AffineRegistration"), platform::Symbol::create( "setMatrix" ) ), nll::core::make_vector<const Type*>( matrix ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionAffineRegistrationSetMatrix( fn ) ) );
   }

   {
      Type* vector = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vector );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "AffineRegistration"), platform::Symbol::create( "transform" ) ), nll::core::make_vector<const Type*>( vector ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionAffineRegistrationTransform( fn ) ) );
   }

   {
      Type* vector = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vector );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "AffineRegistration"), platform::Symbol::create( "transformInverse" ) ), nll::core::make_vector<const Type*>( vector ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionAffineRegistrationTransformInverse( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "AffineRegistration"), platform::Symbol::create( "getMatrix" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionAffineRegistrationGetMatrix( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "AffineRegistration"), platform::Symbol::create( "~AffineRegistration" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionAffineRegistrationDestructor( fn ) ) );
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
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "VolumeContainer"), platform::Symbol::create( "clear" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionVolumeContainerClear( fn ) ) );
   }

   {
      Type* ar = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "AffineRegistration" ) ) ) );
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      Type* lut = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Lut" ) ) ) );
      assert( lut && volumeId && ar );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "VolumeContainer"), platform::Symbol::create( "add" ) ), nll::core::make_vector<const Type*>( volumeId, lut, new TypeFloat( false ), ar ) );
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
   // Segment   Centering
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
   // ToolAnnotations
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolAnnotations"), platform::Symbol::create( "ToolAnnotations" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolAnnotationsConstructor( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolAnnotations"), platform::Symbol::create( "~ToolAnnotations" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolAnnotationsDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolAnnotations"), platform::Symbol::create( "clear" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolAnnotationsClear( fn ) ) );
   }

   {
      Type* vector3f = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      Type* vector3i = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( vector3f && vector3i );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolAnnotations"), platform::Symbol::create( "add" ) ), nll::core::make_vector<const Type*>( vector3f, new TypeString( false ), vector3i, new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolAnnotationsAdd( fn, context ) ) );
   }

   {
      Type* vector3f = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      Type* vector3i = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( vector3f && vector3i );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolAnnotations"), platform::Symbol::create( "add" ) ), nll::core::make_vector<const Type*>( vector3f, vector3f, new TypeString( false ), vector3i, new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolAnnotationsAddLine( fn, context ) ) );
   }

   {
      Type* vector3f = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      Type* vector3i = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( vector3f && vector3i );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolAnnotations"), platform::Symbol::create( "addLinePos" ) ), nll::core::make_vector<const Type*>( vector3f, vector3f, new TypeString( false ), new TypeString( false ), vector3i ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolAnnotationsAddLinePos( fn, context ) ) );
   }

   {
      Type* vector3f = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      Type* vector3i = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( vector3f && vector3i );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolAnnotations"), platform::Symbol::create( "add" ) ), nll::core::make_vector<const Type*>( vector3f, vector3f, new TypeInt( false ), new TypeArray( 1, *vector3i, false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolAnnotationsAddColors( fn, context ) ) );
   }

   {
      Type* annotationId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ToolAnnotations" ), mvv::Symbol::create( "AnnotationID" ) ) ) );
      assert( annotationId );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolAnnotations"), platform::Symbol::create( "erase" ) ), nll::core::make_vector<const Type*>( annotationId ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolAnnotationsErase( fn ) ) );
   }

   {
      Type* annotationId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ToolAnnotations" ), mvv::Symbol::create( "AnnotationID" ) ) ) );
      Type* vector = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( annotationId && vector );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolAnnotations"), platform::Symbol::create( "setPosition" ) ), nll::core::make_vector<const Type*>( annotationId, vector ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolAnnotationsSetPosition( fn ) ) );
   }

   {
      Type* annotationId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ToolAnnotations" ), mvv::Symbol::create( "AnnotationID" ) ) ) );
      assert( annotationId );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolAnnotations"), platform::Symbol::create( "getPosition" ) ), nll::core::make_vector<const Type*>( annotationId ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolAnnotationsGetPosition( fn ) ) );
   }

   //
   // ToolPostprocessing
   //
   {
      Type* callback = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "PostprocessingCallback" ) ) ) );
      assert( callback );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolPostprocessing"), platform::Symbol::create( "ToolPostprocessing" ) ), nll::core::make_vector<const Type*>( callback ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolPostprocessingConstructor( fn, e ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolPostprocessing"), platform::Symbol::create( "~ToolPostprocessing" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolPostprocessingDestructor( fn ) ) );
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
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Segment" ), mvv::Symbol::create( "Nearest" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "setInterpolator" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentSetInterpolationNN( fn ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Segment" ), mvv::Symbol::create( "Linear" ) ) ) );
      assert( ty );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "setInterpolator" ) ), nll::core::make_vector<const Type*>( ty ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentSetInterpolationL( fn ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ToolPostprocessing" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentSetPostprocessing( fn, e ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "SegmentToolCentering" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentSetToolAutocenter( fn ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ToolAnnotations" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentSetToolAnnotations( fn ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ToolManipulators" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentSetToolManipulators( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "getRawImage" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentGetRawImage( fn, e ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "getImage" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentGetImage( fn, e ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( im );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Segment"), platform::Symbol::create( "getRawImage" ) ), nll::core::make_vector<const Type*>( im ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentGetRawImagef( fn, e ) ) );
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
      Type* holder = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ImageHolder" ) ) ) );
      assert( holder );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Layout"), platform::Symbol::create( "Layout" ) ), nll::core::make_vector<const Type*>( holder ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLayoutConstructorImage( fn, context ) ) );
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
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLayoutConstructorSegment( fn, context ) ) );
   }

   {
      Type* console = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Console" ) ) ) );
      assert( console );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Layout"), platform::Symbol::create( "Layout" ) ), nll::core::make_vector<const Type*>( console ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLayoutConstructorLayout( fn, context, e ) ) );
   }

   {
      Type* mip = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Mip" ) ) ) );
      assert( mip );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Layout"), platform::Symbol::create( "Layout" ) ), nll::core::make_vector<const Type*>( mip ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLayoutConstructorMip( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Layout"), platform::Symbol::create( "~Layout" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionLayoutDestructor( fn ) ) );
   }

   //
   // Mip
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "MipToolPointer"), platform::Symbol::create( "MipToolPointer" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMipToolPointerConstructor( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Mip"), platform::Symbol::create( "notify" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMipNotify( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "MipToolPointer"), platform::Symbol::create( "~MipToolPointer" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMipToolPointerDestructor( fn ) ) );
   }

   {
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      Type* lut = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Lut" ) ) ) );
      assert( volumeId && lut );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Mip"), platform::Symbol::create( "Mip" ) ), nll::core::make_vector<const Type*>( volumeId, lut, new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMipConstructor( fn, context ) ) );
   }

   {
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( volumeId );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Mip"), platform::Symbol::create( "setVolume" ) ), nll::core::make_vector<const Type*>( volumeId ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMipSetVolume( fn, context ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "MipToolPointer" ) ) ) );
      assert( tool );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Mip"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMipSetToolPointer( fn ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ToolAnnotations" ) ) ) );
      assert( tool );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Mip"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMipSetToolAnnotations( fn ) ) );
   }

   {
      Type* tool = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ToolPostprocessing" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Mip"), platform::Symbol::create( "setTool" ) ), nll::core::make_vector<const Type*>( tool ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMipSetPostprocessing( fn, e ) ) );
   }

   {
      Type* volume = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      Type* lut = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Lut" ) ) ) );
      assert( volume && lut );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "barycentre") ), nll::core::make_vector<const Type*>( volume, lut ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableBarycentre( fn, context ) ) );
   }

   //
   // OFStream
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "OFStream"), platform::Symbol::create( "OFStream" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionOFStreamConstructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "OFStream"), platform::Symbol::create( "~OFStream" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionOFStreamDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "OFStream"), platform::Symbol::create( "close" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionOFStreamClose( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "OFStream"), platform::Symbol::create( "write" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionOFStreamWriteString( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "OFStream"), platform::Symbol::create( "write" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionOFStreamWriteInt( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "OFStream"), platform::Symbol::create( "write" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionOFStreamWriteFloat( fn ) ) );
   }

   //
   // IFStream
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "IFStream"), platform::Symbol::create( "IFStream" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionIFStreamConstructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "IFStream"), platform::Symbol::create( "~IFStream" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionIFStreamDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "IFStream"), platform::Symbol::create( "close" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionIFStreamClose( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "IFStream"), platform::Symbol::create( "read" ) ), nll::core::make_vector<const Type*>( new TypeInt( true ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionIFStreamReadInt( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "IFStream"), platform::Symbol::create( "read" ) ), nll::core::make_vector<const Type*>( new TypeFloat( true ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionIFStreamReadFloat( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "IFStream"), platform::Symbol::create( "eof" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionIFStreamEof( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "IFStream"), platform::Symbol::create( "good" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionIFStreamGood( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "IFStream"), platform::Symbol::create( "getline" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionIFStreamGetline( fn ) ) );
   }

   //
   // Resampling: Nearest neighbour
   //
   {
      
      Type* reg = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "AffineRegistration" ) ) ) );
      Type* volid = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      Type* vector3i = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      Type* matrix4 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Matrix4f" ) ) ) );
      assert( volid && vector3i && matrix4  );

      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "resampleNearest" ) ), nll::core::make_vector<const Type*>( volid, reg, vector3i, matrix4 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionResampleNearest( fn, context, e ) ) );
   }

   //
   // matrix
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Matrixf"), platform::Symbol::create( "Matrixf" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMatrixfConstructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Matrixf"), platform::Symbol::create( "~Matrixf" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableMatrixfDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Matrixf"), platform::Symbol::create( "set" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMarixfSet( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Matrixf"), platform::Symbol::create( "get" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMarixfGet( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Matrixf"), platform::Symbol::create( "inverse" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMarixfInvert( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Matrixf"), platform::Symbol::create( "det" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMarixfDet( fn ) ) );
   }

   {
      Type* mat = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Matrixf" ) ) ) );
      assert( mat );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Matrixf"), platform::Symbol::create( "operator*" ) ), nll::core::make_vector<const Type*>( mat ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMarixfMul( fn ) ) );
   }

   {
      Type* mat = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Matrixf" ) ) ) );
      assert( mat );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "transpose") ), nll::core::make_vector<const Type*>( mat ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMatrixfTranspose( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Matrixf"), platform::Symbol::create( "sizex" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMatrixfSizex( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Matrixf"), platform::Symbol::create( "sizey" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionMatrixfSizey( fn ) ) );
   }

   //
   // Image
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Image"), platform::Symbol::create( "Image" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageConstructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Image"), platform::Symbol::create( "clone" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageClone( fn, e ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Image"), platform::Symbol::create( "~Image" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Image"), platform::Symbol::create( "set" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ), new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageSet( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Image"), platform::Symbol::create( "get" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageGet( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Image"), platform::Symbol::create( "sizec" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageGetSizec( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Image"), platform::Symbol::create( "sizey" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageGetSizey( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Image"), platform::Symbol::create( "sizex" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageGetSizex( fn ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      assert( im );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( mvv::Symbol::create( "Image" ), platform::Symbol::create( "set" ) ), nll::core::make_vector<const Type*>( im ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageSetImage( fn ) ) );
   }

   //
   // ImageHolder
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ImageHolder"), platform::Symbol::create( "ImageHolder" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageHolderConstructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ImageHolder"), platform::Symbol::create( "~ImageHolder" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageHolderDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ImageHolder"), platform::Symbol::create( "get" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageHolderGet( fn, e ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ImageHolder"), platform::Symbol::create( "set" ) ), nll::core::make_vector<const Type*>( im ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageHolderSet( fn ) ) );
   }

   //
   // Imagef
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Imagef"), platform::Symbol::create( "Imagef" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImagefConstructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Imagef"), platform::Symbol::create( "clone" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImagefClone( fn, e ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Imagef"), platform::Symbol::create( "~Imagef" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImagefDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Imagef"), platform::Symbol::create( "set" ) ), nll::core::make_vector<const Type*>( new TypeFloat( false ), new TypeInt( false ), new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImagefSet( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Imagef"), platform::Symbol::create( "get" ) ), nll::core::make_vector<const Type*>( new TypeInt( false ), new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImagefGet( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Imagef"), platform::Symbol::create( "sizec" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImagefGetSizec( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Imagef"), platform::Symbol::create( "sizey" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImagefGetSizey( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Imagef"), platform::Symbol::create( "sizex" ) ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImagefGetSizex( fn ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Imagef" ) ) ) );
      assert( im );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( mvv::Symbol::create( "Imagef" ), platform::Symbol::create( "set" ) ), nll::core::make_vector<const Type*>( im ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImagefSetImage( fn ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Imagef" ) ) ) );
      Type* lut = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Lut" ) ) ) );
      assert( im && lut );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( mvv::Symbol::create( "transform" ) ), nll::core::make_vector<const Type*>( im, lut ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionApplyLutImagef( fn, e ) ) );
   }

   //
   // image utils
   //
   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      assert( im );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "writeBmp") ), nll::core::make_vector<const Type*>( im, new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionWriteBmp( fn ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      assert( im );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "copy") ), nll::core::make_vector<const Type*>( im, im, new TypeInt( false ), new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionCopyImage( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "readBmp") ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionReadBmp( fn, e ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      assert( im );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "decolor") ), nll::core::make_vector<const Type*>( im ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageDecolor( fn ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      assert( im );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "extend") ), nll::core::make_vector<const Type*>( im, new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageExtend( fn ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector2i" ) ) ) );
      assert( im && vec );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "crop") ), nll::core::make_vector<const Type*>( im, vec, vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageCrop( fn ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector2i" ) ) ) );
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( im && vec && vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "drawRectangle") ), nll::core::make_vector<const Type*>( im, vec, vec, vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageDrawRectangle( fn ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector2i" ) ) ) );
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( im && vec && vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "drawLine") ), nll::core::make_vector<const Type*>( im, vec, vec, vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageDrawLine( fn ) ) );
   }

   {
      Type* im = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector2i" ) ) ) );
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( im && vec && vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "drawText") ), nll::core::make_vector<const Type*>( im, new TypeString( false ), vec, new TypeInt( false ), vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionImageDrawText( fn, context ) ) );
   }

   //
   // ManipulatorPoint
   //
   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      Type* vec3i = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( vec3 && vec3i );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorPoint"), platform::Symbol::create( "ManipulatorPoint") ), nll::core::make_vector<const Type*>( vec3, new TypeString( false ), new TypeInt( false ), vec3i, new TypeFloat( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorPointConstructor( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorPoint"), platform::Symbol::create( "~ManipulatorPoint") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorPointDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorPoint"), platform::Symbol::create( "getPosition") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorPointGetPosition( fn ) ) );
   }

   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorPoint"), platform::Symbol::create( "setPosition") ), nll::core::make_vector<const Type*>( vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorPointSetPosition( fn ) ) );
   }


   //
   // ManipulatorPointer
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorPointer"), platform::Symbol::create( "ManipulatorPointer") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorPointerConstructor( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorPointer"), platform::Symbol::create( "~ManipulatorPointer") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorPointerDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorPointer"), platform::Symbol::create( "getPosition") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorPointerGetPosition( fn ) ) );
   }

   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorPointer"), platform::Symbol::create( "setPosition") ), nll::core::make_vector<const Type*>( vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorPointerSetPosition( fn ) ) );
   }

   //
   // ToolManipulators
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "ToolManipulators") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorsConstructor( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "~ToolManipulators") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorsDestructor( fn ) ) );
   }

   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ManipulatorPoint" ) ) ) );
      assert( vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "add") ), nll::core::make_vector<const Type*>( vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorsAddPoint( fn ) ) );
   }

   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ManipulatorPointer" ) ) ) );
      assert( vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "add") ), nll::core::make_vector<const Type*>( vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorsAddPointer( fn ) ) );
   }

   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ManipulatorCuboid" ) ) ) );
      assert( vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "add") ), nll::core::make_vector<const Type*>( vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorsAddCuboid( fn ) ) );
   }

   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ManipulatorPointer" ) ) ) );
      assert( vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "erase") ), nll::core::make_vector<const Type*>( vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorsErasePointer( fn ) ) );
   }

   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ManipulatorPoint" ) ) ) );
      assert( vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "erase") ), nll::core::make_vector<const Type*>( vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorsErasePoint( fn ) ) );
   }

   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "ManipulatorCuboid" ) ) ) );
      assert( vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "erase") ), nll::core::make_vector<const Type*>( vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorsEraseCuboid( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "clear") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorsClear( fn ) ) );
   }

   /*
   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vec3 );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "setPointerPosition") ), nll::core::make_vector<const Type*>( vec3 ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorSetPosition( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "getPointerPosition") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorGetPosition( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ToolManipulators"), platform::Symbol::create( "notify") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionToolManipulatorsNotify( fn ) ) );
   }*/

   //
   // ManipulatorCuboid
   //
   {
      Type* vec3 = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      Type* vec3i = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      assert( vec3 && vec3i );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorCuboid"), platform::Symbol::create( "ManipulatorCuboid") ), nll::core::make_vector<const Type*>( vec3, vec3, vec3i ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorCuboidConstructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorCuboid"), platform::Symbol::create( "~ManipulatorCuboid") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorCuboidDestructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorCuboid"), platform::Symbol::create( "getP1") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorCuboidGetPoint1( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorCuboid"), platform::Symbol::create( "getP2") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorCuboidGetPoint2( fn ) ) );
   }

   {
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vec );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorCuboid"), platform::Symbol::create( "setP1") ), nll::core::make_vector<const Type*>( vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorCuboidSetPoint1( fn ) ) );
   }

   {
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vec );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "ManipulatorCuboid"), platform::Symbol::create( "setP2") ), nll::core::make_vector<const Type*>( vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionManipulatorCuboidSetPoint2( fn ) ) );
   }


   //
   // Mouse
   //
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "setMousePointer") ), nll::core::make_vector<const Type*>( new TypeInt( false ) ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSetMousePointer( fn, context ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "getMousePointer") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionGetMousePointer( fn, context ) ) );
   }

   {
      Type* mouse = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "MouseCallback" ) ) ) );
      assert( mouse );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "setOnSegmentLeftMouseClick") ), nll::core::make_vector<const Type*>( mouse ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentMouseOnLeftClick( fn, e, context ) ) );
   }

   {
      Type* mouse = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "MouseCallback" ) ) ) );
      assert( mouse );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "setOnSegmentRightMouseClick") ), nll::core::make_vector<const Type*>( mouse ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentMouseOnRightClick( fn, e, context ) ) );
   }

   {
      Type* mouse = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "MouseCallback" ) ) ) );
      assert( mouse );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "setOnSegmentRightMouseRelease") ), nll::core::make_vector<const Type*>( mouse ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentMouseOnRightRelease( fn, e, context ) ) );
   }

   {
      Type* mouse = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "MouseCallback" ) ) ) );
      assert( mouse );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "setOnSegmentLeftMouseRelease") ), nll::core::make_vector<const Type*>( mouse ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentMouseOnLeftRelease( fn, e, context ) ) );
   }

   {
      Type* mouse = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "MouseCallback" ) ) ) );
      assert( mouse );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "setOnSegmentLeftMousePressed") ), nll::core::make_vector<const Type*>( mouse ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentMouseOnLeftPressed( fn, e, context ) ) );
   }

   {
      Type* mouse = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "MouseCallback" ) ) ) );
      assert( mouse );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "setOnSegmentRightMousePressed") ), nll::core::make_vector<const Type*>( mouse ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSegmentMouseOnRightPressed( fn, e, context ) ) );
   }

   //
   // geometry
   //
   {
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vec );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "Plane") ), nll::core::make_vector<const Type*>( vec, vec, vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlaneConstructor( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "~Plane") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlaneDestructor( fn ) ) );
   }

   {
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vec );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "orthogonalProjection") ), nll::core::make_vector<const Type*>( vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlaneOrthogonalProjection( fn ) ) );
   }

   {
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vec );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "worldToPlaneCoordinate") ), nll::core::make_vector<const Type*>( vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlaneWorldToPlaneCoordinate( fn ) ) );
   }

   {
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector2f" ) ) ) );
      assert( vec );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "planeToWorldCoordinate") ), nll::core::make_vector<const Type*>( vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlanePlaneToWorldCoordinate( fn ) ) );
   }

   {
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vec );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "getIntersection") ), nll::core::make_vector<const Type*>( vec, vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlaneGetIntersection( fn ) ) );
   }

   {
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3f" ) ) ) );
      assert( vec );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "contains") ), nll::core::make_vector<const Type*>( vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlaneContains( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "getAxisX") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlaneGetAxisX( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "getAxisY") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlaneGetAxisY( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "getNormal") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlaneGetNormal( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "Plane"), platform::Symbol::create( "getOrigin") ), std::vector<const Type*>() );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionPlaneGetOrigin( fn ) ) );
   }

   {
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( volumeId );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "affineIsotropicPlanarRegistrationCt") ), nll::core::make_vector<const Type*>( volumeId, volumeId, new TypeInt( false ), vec, vec, vec, vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRegistrationAffineIstropicPlanar( fn,context, &e.getEvaluator(), e ) ) );
   }

   {
      Type* vec = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Vector3i" ) ) ) );
      Type* volumeId = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      assert( volumeId );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "affinePlanarRegistrationCt") ), nll::core::make_vector<const Type*>( volumeId, volumeId, new TypeInt( false ), vec, vec, vec, vec ) );
      assert( fn );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRegistrationAffinePlanar( fn,context, &e.getEvaluator(), e ) ) );
   }

   {
      Type* ty = const_cast<Type*>( e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );
      TypeArray* arrayty = new TypeArray( 0, *ty, false );
      ensure( ty, "can't find 'VolumeID' in srouce" );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "size" ) ), nll::core::make_vector<const Type*>( arrayty ) );
      ensure( fn, "can't find the function declaration in core.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSize( fn ) ) );
   }

   {
      TypeArray* arrayty = new TypeArray( 0, *new TypeInt( false ), false );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "size" ) ), nll::core::make_vector<const Type*>( arrayty ) );
      ensure( fn, "can't find the function declaration in core.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSize( fn ) ) );
   }

   {
      TypeArray* arrayty = new TypeArray( 0, *new TypeFloat( false ), false );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "size" ) ), nll::core::make_vector<const Type*>( arrayty ) );
      ensure( fn, "can't find the function declaration in core.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSize( fn ) ) );
   }

   {
      TypeArray* arrayty = new TypeArray( 0, *new TypeString( false ), false );
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "size" ) ), nll::core::make_vector<const Type*>( arrayty ) );
      ensure( fn, "can't find the function declaration in core.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSize( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "replace" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), new TypeString( false ), new TypeString( false ) ) );
      ensure( fn, "can't find the function declaration in core.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionRunnableReplace( fn ) ) );
   }

   std::cout << "core.dll import functions done" << std::endl;
}