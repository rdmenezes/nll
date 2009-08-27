#ifndef MVV_RESOURCE_H_
# define MVV_RESOURCE_H_

# include "dynamic-resource.h"
# include "transfer-function.h"
# include "types.h"
# include <nll/nll.h>

namespace mvv
{
   typedef nll::imaging::VolumeSpatial<double> MedicalVolume;

   class ResourceTransferFunctionWindowing : public DynamicResource, TransferFunction
   {
   public:
      ResourceTransferFunctionWindowing( double minWindow, double maxWindow ) : _minWindow( minWindow ),
         _maxWindow( maxWindow )
      {
         notifyChanges();
      }

      void setMinWindow( double v )
      {
         _minWindow = v;
         notifyChanges();
      }

      double getMinWindow() const
      {
         return _minWindow;
      }

      void setMaxWindow( double v )
      {
         _maxWindow = v;
         notifyChanges();
      }

      double getMaxWindow() const
      {
         return _minWindow;
      }

      /**
       @brief Transform a real value to a RGB value
       @param inValue the input value
       @param outValue must be allocated (3 * ui8)
       */
      virtual void transform( double inValue, ui8* outValue )
      {
         double interval = _maxWindow - _minWindow;
         ensure( interval > 0, "must be >0" );
         const ui8 val = static_cast<ui8>( ( inValue - _minWindow ) / interval * 255 );

         outValue[ 0 ] = val;
         outValue[ 1 ] = val;
         outValue[ 2 ] = val;
      }

   protected:
      double   _minWindow;
      double   _maxWindow;
   };

   /**
    @ingroup mvv
    @brief Holds a set of volumes
    */
   class ResourceVolumes : public DynamicResource
   {
      typedef std::set<MedicalVolume*> Volumes;

   public:
      typedef Volumes::const_iterator  const_iterator;

   public:
      /**
       @brief Attach a volume. The pointer must be valid until this object is used/volume attached
       */
      void attachVolume( MedicalVolume* volume )
      {
         _volumes.insert( volume );
         notifyChanges();
      }

      void detachVolume( MedicalVolume* volume )
      {
         _volumes.erase( volume );
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
         return _volumes.size();
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