#ifndef CORE_MVV_SPECIAL_H_
# define CORE_MVV_SPECIAL_H_

# include "core.h"
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include <mvvPlatform/resource-typedef.h>
# include <mvvPlatform/resource-volumes.h>
# include <mvvPlatform/resource-storage-volumes.h>
# include "mvv-affine-registration.h"
# include "mvv-lut.h"

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

class FunctionRunnableBarycentre : public FunctionRunnable
{
   typedef platform::ResourceLut Pointee;

public:
   FunctionRunnableBarycentre( const AstDeclFun* fun, mvv::platform::Context& context ) : FunctionRunnable( fun ), _context( context )
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
      if ( v1.type != RuntimeValue::TYPE && (*v1.vals).size() == 1 && (*v1.vals)[ 0 ].type == RuntimeValue::STRING )
      {
         throw RuntimeException( "invalid argument" );
      }

      ContextTools* tools = _context.get<ContextTools>();
      if ( !tools )
      {
         throw RuntimeException( "ContextTools context has not been loaded" );
      }

      // it is guaranteed we have a volume
      mvv::platform::RefcountedTyped<Volume> vol = tools->getVolume( mvv::SymbolVolume::create( (*v1.vals)[ 0 ].stringval ) );

      ensure( (*v2.vals)[ 0 ].type == RuntimeValue::PTR, "compiler error: must be a pointer type" ); // it must be 1 field, PTR type
      Pointee* lut = reinterpret_cast<Pointee*>( (*v2.vals)[ 0 ].ref );
      ensure( lut, "compiler error: must be != 0" );

      nll::core::vector3f b = nll::imaging::computeBarycentre( *vol, *lut );
      
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, b[ 0 ], b[ 1 ], b[ 2 ] );
      return rt;
   }
   
private:
   mvv::platform::Context&    _context;
};

#endif 