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
   FunctionLoadVolumeS( const AstDeclFun* fun, mvv::platform::Context& context ) : FunctionRunnable( fun ), _context( context )
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

      ContextTools* tools = _context.get<ContextTools>();
      if ( !tools )
      {
         throw RuntimeException( "mvv context has not been loaded" );
      }
      tools->loadVolume( v1.stringval, mvv::SymbolVolume::create( nll::core::val2str( id ) ) );

      RuntimeValue rt( RuntimeValue::TYPE );
      rt.vals = RuntimeValue::RefcountedValues( 0, 0, new RuntimeValues( 1 ) );
      (*rt.vals)[ 0 ].setType( RuntimeValue::STRING );
      (*rt.vals)[ 0 ].stringval = nll::core::val2str( id );
      return rt;
   }

private:
   mvv::platform::Context&  _context;
};

class FunctionGetVolumeId : public FunctionRunnable
{
public:
   FunctionGetVolumeId( const AstDeclFun* fun, mvv::platform::Context& context ) : FunctionRunnable( fun ), _context( context )
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
      if ( v1.type != RuntimeValue::TYPE && (*v1.vals).size() == 1 && (*v1.vals)[ 0 ].type == RuntimeValue::STRING )
      {
         throw RuntimeException( "invalid argument" );
      }

      ContextTools* tools = _context.get<ContextTools>();
      if ( !tools )
      {
         throw RuntimeException( "mvv context has not been loaded" );
      }
      
      mvv::platform::RefcountedTyped<Volume> vol = tools->getVolume( mvv::SymbolVolume::create( v1.stringval ) );

      RuntimeValue rt( RuntimeValue::TYPE );
      assert( 0 );
      // TODO build the volume
      return rt;
   }

private:
   mvv::platform::Context&  _context;
};

#endif
