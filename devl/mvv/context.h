#ifndef MVV_CONTEXT_H_
# define MVV_CONTEXT_H_

# include "symbol.h"
# include "resource.h"

namespace mvv
{
   /**
    @brief a Tag to specify the derived type is a context
    */
   class ContextInstance
   {
   public:
      virtual ~ContextInstance()
      {}
   };

   /**
    @brief A context holder
    */
   class _Context
   {
      typedef std::map<std::string, ContextInstance*>   ContextContainer;

   public:
      /**
       @brief deallocates all the resources
       */
      ~_Context()
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

   private:
      ContextContainer     _contexts;
   };

   typedef nll::core::Singleton<_Context> Context;


   /**
    @brief Get a MPR context. This context is local to this specific MPR object.
    */
   class ContextMpr : public ContextInstance
   {
   public:
      class ContextMprInstance
      {
      public:
         /**
          @param lut it must be an allocated pointer as it will be deallocated later on
          */
         void addVolume( MedicalVolume* volume, double volumeIntensity, ResourceLut* lut )
         {
            ensure( volume, "must not be empty" );
            volumes.attachVolume( volume );
            luts.addLut( volume, lut );
            volumeIntensities.addIntensity( volume, volumeIntensity );
         }

         void removeVolume( const MedicalVolume* volume )
         {
            ensure( volume, "must not be empty" );
            volumes.detachVolume( volume );
            luts.removeLut( volume );
            volumeIntensities.removeIntensity( volume );
         }

         /// a list of volumes attached to the MPR
         ResourceVolumes            volumes;

         /// the origin of the MPR
         ResourceVector3d           origin;

         /// the first vector of the MPR basis
         ResourceVector3d           vector1;

         /// the second vector of the MPR basis
         ResourceVector3d           vector2;

         /// the zoom factor of the MPR
         ResourceVector2d           zoom;

         /// the luts attached to the volumes. Every volume must have an attached lut!
         ResourceLuts               luts;

         /// intensities attached to a volume. Every volume must have an intensity.
         ResourceVolumeIntensities  volumeIntensities;
      };

   public:
      typedef std::map<Symbol, ContextMprInstance*>   Mprs;

      /**
       @param contextInstance must be an allocated pointer as it will be deallocated by the constructor
       */
      void addMpr( const Symbol& mprName, ContextMprInstance* contextInstance )
      {
         ensure( contextInstance, "must not be empty" );
         _mprs[ mprName ] = contextInstance;
      }

      /**
       @brief Returns a MPR according to its name. Return 0 if it doesnt exist
       */
      ContextMprInstance* getMpr( const Symbol& name )
      {
         Mprs::iterator it = _mprs.find( name );
         if ( it == _mprs.end() )
            return 0;
         return it->second;
      }

      /**
       @brief Remove a MPR
       */
      void removeMpr( const Symbol& name )
      {
         Mprs::iterator it = _mprs.find( name );
         if ( it == _mprs.end() )
            return;
         delete it->second;
         _mprs.erase( name );
      }

      ~ContextMpr()
      {
         for ( Mprs::iterator it = _mprs.begin(); it != _mprs.end(); ++it )
            delete it->second;
      }

      /**
       @brief Return an iterator on the list of MPR
       */
      Mprs::iterator begin()
      {
         return _mprs.begin();
      }

      /**
       @brief Return an end iterator on the list of MPR
       */
      Mprs::iterator end()
      {
         return _mprs.end();
      }

   private:
      Mprs     _mprs;
   };

   /**
    @brief Holds all the shared resources
    */
   class ContextGlobalResource : public ContextInstance
   {
      typedef std::map<Symbol, MedicalVolume*>  Volumes;

   public:
      ~ContextGlobalResource()
      {
         for ( Volumes::iterator it = _volumes.begin(); it != _volumes.end(); ++it )
            removeVolume( it->first );
      }

      /**
       @brief Add a volume
       @param name must be unique
       @param vol must be an allocated pointer. It will be freed with the destruction of this context.
       */
      void addVolume( const Symbol& name, MedicalVolume* vol )
      {
         ensure( vol, "can't be null" );
         _volumes[ name ] = vol;
      }

      void removeVolume( const Symbol& name )
      {
         Volumes::iterator it = _volumes.find( name );
         if ( it != _volumes.end() )
         {
            // get the pointer
            MedicalVolume* vol = it->second;
            _volumes.erase( it );

            // clear the other contexts
            ContextMpr* contextMpr = Context::instance().get<ContextMpr>();
            if ( contextMpr )
            {
               for ( ContextMpr::Mprs::iterator it = contextMpr->begin(); it != contextMpr->end(); ++it )
               {
                  it->second->removeVolume( vol );
               }
            }
         }
      }

   private:
      Volumes     _volumes;
   };
}

#endif