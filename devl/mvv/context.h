#ifndef MVV_CONTEXT_H_
# define MVV_CONTEXT_H_

namespace mvv
{
   class ContextInstance
   {
   public:
      virtual ~ContextInstance()
      {}
   };

   class Context
   {
      typedef std::map<std::string, ContextInstance*>   ContextContainer;

   public:
      /**
       @brief deallocates all the resources
       */
      ~Context()
      {
         for ( ContextContainer::iterator it = _contexts.begin(); it != _contexts.end(); ++it )
            delete it->second;
      }

      /**
       @brief Stores a context. If one already existed, it is destroyed.
       @param context the context. It will be deallocated when the context is destroyed.
       */
      template <class T> void add( T* context )
      {
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

   private:
      ContextContainer     _contexts;
   };
}

#endif