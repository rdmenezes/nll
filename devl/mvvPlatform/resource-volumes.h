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
   // TODO test multiple reference?

   /**
    @brief Hold reference volumes stored in a specific storage.

    The principle is for example for a Segment, it is holding reference to volumes.
    In case the storage doesn't contain the volume anymore (but still referenced somewhere else), 
    we can remove volumes safely
    */
   class MVVPLATFORM_API ResourceVolumes : public Resource< impl::ResourceVolumesList >
   {
      typedef ResourceSet< SymbolVolume > Base;

   public:
      class Iterator
      {
      public:
         Iterator( ResourceStorageVolumes storage, ResourceSet< SymbolVolume >::Storage::iterator it, ResourceSet< SymbolVolume >::Storage::iterator end ) : _storage( storage ), _it( it ), _end( end )
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
    //           std::cout << "val=" << (*_it).getName() << std::endl;
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
       //     std::cout << "found=" << res << std::endl;
       //     std::cout << "val=" << (*_it).getName() << std::endl;
            while ( ( !res || _vol.isEmpty() ) && _it != _end )
            {
               // skip if non valid reference or reference not found
       //        std::cout << "skip:" << &*( _vol ) << std::endl;
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

      ResourceVolumes( ResourceStorageVolumes volumeStorage ) : Resource( new impl::ResourceVolumesList )
      {
         getValue()._volumeStorage = volumeStorage;
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
         return getValue()._volumeStorage.size();
      }

      ResourceSet< SymbolVolume >& getVolumes()
      {
         return getValue()._volumes;
      }

   private:
      //ResourceStorageVolumes     _volumeStorage;
   };
}
}

#endif