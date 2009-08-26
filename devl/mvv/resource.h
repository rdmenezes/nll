#ifndef MVV_RESOURCE_H_
# define MVV_RESOURCE_H_

# include "dynamic-resource.h"
# include "types.h"
# include <nll/nll.h>

namespace mvv
{
   class ResourceVector3d : public DynamicResource
   {
   public:
      ResourceVector3d( double a, double b, double c )
      {
         _buf[ 0 ] = a;
         _buf[ 1 ] = b;
         _buf[ 2 ] = c;
      }

      ResourceVector3d()
      {}

      double getValue( ui32 v ) const
      {
         assert( v < 3 );
         return _buf[ v ];
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

   class ResourceVector2d : public DynamicResource
   {
   public:
      ResourceVector2d( double a, double b )
      {
         _buf[ 0 ] = a;
         _buf[ 1 ] = b;
      }

      ResourceVector2d()
      {}

      double getValue( ui32 v ) const
      {
         assert( v < 2 );
         return _buf[ v ];
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