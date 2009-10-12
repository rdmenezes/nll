#ifndef MVV_CONTEXT_MPR_H_
# define MVV_CONTEXT_MPR_H_

# include "symbol.h"
# include "resource.h"
# include "order-creator.h"
# include "drawable-engine-mpr.h"
# include "context.h"

namespace mvv
{
   // foraward declaration
   class DrawableMprToolkits;

   /**
    @brief Get a MPR context. This context is local to this specific MPR object.
    */
   class ContextMpr : public ContextInstance
   {
   public:
      class ContextMprInstance
      {
      public:
         ContextMprInstance() : _toolkits( 0 )
         {
         }

         /**
          @param toolkits must an allocated object. It will be deallocated by the context.
          */
         void setDrawableMprToolkits( DrawableMprToolkits* toolkits )
         {
            if ( _toolkits )
               removeDrawableMprToolkits( _toolkits );
            _toolkits = toolkits;
         }

         DrawableMprToolkits* getDrawableMprToolkits()
         {
            return _toolkits;
         }

         /**
          @brief delete the current toolkit
          */
         void removeDrawableMprToolkits( DrawableMprToolkits* toolkits )
         {
            // TODO fix the warning
            delete _toolkits;
            _toolkits = 0;
         }

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

         /**
          @brief Removes a 
          */
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

      private:
         DrawableMprToolkits* _toolkits;
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
}

#endif