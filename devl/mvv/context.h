#ifndef MVV_CONTEXT_H_
# define MVV_CONTEXT_H_

# include "symbol.h"
# include "resource.h"

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

   /**
    @brief Get a MPR context. This context is local to this specific MPR object.
    */
   class ContextMpr : public ContextInstance
   {
   public:
      class ContextMprInstance
      {
         /**
          @param lut it must be an allocated pointer as it will be deallocated later on
          */
         void addVolume( MedicalVolume* volume, double volumeIntensity, ResourceLut* lut )
         {
            // TODO PUT ACTUAL VALUE!
            volumes.attachVolume( volume, 0, 0 );
            luts.addLut( volume, lut );
            volumeIntensities.addIntensity( volume, volumeIntensity );
         }

         void removeVolume( const MedicalVolume* volume )
         {
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

   protected:
      typedef std::map<Symbol, ContextMprInstance*>   Mprs;

   public:
      /**
       @param contextInstance must be an allocated pointer as it will be deallocated by the constructor
       */
      void addMpr( const Symbol& mprName, ContextMprInstance* contextInstance )
      {
         _mprs[ mprName ] = contextInstance;
      }

      ContextMprInstance* getMpr( const Symbol& name )
      {
         Mprs::iterator it = _mprs.find( name );
         if ( it == _mprs.end() )
            return 0;
         return it->second;
      }

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

   protected:
      Mprs     _mprs;
   };
}

#endif