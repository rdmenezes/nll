# pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data

#include "resource-storage-volumes.h"
#include <boost/thread/mutex.hpp>

namespace mvv
{
namespace platform
{
   namespace impl
   {
      ResourceVolumesStorage::ResourceVolumesStorage()
      {
         mutex = new boost::mutex();
      }

      ResourceVolumesStorage::~ResourceVolumesStorage()
      {
         delete mutex;
      }
   }

   void ResourceStorageVolumes::insert( SymbolVolume name, RefcountedTyped<Volume> volume )
   {
      boost::mutex::scoped_lock( *getValue().mutex );

      getValue().volumes[ name ] = volume;

      notify();
   }

   /**
    @brief Preallocate a name but with no associated volume

    This is useful when we want to load a serie of volume and we are waiting for all of them to be loadded
    before doing some processing...
    */ 
   void ResourceStorageVolumes::insert( SymbolVolume name )
   {
      boost::mutex::scoped_lock( *getValue().mutex );

      getValue().volumes[ name ] = RefcountedTyped<Volume>();

      notify();
   }

   /**
    @brief Returns true if a volume is null
    */
   bool ResourceStorageVolumes::hasEmptyVolume() const
   {
      boost::mutex::scoped_lock( *getValue().mutex );

      for ( Storage::const_iterator it = getValue().volumes.begin(); it != getValue().volumes.end(); ++it )
      {
         if ( it->second.isEmpty() )
            return true;
      }
      return false;
   }

   /**
    @brief remove a volume
    */
   void ResourceStorageVolumes::erase( SymbolVolume name )
   {
      boost::mutex::scoped_lock( *getValue().mutex );

      Storage::iterator it = getValue().volumes.find( name );
      if ( it == getValue().volumes.end() )
         return;
      std::cout << "erase volume nb ref=" << it->second.getNumberOfReference() << std::endl;
      if ( it != getValue().volumes.end() )
      {
         getValue().volumes.erase( it );
      }

      notify();
   }

   /**
    @brief Find a symbol and return its associated volume. If not found, false is returned
    */
   bool ResourceStorageVolumes::find( SymbolVolume name, RefcountedTyped<Volume>& volume )
   {
      boost::mutex::scoped_lock( *getValue().mutex );

      Storage::iterator it = getValue().volumes.find( name );
      if ( it != getValue().volumes.end() )
      {
         volume = it->second;
         return true;
      }
      return false;
   }
}
}