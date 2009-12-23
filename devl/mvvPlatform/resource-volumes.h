#ifndef MVV_PLATFORM_RESOURCE_VOLUME_H_
# define MVV_PLATFORM_RESOURCE_VOLUME_H_

# include "resource-set.h"
# include "resource-storage-volumes.h"

namespace mvv
{
namespace platform
{
   // TODO test multiple reference?

   /**
    @brief Hold reference volumes stored in a specific storage.

    The principle is for example for a Segment, it is holding reference to volumes.
    In case the storage doesn't contain the volume anymore (but still referenced somewhere else), 
    we can remove volumes safely
    */
   class MVVPLATFORM_API ResourceVolumes : public ResourceSet< SymbolVolume >
   {
      typedef ResourceSet< SymbolVolume > Base;

   public:
      class Iterator
      {
      public:
         Iterator( ResourceStorageVolumes storage, Storage::iterator it, Storage::iterator end ) : _storage( storage ), _it( it ), _end( end )
         {
            // init
            _storage.find( *_it, _vol );
         }

         bool operator==( const Iterator& rhs )
         {
            return _it == rhs._it;
         }

         Iterator& operator++()
         {
            ++_it;
            // we need to run throu the volume list and look it up in the storage
            bool res = _storage.find( *_it, _vol );
            while ( ( !res || _vol.isEmpty() ) && _it != _end )
            {
               // skip if non valid reference or reference not found
               ++_it;
               res = _storage.find( *_it, _vol );
            }
            return *this;
         }

         RefcountedTyped<Volume> operator*()
         {
            assert( _vol.isEmpty() );
            return _vol;
         }

      private:
         ResourceStorageVolumes  _storage;
         Storage::iterator       _it;
         Storage::iterator       _end;
         RefcountedTyped<Volume> _vol;
      };

      ResourceVolumes( ResourceStorageVolumes volumeStorage ) : _volumeStorage( volumeStorage )
      {
         // nothing to do
      }

      bool find( SymbolVolume name, RefcountedTyped<Volume>& volume )
      {
         return _volumeStorage.find( name, volume );
      }

   private:
      ResourceStorageVolumes     _volumeStorage;
   };
}
}

#endif