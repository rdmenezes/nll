#ifndef MVV_PLATFORM_RESOURCE_BARRIER_H_
# define MVV_PLATFORM_RESOURCE_BARRIER_H_

# include "resource.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief This class will prevent any notification of the engines connected to a resource when alive.

    This class will save the state of the resource, set it to deactivated and when destroyed, the resource
    will notify all connected engines if it has been modified (only if it's old state is 'enabled')

    This class is useful if the resource needs to be modified a lot of time, but prevent the engines to
    be notified each time.

    @note the resource's state must not be changed when a ResourceBarrier is activated as it will be lost...

    */
   class MVVPLATFORM_API ResourceScopedBarrier
   {
   public:
      ResourceScopedBarrier( impl::Resource resource ) : _resource( resource ), _initialState( resource.getState() )
      {
         _resource.setState( STATE_DISABLED );
      }

      ~ResourceScopedBarrier()
      {
         _resource.setState( _initialState );
      }

   private:
      // disable copy
      ResourceScopedBarrier& operator=( const ResourceScopedBarrier& );
      ResourceScopedBarrier( const ResourceScopedBarrier& );

   private:
      ResourceState  _initialState;
      impl::Resource _resource;
   };


   /**
    @ingroup platform
    @brief This class will prevent any notification of the engines connected to a resource when alive.

    This class is useful if the resource needs to be modified a lot of time, but prevent the engines to
    be notified each time.

    @note the resource's state must not be changed when a ResourceBarrier is activated as it will be lost...
    */
   class MVVPLATFORM_API ResourceBarrier
   {
   public:
      ResourceBarrier() : _resource( 0 )
      {
      }

      void connect( impl::Resource resource )
      {
         disconnect();
         _resource = resource;
         _initialState = resource.getState();
      }

      void disconnect()
      {
         if ( !_resource.isEmpty() )
         {
            _resource.setState( _initialState );
            _resource.unref();
         }
      }

      ~ResourceBarrier()
      {
         disconnect();
      }

   private:
      // disable copy
      ResourceBarrier& operator=( const ResourceBarrier& );
      ResourceBarrier( const ResourceBarrier& );

   private:
      ResourceState  _initialState;
      impl::Resource _resource;
   };
}
}

#endif