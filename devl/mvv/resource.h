#ifndef MVV_RESOURCE_H_
# define MVV_RESOURCE_H_

# include "dynamic-resource.h"
# include "transfer-function.h"
# include "types.h"
# include <nll/nll.h>

namespace mvv
{
   typedef nll::imaging::VolumeSpatial<float> MedicalVolume;

   class ResourceOrderList : public DynamicResource
   {
   public:
      typedef std::vector<Order*>   Orders;

      void setOrders( const Orders& o )
      {
         _orders = o;
         notifyChanges();
      }

      const Orders& getOrders() const
      {
         return _orders;
      }

   private:
      Orders   _orders;
   };

   class ResourceVolumeIntensities : public DynamicResource
   {
      typedef std::map<const MedicalVolume*, float> Intensities;

   public:
      void addIntensity( const MedicalVolume* vol, float intensity )
      {
         _intensities[ vol ] = intensity;
         notifyChanges();
      }

      void removeIntensity( const MedicalVolume* vol )
      {
         Intensities::iterator it = _intensities.find( vol );
         if ( it == _intensities.end() )
            return;
         _intensities.erase( it );
         notifyChanges();
      }

      float getIntensity( const MedicalVolume* vol ) const
      {
         Intensities::const_iterator it = _intensities.find( vol );
         if ( it == _intensities.end() )
         {
            unreachable( "should never happen" );
            return 0;
         }
         return it->second;
      }

   private:
      Intensities _intensities;
   };

   // define a Lut resource interface
   class ResourceLut : public DynamicResource, public TransferFunction
   {
   };

   class ResourceLuts : public DynamicResource
   {
   typedef std::map<const MedicalVolume*, ResourceLut*> Luts;

   public:
      ResourceLuts()
      {}

      ~ResourceLuts()
      {
      }

      /**
       @param lut will not be freed
       */
      void addLut( const MedicalVolume* vol, ResourceLut* lut )
      {
         _luts[ vol ] = lut;
         lut->setFather( this );
         notifyChanges();
      }

      void removeLut( const MedicalVolume* vol )
      {
         Luts::iterator it = _luts.find( vol );
         if ( it == _luts.end() )
            return;
         _luts.erase( it );
         notifyChanges();
      }

      ResourceLut* getLut( const MedicalVolume* vol ) const
      {
         Luts::const_iterator it = _luts.find( vol );
         if ( it == _luts.end() )
         {
            unreachable( "should never happen" );
            return 0;
         }
         return it->second;
      }

   private:
      // not supposed to be copiable
      ResourceLuts& operator=( const ResourceLuts );
      ResourceLuts( const ResourceLuts& );

   private:
      Luts _luts;
   };

   class ResourceTransferFunctionWindowing : public ResourceLut
   {
   public:
      ResourceTransferFunctionWindowing( float minWindow, float maxWindow ) : 
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

      void setMinWindow( float v )
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

      void setMaxWindow( float v )
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

      void setLutColor( const f32* colors )
      {
         _lut.createColorScale( colors );
      }

      /**
       @brief Transform a real value to a RGB value
       @param inValue the input value
       @param outValue must be allocated (3 * ui8)
       */
      virtual const f32* transform( float inValue ) const
      {
         return _lut.transform( inValue );
      }

      const nll::imaging::LookUpTransformWindowingRGB& getLut() const
      {
         return _lut;
      }

   protected:
      nll::imaging::LookUpTransformWindowingRGB _lut;
      float   _minWindow;
      float   _maxWindow;
   };



   /**
    @ingroup mvv
    @brief Holds a set of volumes
    */
   class ResourceVolumes : public DynamicResource
   {
      typedef std::set<MedicalVolume*>  Volumes;

   public:
      typedef Volumes::iterator  iterator;

   public:
      /**
       @brief Attach a volume. The pointer must be valid until this object is used/volume attached
       */
      void attachVolume( MedicalVolume* volume )
      {
         _volumes.insert( volume );
         notifyChanges();
      }

      void detachVolume( const MedicalVolume* volume )
      {
         _volumes.erase( const_cast<MedicalVolume*>( volume ) );
         notifyChanges();
      }

