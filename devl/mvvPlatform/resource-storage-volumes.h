#ifndef MVV_PLATFORM_RESOURCE_STORAGE_VOLUMES_H_
# define MVV_PLATFORM_RESOURCE_STORAGE_VOLUMES_H_

# include "context.h"
# include "symbol-typed.h"
# include "types.h"
# include "resource.h"
# include <map>

namespace boost
{
   class mutex;
}

namespace mvv
{
namespace platform
{
   namespace impl
   {
      struct ResourceVolumesStorage
      {
         ResourceVolumesStorage();
         ~ResourceVolumesStorage();

         typedef std::map<SymbolVolume, RefcountedTyped<Volume> > Storage;
         Storage      volumes;
         boost::mutex* mutex;
      };
   }

   /**
    @ingroup platform
    @brief Hold a set of volumes. They are referenced by a unique name and are refcounted.

    Each time a volume is inserted/removed, the resource is notified.
    */
   class MVVPLATFORM_API ResourceStorageVolumes : public Resource<impl::ResourceVolumesStorage>
   {
      typedef impl::ResourceVolumesStorage::Storage Storage;

   public:
      /**
       @brief Defines an iterator over all the volumes
       */
      class Iterator
      {
      public:
         Iterator( Storage::iterator i ) : _it( i )
         {}

         Iterator& operator++()
         {
            ++_it;
            return *this;
         }

         bool operator==( const Iterator& rhs ) const
         {
            return _it == rhs._it;
         }

         bool operator!=( const Iterator& rhs ) const
         {
            return _it != rhs._it;
         }

         RefcountedTyped<Volume> operator*()
         {
            return (*_it).second;
         }

         Storage::iterator _it;
      };

      /**
       @brief Defines an iterator over all the volumes
       */
      class ConstIterator
      {
      public:
         ConstIterator( Storage::const_iterator i ) : _it( i )
         {}

         ConstIterator( Iterator it ) : _it( it._it )
         {
         }

         ConstIterator& operator++()
         {
            ++_it;
            return *this;
         }

         bool operator==( const ConstIterator& rhs ) const
         {
            return _it == rhs._it;
         }

         bool operator!=( const ConstIterator& rhs ) const
         {
            return _it != rhs._it;
         }

         RefcountedTyped<Volume> operator*() const
         {
            return (*_it).second;
         }
      private:
         Storage::const_iterator _it;
      };

   public:
      /**
       @brief Construct an empty volume resource
       */
      ResourceStorageVolumes() : Resource<impl::ResourceVolumesStorage>( new impl::ResourceVolumesStorage(), true )
      {
      }

      /**
       @brief Insert a volume
       */
      void insert( SymbolVolume name, RefcountedTyped<Volume> volume );

      /**
       @brief Preallocate a name but with no associated volume

       This is useful when we want to load a serie of volume and we are waiting for all of them to be loadded
       before doing some processing...
       */ 
      void insert( SymbolVolume name );

      /**
       @brief Returns true if a volume is null
       */
      bool hasEmptyVolume() const;

      /**
       @brief remove a volume
       */
      void erase( SymbolVolume name );

      /**
       @brief Find a symbol and return its associated volume. If not found, false is returned
       */
      bool find( SymbolVolume name, RefcountedTyped<Volume>& volume );

      Iterator begin()
      {
         return Iterator( getValue().volumes.begin() );
      }

      Iterator end()
      {
         return Iterator( getValue().volumes.end() );
      }

      ConstIterator begin() const
      {
         return ConstIterator( getValue().volumes.begin() );
      }

      ConstIterator end() const
      {
         return ConstIterator( getValue().volumes.end() );
      }

      size_t size() const
      {
         return getValue().volumes.size();
      }
   };
}
}

#endif
