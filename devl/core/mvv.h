#ifndef CORE_MVV_H_
# define CORE_MVV_H_

# include "core.h"
# include <mvvLauncher/init.h>
# include <mvvScript/function-runnable.h>

using namespace mvv::parser;
using namespace mvv;

class FunctionLoadVolumeS : public FunctionRunnable
{
public:
   FunctionLoadVolumeS( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      static ui32 id = 0;

      ++id;
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::STRING )
      {
         throw RuntimeException( "wrong arguments: expecting 1 string as arguments" );
      }

      applicationVariables->context.get<ContextTools>()->loadVolume( v1.stringval, mvv::SymbolVolume::create( nll::core::val2str( id ) ) );

      RuntimeValue rt( RuntimeValue::TYPE );
      rt.vals = RuntimeValue::RefcountedValues( 0, 0, new RuntimeValues( 1 ) );
      (*rt.vals)[ 0 ].setType( RuntimeValue::STRING );
      (*rt.vals)[ 0 ].stringval = nll::core::val2str( id );
      return rt;
   }
};

#endif
