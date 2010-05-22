#ifndef MVV_PLATFORN_CONTEXT_TOOLS_H_
# define MVV_PLATFORN_CONTEXT_TOOLS_H_

# include "mvvPlatform.h"
# include "context.h"
# include "engine-order-volume-loading.h"

namespace mvv
{
namespace platform
{
   class MVVPLATFORM_API ContextTools : public ContextInstance
   {
   public:
      ContextTools( ResourceStorageVolumes& storage, EngineHandler& handler, OrderProvider& provider, OrderDispatcher& dispatcher ) : _loader( storage, handler, provider, dispatcher )
      {
         handler.connect( _loader );
      }

      void loadVolume( const std::string& path, SymbolVolume name )
      {
         _loader.loadVolume( path, name );
      }

      RefcountedTyped<Volume> getVolume( SymbolVolume name )
      {
         return _loader.getVolume( name );
      }

   private:
      EngineOrderVolumeLoader    _loader;
   };
}
}

#endif