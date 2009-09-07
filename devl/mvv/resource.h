#ifndef MVV_RESOURCE_H_
# define MVV_RESOURCE_H_

# include "dynamic-resource.h"
# include "transfer-function.h"
# include "types.h"
# include <nll/nll.h>
//# include <nll/lut.h>

namespace mvv
{
   typedef nll::imaging::VolumeSpatial<double> MedicalVolume;

   class ResourceTransferFunctionWindowing : public DynamicResource, public TransferFunction
   {
   public:
      ResourceTransferFunctionWindowing( double minWindow, double maxWindow ) : 
         _lut( minWindow, maxWindow, 256, 3 ),
         _minWindow( minWindow ),
         _maxWindow( maxWindow )
      {
         _lut.createGreyscale();
         notifyChanges();
      }

      ResourceTransferFunctionWindowing() : 
         _lut( 0, 1, 256, 3 ),
         _minWindow( 0 ),
         _maxWindow( 1 )
      {
         _lut.createGreyscale();
      }

      void setMinWindow( double v )
      {
         _minWindow = v;
         _lut.reset( _minWindow, _maxWindow, 256, 3 );
         _lut.createGreyscale();
         notifyChanges();
      }

      double getMinWindow() const
      {
         return _minWindow;
      }

      void setMaxWindow( double v )
      {
         _maxWindow = v;
         _lut.reset( _minWindow, _maxWindow, 256, 3 );
         _lut.createGreyscale();
         notifyChanges();
      }

      double getMaxWindow() const
      {
         return _minWindow;
      }

      void setLutColor( const ui8* colors )
      {
         _lut.createColorScale( colors );
      }

      /**
       @brief Transform a real value to a RGB value
       @param inValue the input value
       @param outValue must be allocated (3 * ui8)
       */
      virtual const ui8* transform( double inValue )
      {
         return _lut.transform( inValue );
      }

   protected:
      nll::imaging::LookUpTransformWindowingRGB _lut;
      double   _minWindow;
      double   _maxWindow;
   };

   /**
    @ingroup mvv
    @brief Holds a set of volumes
    */
   class ResourceVolumes : public DynamicResource
   {
      struct Pair
      {
         Pair( MedicalVolume* v, double r, ResourceTransferFunctionWindowing* w ) : volume( v ), ratio( r ), windowing( w )
         {}

         bool operator==( const Pair& p ) const
         {
            return volume == p.volume;
         }

         bool operator<( const Pair& p ) const
         {
            return volume < p.volume;
         }

         MedicalVolume* volume;
         double         ratio;
         ResourceTransferFunctionWindowing* windowing;
      };
      typedef std::set<Pair> Volumes;

   public:
      typedef Volumes::const_iterator  const_iterator;

   public:
      /**
       @brief Attach a volume. The pointer must be valid until this object is used/volume attached
       @param volume the volume
       @param ratio the ratio used to fuse volumes. The sum of ratio must be equal to 1
       */
      void attachVolume( MedicalVolume* volume, double ratio, ResourceTransferFunctionWindowing* windowing )
      {
         _volumes.insert( Pair( volume, ratio, windowing ) );
         windowing->setFather( this ); // we want to notify the Volumes if a windowing has changed!
         notifyChanges();
      }

      void setRatio( MedicalVolume* volume, double newRatio )
      {
         Volumes::iterator it = _volumes.find( Pair( volume, 0, 0 ) );
         if ( it != _volumes.end() )
         {
            it->ratio = newRatio;
         } else {
            ensure( 0, "error: volume not found" );
         }
      }

      void detachVolume( MedicalVolume* volume )
      {
         _volumes.erase( Pair( volume, 0, 0 ) );
         notifyChanges();
      }

      const_iterator begin() const
      {
         return _volumes.begin();
      }

      const_iterator end() const
      {
         return _volumes.end();
      }

      ui32 size() const
      {
         return (ui32)_volumes.size();
      }

   protected:
      Volumes  _volumes;
   };

   /**
    @ingroup mvv
    @brief Holds a 3D vector
    */
   class ResourceVector3d : public DynamicResource
   {
   public:
      ResourceVector3d( double a, double b, double c )
      {
         _buf[ 0 ] = a;
         _buf[ 1 ] = b;
         _buf[ 2 ] = c;
         notifyChanges();
      }

      ResourceVector3d()
      {
         _buf[ 0 ] = 0;
         _buf[ 1 ] = 0;
         _buf[ 2 ] = 0;
         notifyChanges();
      }

      double getValue( ui32 v ) const
      {
         assert( v < 3 );
         return _buf[ v ];
      }

      double operator[]( ui32 v ) const
      {
         return getValue( v );
      }

      void setValue( ui32 v, double val )
      {
         assert( v < 3 );
         notifyChanges();
         _buf[ v ] = val;
      }

   protected:
      double   _buf[ 3 ];
   };

   /**
    @ingroup mvv
    @brief Holds a 2D vector
    */
   class ResourceVector2d : public DynamicResource
   {
   public:
      ResourceVector2d( double a, double b )
      {
         _buf[ 0 ] = a;
         _buf[ 1 ] = b;
         notifyChanges();
      }

      ResourceVector2d()
      {
         _buf[ 0 ] = 0;
         _buf[ 1 ] = 0;
         notifyChanges();
      }

      double getValue( ui32 v ) const
      {
         assert( v < 2 );
         return _buf[ v ];
      }

      double operator[]( ui32 v ) const
      {
         return getValue( v );
      }

      void setValue( ui32 v, double val )
      {
         assert( v < 2 );
         notifyChanges();
         _buf[ v ] = val;
      }

   protected:
      double   _buf[ 2 ];
   };
}

#endif