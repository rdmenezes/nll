#ifndef MVV_PLATFORM_CONTEXT_H_
# define MVV_PLATFORM_CONTEXT_H_

# include <map>
# include <string>
# include <iostream>
# include "mvvPlatform.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief a Tag to specify the derived type is a context
    */
   class MVVPLATFORM_API ContextInstance
   {
   public:
      virtual ~ContextInstance()
      {}
   };

   /**
    @ingroup algoritm
    @brief Hold a set of <code>ContextInstance</code>.
    @note ContextInstance must be allocated and it will automatically be deallocated when the context instance is destroyed
    */
   class MVVPLATFORM_API Context
   {
      typedef std::map<std::string, ContextInstance*>   ContextContainer;

   public:
      /**
       @brief deallocates all the resources
       */
      ~Context()
      {
         clear();
      }

      /**
       @brief Stores a context. If one already existed, it is destroyed.
       @param context the context. It will be deallocated when the context is destroyed.
       */
      template <class T> void add( T* context )
      {
         ensure( context, "must not be empty" );
         const std::string s = typeid( T ).name();
         ContextContainer::iterator it = _contexts.find( s );
         if ( it != _contexts.end() )
            delete it->second;
         _contexts[ s ] = context;
      }

      /**
       @brief returns the specified context
       */
      template <class T> T* get() const
      {
         const std::string s = typeid( T ).name();
         ContextContainer::const_iterator it = _contexts.find( s );
         if ( it == _contexts.end() )
            return 0;
         return dynamic_cast<T*>( it->second );
      }

      template <class T> void get( T*& out ) const
      {
         out = get<T>();
      }

      void clear()
      {
         for ( ContextContainer::iterator it = _contexts.begin(); it != _contexts.end(); ++it )
         {
            delete it->second;
         }
         _contexts.clear();
      }

   private:
      ContextContainer     _contexts;
   };
}
}

#endif