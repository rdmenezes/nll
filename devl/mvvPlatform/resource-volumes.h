#ifndef MVV_PLATFORM_RESOURCE_VOLUME_H_
# define MVV_PLATFORM_RESOURCE_VOLUME_H_

# include "resource-set.h"
# include "resource-storage-volumes.h"

namespace mvv
{
namespace platform
{
   namespace impl
   {
      struct ResourceVolumesList
      {
         ResourceStorageVolumes        _volumeStorage;
         ResourceSet< SymbolVolume >   _volumes;
      };
   }

   /**
    @brief Hold reference volumes stored in a specific storage. A volume name can be potentially
           associated with an empty volume (example: we preallocate the name for loading, but
           voxel data is not yet available)
    @note the volumes should share the same storage, but with different ResourceVolumes for each segment-timepoint
          for example
    */
   class MVVPLATFORM_API ResourceVolumes : public Resource< impl::ResourceVolumesList >
   {
      typedef ResourceSet< SymbolVolume > Base;

   public:
      class Iterator
      {
      public:
         Iterator( ResourceStorageVolumes& storage, ResourceSet< SymbolVolume >::Storage::iterator it, ResourceSet< SymbolVolume >::Storage::iterator end ) : _storage( storage ), _it( it ), _end( end )
         {
            // init
            if ( _it != _end )
            {
               bool res = _storage.find( *_it, _vol );
               while ( ( !res || _vol.isEmpty() ) && _it != _end )
               {
                  // skip if non valid reference or reference not found
                  ++_it;
                  if ( _it == _end )
                     break;
                  res = _storage.find( *_it, _vol );
               }
            }
         }

         bool operator==( const Iterator& rhs ) const
         {
            return _it == rhs._it;
         }

         bool operator!=( const Iterator& rhs ) const
         {
            return _it != rhs._it;
         }

         Iterator& operator++()
         {
            ++_it;

            // we need to run throu the volume list and look it up in the storage
            if ( _it == _end )
               return *this;
            bool res = _storage.find( *_it, _vol );
            while ( ( !res || _vol.isEmpty() ) && _it != _end )
            {
               ++_it;
               if ( _it == _end )
                  return *this;
               res = _storage.find( *_it, _vol );
            }
            return *this;
         }

         RefcountedTyped<Volume> operator*()
         {
            assert( !_vol.isEmpty() );
            return _vol;
         }

         SymbolVolume getName() const
         {
            return *_it;
         }



      private:
         ResourceStorageVolumes  _storage;
         ResourceSet< SymbolVolume >::Storage::iterator       _it;
         ResourceSet< SymbolVolume >::Storage::iterator       _end;
         RefcountedTyped<Volume> _vol;
      };

      ResourceVolumes( ResourceStorageVolumes& volumeStorage ) : Resource( new impl::ResourceVolumesList )
      {
         volumeStorage.connect( *this );
         getValue()._volumeStorage = volumeStorage;
      }

      ResourceStorageVolumes& getStorage()
      {
         return getValue()._volumeStorage;
      }

      bool find( SymbolVolume name, RefcountedTyped<Volume>& volume )
      {
         return getValue()._volumeStorage.find( name, volume );
      }

      Iterator begin()
      {
         return Iterator( getValue()._volumeStorage, getValue()._volumes.begin(), getValue()._volumes.end() );
      }

      Iterator end()
      {
         return Iterator( getValue()._volumeStorage, getValue()._volumes.end(), getValue()._volumes.end() );
      }

      size_t size() const
      {
         return getValue()._volumes.size();
      }

      void insert( SymbolVolume val )
      {
         getValue()._volumes.insert( val );
         notify();
      }

      void erase( SymbolVolume val )
      {
         getValue()._volumes.erase( val );
         notify();
      }
   };
}
}

#endif