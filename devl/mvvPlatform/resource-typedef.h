#ifndef MVV_PLATFORM_RESOURCE_TYPEDEF_H_
# define MVV_PLATFORM_RESOURCE_TYPEDEF_H_

# include "resource-set.h"
# include "resource-map.h"
# include "resource-vector.h"
# include "transfer-function.h"
# include "resource-value.h"
# include "resource-lut.h"

namespace mvv
{
namespace platform
{
   /// defines a set of order in no particular order.
   typedef ResourceSet<RefcountedTyped<Order> >          ResourceOrders;

   /// defines a map of floats. Given a volume name, return the assiated floating value
   typedef ResourceMap<SymbolVolume, f32>                ResourceFloats;

   /// hold a boolean
   typedef ResourceValue<bool>                           ResourceBool;

   /// hold a float
   typedef ResourceValue<f32>                            ResourceFloat;

   /// hold an unsigned int
   typedef ResourceValue<ui32>                           ResourceUi32;

   /// hold a RGB image
   typedef ResourceValue< nll::core::Image< nll::ui8 > > ResourceImageuc;

   /// defines a map of transfer function. Given a volume name, retrieve its associated transfer function
   typedef ResourceMapResource<SymbolVolume, ResourceLut >ResourceMapTransferFunction;

   /// defines a list of 3D vectors
   typedef ResourceVector<nll::core::vector3f>           ResourceVector3fs;

   /// hold an affine registration matrix, must be a 4*4 matrix
   /// the transformation is defined from source->target space
   typedef ResourceValue<nll::core::Matrixf>             ResourceRegistration;

   /// defines a map of registration. Given a volume name, retrieve its associated registration
   typedef ResourceMapResource<SymbolVolume, ResourceRegistration >ResourceMapRegistrations;

   template <>
   class ResourceValue<nll::core::Image< nll::f32 > > : public Resource<nll::core::Image< nll::f32 > >
   {
   public:
      typedef nll::core::Image< nll::f32 > Imagef;

      ResourceValue( bool simple = false ) : Resource( new Imagef(), true, simple )
      {
      }

      virtual ~ResourceValue()
      {
      }

      void setValue( Imagef val )
      {
         // we don't check for equality as potentially, it can be very long, so instead we prefere to notify
         Resource<Imagef>::getValue() = val;
         notify();
      }

      Imagef& getValue()
      {
         return Resource<Imagef>::getValue();
      }

      const Imagef& getValue() const
      {
         return Resource<Imagef>::getValue();
      }
   };

   template <>
   class ResourceValue<nll::core::Image< nll::ui8 > > : public Resource<nll::core::Image< nll::ui8 > >
   {
   public:
      typedef nll::core::Image< nll::ui8 > Imagef;

      ResourceValue( bool simple = false ) : Resource( new Imagef(), true, simple )
      {
      }

      virtual ~ResourceValue()
      {
      }

      void setValue( Imagef val )
      {
         // we don't check for equality as potentially, it can be very long, so instead we prefere to notify
         Resource<Imagef>::getValue() = val;
         notify();
      }

      Imagef& getValue()
      {
         return Resource<Imagef>::getValue();
      }

      const Imagef& getValue() const
      {
         return Resource<Imagef>::getValue();
      }
   };

   /// hold a 32b flaoting image
   typedef ResourceValue< nll::core::Image< nll::f32 > >       ResourceImagef;

   typedef ResourceMapResource<SymbolVolume, ResourceImagef >  ResourceMapImage;
}
}

#endif