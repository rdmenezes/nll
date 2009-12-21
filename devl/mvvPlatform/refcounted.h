#ifndef MVV_PLATFORM_REFCOUNTED_H_
# define MVV_PLATFORM_REFCOUNTED_H_

# include <assert.h>
# include "mvvPlatform.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Reference counted object
    @note should only hold pointers

    Stores as void* and must be handled by the user. This class should not be used by the client, but instead
    use the derived type safe class
    */
   class MVVPLATFORM_API Refcounted
   {
      struct      Internals
      {
         void*    data;
         mutable unsigned ref;
         bool     own;

         Internals()
         {
            data = 0;
            ref = 0;
            own = false;
         }
      };

   public:
      Refcounted( const Refcounted& rhs ) : _data( 0 )
      {
         copy( rhs );
      }

      Refcounted& operator=( const Refcounted& rhs )
      {
         copy( rhs );
         return *this;
      }

      Refcounted() : _data( 0 )
      {
         ref();
      }

      virtual ~Refcounted()
      {
         unref();
      }

      void ref()
      {
         if ( !_data )
         {
            _data = new Internals();
         }
         ++_data->ref;
      }

      bool operator<( const Refcounted& r ) const
      {
         return _data < r._data;
      }

      bool operator==( const Refcounted& r ) const
      {
         return _data == r._data;
      }

      bool operator!=( const Refcounted& r ) const
      {
         return _data != r._data;
      }

      void unref()
      {
         if ( _data )
         {
            assert( _data->ref );
            --_data->ref;
            if ( !_data->ref )
            {
               destroy();
            }

            _data = 0;
         }
      }

      bool own() const
      {
         if ( _data )
         {
            return _data->own;
         } else return false;
      }

      /**
       @brief Frre up all resources allocated
       */
      virtual void destroy()
      {
         delete _data;
      }

      unsigned getNumberOfReference() const
      {
         if ( _data )
         {
            return _data->ref;
         } else {
            return 0;
         }
      }

   protected:
      void copy( const Refcounted& rhs )
      {
         // first check if we can really copy this reference
         assert( rhs._data );

         // if same pointee, then do nothing
         if ( rhs._data == _data )
         {
            return;
         }

         // unref the current and copy the rhs
         unref();
         if ( rhs._data )
         {
            _data = rhs._data;
            ref();
         }
      }

   protected:
      Internals*  _data;
   };

   /**
    @ingroup platform
    @brief Typed Reference counted object
    */
   template <class T>
   class RefcountedTyped : public Refcounted
   {
   public:
      typedef T   value_type;

      RefcountedTyped()
      {}

      RefcountedTyped( T* data, bool own = true )
      {
         _data->own = own;
         _data->data = data;
      }

      virtual void destroy()
      {
         if ( _data )
         {
            if ( _data->own )
            {
               delete reinterpret_cast<T*>( _data->data );
            }
            delete _data;
            _data = 0;
         }
      }

      T& getData()
      {
         return *reinterpret_cast<T*>( _data->data );
      }

      const T& getData() const
      {
         return *reinterpret_cast<T*>( _data->data );
      }

      T& operator*()
      {
         return *reinterpret_cast<T*>( _data->data );
      }

      T* operator&()
      {
         return reinterpret_cast<T*>( _data->data );
      }

      const T& operator*() const
      {
         return *reinterpret_cast<T*>( _data->data );
      }

      const T* operator&() const
      {
         return reinterpret_cast<T*>( _data->data );
      }

      /*
      const T& operator->() const
      {
         return *reinterpret_cast<T*>( _data->data );
      }

      T& operator->()
      {
         return *reinterpret_cast<T*>( _data->data );
      }*/
   };
}
}

#endif