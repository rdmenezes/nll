#ifndef MVV_PLATFORM_REFCOUNTED_H_
# define MVV_PLATFORM_REFCOUNTED_H_

# include <assert.h>
# include <iostream>
# include <nll/nll.h>
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
         mutable int ref;
         bool     own;
         void*    extension;

         Internals()
         {
            data = 0;
            ref = 0;
            own = false;
            extension = 0;
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
         //std::cout << "refcounted destructor" << std::endl;
         unref();
      }

      void ref()
      {
         //#pragma omp critical
         {
            if ( !_data )
            {
               _data = new Internals();
            }
            ++_data->ref;
         }
      }

      bool isEmpty() const
      {
         return _data == 0;
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
         //#pragma omp critical
         {
            if ( _data )
            {
               assert( _data->ref > 0 );
               --_data->ref;
               if ( !_data->ref )
               {
                  // if a virtual failed here, it means we are calling inherited destroy() from
                  // the destructor of refcounted, which is wrong: the derived class has been
                  // deconstructed hence the error
                  destroy();
               }

               _data = 0;
            }
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
       @brief Free up all resources allocated
       */
      virtual void destroy() = 0;
      //{
      //   delete _data;
      //}

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
         //assert( rhs._data );

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

      virtual ~RefcountedTyped()
      {
         if ( _data )
         {
            // we unref here to avoid virtual call to destroy! a second unref() might be run
            // in the base destructor but this doesn't do anything.
            unref();
         }
      }

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

      T* getDataPtr()
      {
         return reinterpret_cast<T*>( _data->data );
      }

      const T* getDataPtr() const
      {
         return reinterpret_cast<T*>( _data->data );
      }

      const T& getData() const
      {
         return *reinterpret_cast<T*>( _data->data );
      }

      T& operator*()
      {
         ensure( getNumberOfReference(), "invalid operation" );
         return *reinterpret_cast<T*>( _data->data );
      }

      T* operator&()
      {
         return reinterpret_cast<T*>( _data->data );
      }

      const T& operator*() const
      {
         ensure( getNumberOfReference(), "invalid operation" );
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