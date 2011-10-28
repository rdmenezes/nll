#ifndef CORE_MVV_RESAMPLE_H_
# define CORE_MVV_RESAMPLE_H_

# include "core.h"
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include "mvv.h"

using namespace mvv::parser;
using namespace mvv;

class FunctionResampleNearest: public FunctionRunnable
{
public:
   FunctionResampleNearest( const AstDeclFun* fun, mvv::platform::Context& context, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _context( context ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 2 ] );
      RuntimeValue& v3 = unref( *args[ 3 ] );
      RuntimeValue& v4 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::TYPE && (*v1.vals).size() == 1 && (*v1.vals)[ 0 ].type == RuntimeValue::STRING )
      {
         throw std::runtime_error( "first argument has not a VolumeID object layout" );
      }
      nll::core::vector3i size;
      getVector3iValues( v2, size );

      nll::core::Matrixf m;
      getMatrix4fValues( v3, m );

      typedef FunctionAffineRegistrationConstructor::Pointee Pointee;
      Pointee* reg = reinterpret_cast<Pointee*>( (*v4.vals)[ 0 ].ref );

     
      // get the target volume
      ContextTools* tools = _context.get<ContextTools>();
      if ( !tools )
      {
         throw std::runtime_error( "ContextTools context has not been loaded" );
      }

      ContextVolumes* volumes = _context.get<ContextVolumes>();
      if ( !volumes )
      {
         throw std::runtime_error( "ContextVolumes context has not been loaded" );
      }

      // it is guaranteed we have a volume
      mvv::platform::RefcountedTyped<Volume> vol = tools->getVolume( mvv::SymbolVolume::create( (*v1.vals)[ 0 ].stringval ) );


      // create the UID
      ++volumeId;
      std::string volumeIdStr = nll::core::val2str( volumeId );
      mvv::platform::RefcountedTyped<Volume> resampled( new Volume( nll::core::vector3ui( size[ 0 ], size[ 1 ], size[ 2 ] ), m, (*vol).getBackgroundValue(), false ) );
      nll::imaging::resampleVolumeNearestNeighbour( *vol, reg->getValue(), *resampled );

      volumes->volumes.insert( mvv::SymbolVolume::create( volumeIdStr ), resampled );
      
      // create the volume ID
      RuntimeValue rt( RuntimeValue::TYPE );
      Type* ty = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("VolumeID") ) ) );
      assert( ty );

      rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), ty, new RuntimeValues( 1 ) );
      (*rt.vals)[ 0 ].setType( RuntimeValue::STRING );
      (*rt.vals)[ 0 ].stringval = nll::core::val2str( volumeId );

      std::cout << "created volume:" << volumeId << std::endl;
      return rt;
   }

private:
   mvv::platform::Context& _context;
   CompilerFrontEnd&       _e;
};

#endif