      iterator begin()
      {
         return _volumes.begin();
      }

      iterator end()
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
   class ResourceVector3f : public DynamicResource
   {
   public:
      ResourceVector3f( float a, float b, float c )
      {
         _buf[ 0 ] = a;
         _buf[ 1 ] = b;
         _buf[ 2 ] = c;
         notifyChanges();
      }

      ResourceVector3f()
      {
         _buf[ 0 ] = 0;
         _buf[ 1 ] = 0;
         _buf[ 2 ] = 0;
         notifyChanges();
      }

      float getValue( ui32 v ) const
      {
         assert( v < 3 );
         return _buf[ v ];
      }

      nll::core::vector3f getValue() const
      {
         return nll::core::vector3f( _buf[ 0 ], _buf[ 1 ], _buf[ 2 ] );
      }

      float operator[]( ui32 v ) const
      {
         return getValue( v );
      }

      void setValue( ui32 v, float val )
      {
         assert( v < 3 );
         if ( nll::core::equal( val, _buf[ v ] ) )
            return;
         notifyChanges();
         _buf[ v ] = val;
      }

      void setValue( const nll::core::vector3f& v )
      {
         if ( nll::core::equal( v[ 0 ], _buf[ 0 ] ) &&
              nll::core::equal( v[ 1 ], _buf[ 0 ] ) &&
              nll::core::equal( v[ 2 ], _buf[ 0 ] ) )
         {
            return;
         }
         _buf[ 0 ] = v[ 0 ];
         _buf[ 1 ] = v[ 1 ];
         _buf[ 2 ] = v[ 2 ];
         notifyChanges();
      }

      void normalize()
      {
         float l = sqrtf( _buf[ 0 ] * _buf[ 0 ] +
                          _buf[ 1 ] * _buf[ 1 ] +
                          _buf[ 2 ] * _buf[ 2 ] );
         if ( nll::core::equal<float>( l, 1 ) )
            return;
         assert( l > 0 );
         _buf[ 0 ] /= l;
         _buf[ 1 ] /= l;
         _buf[ 2 ] /= l;
         notifyChanges();
      }

   protected:
      float   _buf[ 3 ];
   };

   /**
    @ingroup mvv
    @brief Holds a 2D vector
    */
   template <class T>
   class ResourceVector2 : public DynamicResource
   {
   public:
      ResourceVector2( T a, T b )
      {
         _buf[ 0 ] = a;
         _buf[ 1 ] = b;
         notifyChanges();
      }

      ResourceVector2()
      {
         _buf[ 0 ] = 0;
         _buf[ 1 ] = 0;
         notifyChanges();
      }

      T getValue( ui32 v ) const
      {
         assert( v < 2 );
         return _buf[ v ];
      }

      T operator[]( ui32 v ) const
      {
         return getValue( v );
      }

      void setValue( ui32 v, T val )
      {
         assert( v < 2 );
         if ( nll::core::equal( val, _buf[ v ] ) )
            return;
         notifyChanges();
         _buf[ v ] = val;
      }

   protected:
      T   _buf[ 2 ];
   };

   typedef ResourceVector2<float>    ResourceVector2f;
   typedef ResourceVector2<ui32>     ResourceVector2ui;

   /**
    @brief This class needs to be notified if a change occur as it is very costly to monitor all changes on a volume...
    */
   class ResourceSlicef : public DynamicResource
   {
   public:
      typedef nll::imaging::Slice<nll::f32> Slice;

      Slice slice;
   };

   /**
    @brief This class needs to be notified if a change occur as it is very costly to monitor all changes on a volume...
    */
   class ResourceSliceRGB : public DynamicResource
   {
   public:
      typedef nll::imaging::Slice<nll::ui8> Slice;

      Slice slice;
   };

   /**
    @brief This class needs to be notified if a change occur as it is very costly to monitor all changes on a volume...
    */
   class ResourceImageRGB : public DynamicResource
   {
   public:
      typedef nll::core::Image<ui8> Image;

      Image image;
   };
}

#endif